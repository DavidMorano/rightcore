/* vecobj */

/* vector object list operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller wants to store a COPY of the
	passed object data into a vector.  These routines will copy and store
	the copied data in the list.  The advantage is that the caller does not
	have to keep the orginal data around in order for the list data to be
	accessed later.  Element data (unlike string data) can contain NULL
	characters-bytes.


*******************************************************************************/


#define	VECOBJ_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<lookaside.h>
#include	<localmisc.h>

#include	"vecobj.h"


/* local defines */

#define	NDEBVECOBJ	"ucmemalloc.deb"

/* forward references */

int		vecobj_addnew(VECOBJ *,void **) ;

static int	vecobj_setopts(VECOBJ *,int) ;
static int	vecobj_extend(VECOBJ *) ;
static int	vecobj_iget(VECOBJ *,int,void **) ;

int		vecobj_search(vecobj *,void *,int (*)(),void *) ;


/* local variables */


/* exported subroutines */


int vecobj_start(vecobj *op,int osize,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (osize <= 0) return SR_INVALID ;

	if (n <= 0) n = VECOBJ_DEFENTS ;

	op->va = NULL ;
	op->esize = osize ;
	if ((rs = vecobj_setopts(op,opts)) >= 0) {
	    const int	size = (n + 1) * sizeof(char **) ;
	    void	*p ;
	    op->i = 0 ;
	    op->c = 0 ;
	    op->fi = 0 ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->va = p ;
	        op->va[0] = NULL ;
	        op->n = n ;
	        rs = lookaside_start(&op->la,osize,n) ;
	        if (rs < 0)
	            uc_free(p) ;
	    } /* end if */
	} /* end if (options) */

#if	CF_DEBUGS
	debugprintf("vecobj_start: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecobj_start) */


int vecobj_finish(vecobj *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	rs1 = lookaside_finish(&op->la) ;
	if (rs >= 0) rs = rs1 ;

/* free the pointer array itself */

	rs1 = uc_free(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->i = 0 ;
	op->n = 0 ;
	return rs ;
}
/* end subroutine (vecobj_finish) */


int vecobj_add(vecobj *op,const void *s)
{
	int		rs ;
	void		*ep ;

	if ((rs = vecobj_addnew(op,&ep)) >= 0) {
	    memcpy(ep,s,op->esize) ;
	}

	return rs ;
}
/* end subroutine (vecobj_add) */


int vecobj_adduniq(vecobj *op,const void *ep)
{
	int		rs = INT_MAX ;
	int		i ;
	int		esize ;
	const caddr_t	*vepp ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

/* first, search for this value */

	esize = op->esize ;
	for (i = 0 ; i < op->i ; i += 1) {
	    vepp = (caddr_t *) op->va ;
	    vepp += i ;
	    if (memcmp(*vepp,ep,esize) == 0) break ;
	} /* end for */

	if (i >= op->i) {
	    rs = vecobj_add(op,ep) ;
	}

	return rs ;
}
/* end subroutine (vecobj_adduniq) */


int vecobj_addnew(vecobj *op,void **epp)
{
	int		rs ;
	int		i = 0 ;
	void		*sp = NULL ;		/* storage pointer */

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* allocate space and save the pointer to it */

	if ((rs = lookaside_get(&op->la,&sp)) >= 0) {
	    int		f ;
	    int		f_done = FALSE ;

/* can we fit this new entry within the existing extent? */

	    f = (op->f.oreuse || op->f.oconserve) && (! op->f.oordered) ;
	    if (f && (op->c < op->i)) {

	        i = op->fi ;
	        while ((i < op->i) && (op->va[i] != NULL)) {
	            i += 1 ;
		}

	        if (i < op->i) {
	            (op->va)[i] = sp ;
	            op->fi = i + 1 ;
	            f_done = TRUE ;
	        } else {
	            op->fi = i ;
		}

	    } /* end if (possible reuse strategy) */

/* do we have to grow the vector array? */

	    if (! f_done) {

/* do we have to grow the array? */

	        if ((op->i + 1) > op->n) {
	            rs = vecobj_extend(op) ;
		}

/* link into the list structure */

	        if (rs >= 0) {
	            i = op->i ;
	            (op->va)[(op->i)++] = sp ;
	            (op->va)[op->i] = NULL ;
	        }

	    } /* end if (added elsewhere) */

	    if (rs >= 0) {
	        op->c += 1 ;			/* increment list count */
	        op->f.issorted = FALSE ;
	    } else {
	        lookaside_release(&op->la,sp) ;
	    }

	} /* end if (entry allocated) */

	if (epp != NULL) *epp = (rs >= 0) ? sp : NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecobj_addnew) */


/* insert the entry (possible conditionally) into its proper position */
int vecobj_inorder(vecobj *op,void *ep,int (*fvcmp)(),int cn)
{
	int		rs = SR_OK ;
	int		i, n, ei ;
	int		cr ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (fvcmp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecobj_inorder: cn=%d\n",cn) ;
#endif

	if (op->f.osorted) {

	    if (! op->f.issorted) {

	        op->f.issorted = TRUE ;
	        if (op->c > 1) {
	            const int	esize = sizeof(void *) ;
	            qsort(op->va,op->i,esize,fvcmp) ;
	        }

	    } /* end if (sorting) */

	    if (cn != 0) {

	        n = (cn > 0) ? MIN(cn,op->i) : op->i ;

	        cr = 1 ;
	        i = 0 ;
	        if ((cn > 0) && (n > 0)) {

	            for (i = (n - 1) ; i >= 0 ; i -= 1) {
	                if (op->va[i] != NULL) break ;
	            }

	            if (i >= 0) {
	                cr = (*fvcmp)(&ep,&op->va[i]) ;
	                if (cr >= 0) i = n ;
	            } else {
	                i = 0 ;
		    }

	        } /* end if (conditional insertion) */

	        if ((cn < 0) || (cr < 0)) {

	            for (i = 0 ; i < n ; i += 1) {
	                if (op->va[i] == NULL) continue ;
	                cr = (*fvcmp)(&ep,&op->va[i]) ;
	                if (cr < 0) break ;
	            } /* end for */

	        } /* end if (finding insert point) */

/* do the insertion according to what has been determined */

	        if (i < n) {
	            int		j ;
	            void	*tp ;

	            ei = i ;
	            if ((rs = vecobj_add(op,ep)) >= 0) {

	                tp = op->va[rs] ;
	                for (j = (op->i - 1) ; j > i ; j -= 1) {
	                    op->va[j] = op->va[j - 1] ;
	                }
	                op->va[i] = tp ;

	            } /* end if */

	        } else if ((cn < 0) || (i < cn)) {

	            rs = vecobj_add(op,ep) ;
	            ei = rs ;

	        } else
	            rs = SR_NOTFOUND ;

	    } else
	        rs = SR_NOTFOUND ;

	} else {
	    rs = vecobj_add(op,ep) ;
	    ei = rs ;
	}

#if	CF_DEBUGS
	debugprintf("vecobj_inorder: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (vecobj_inorder) */


int vecobj_get(vecobj *op,int i,void *rp)
{
	void		**rpp = (void **) rp ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i >= 0) && (i < op->i)) {
	    *rpp = op->va[i] ;
	} else {
	    *rpp = NULL ;
	    rs = SR_NOTFOUND ;
	}

	return rs ;
}
/* end subroutine (vecobj_get) */


int vecobj_store(vecobj *op,const void *s,void *rp)
{
	int		rs ;
	int		i = 0 ;

	if ((rs = vecobj_add(op,s)) >= 0) {
	    i = rs ;
	    if (rp != NULL) {
	        rs = vecobj_get(op,i,rp) ;
	    }
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecobj_store) */


/* delete an object from the list */
int vecobj_del(vecobj *op,int i)
{
	int		rs = SR_OK ;
	int		f_fi = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i)) return SR_NOTFOUND ;

/* delete the entry */

	if ((op->va)[i] != NULL) {
	    op->c -= 1 ;		/* decrement list count */
	    rs = lookaside_release(&op->la,(op->va)[i]) ;
	    op->va[i] = NULL ;
	}

/* apply the appropriate deletion based on management policy */

	if (rs >= 0) {

	    if (op->f.ostationary) {

	        (op->va)[i] = NULL ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } else if (op->f.issorted || op->f.oordered) {

	        if (op->f.ocompact) {
	            int	j ;

	            op->i -= 1 ;
	            for (j = i ; j < op->i ; j += 1) {
	                (op->va)[j] = (op->va)[j + 1] ;
		    }

	            (op->va)[op->i] = NULL ;

	        } else {

	            (op->va)[i] = NULL ;
	            if (i == (op->i - 1))
	                op->i -= 1 ;

	            f_fi = TRUE ;

	        } /* end if */

	    } else {

	        if ((op->f.oswap || op->f.ocompact) && (i < (op->i - 1))) {

	            (op->va)[i] = (op->va)[op->i - 1] ;
	            (op->va)[--op->i] = NULL ;
	            op->f.issorted = FALSE ;

	        } else {

	            (op->va)[i] = NULL ;
	            if (i == (op->i - 1))
	                op->i -= 1 ;

	            f_fi = TRUE ;

	        } /* end if */

	    } /* end if */

	    if (op->f.oconserve) {

	        while (op->i > i) {
	            if (op->va[op->i - 1] != NULL) break ;
	            op->i -= 1 ;
	        } /* end while */

	    } /* end if */

	    if (f_fi && (i < op->fi)) {
	        op->fi = i ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("vecobj_del: ret rs=%d c=%d i=%d\n",
	    rs,op->c,op->i) ;
#endif

	return (rs >= 0) ? op->c : rs ;
}
/* end subroutine (vecobj_del) */


/* delete all objects from the list */
int vecobj_delall(vecobj *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if ((op->va)[i] != NULL) {
	        rs1 = lookaside_release(&op->la,(op->va)[i]) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	op->va[0] = NULL ;
	op->i = 0 ;
	op->fi = 0 ;
	op->c = 0 ;
	return rs ;
}
/* end subroutine (vecobj_delall) */


/* return the count of the number of items in this list */
int vecobj_count(vecobj *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (vecobj_count) */


/* sort the entries in the list */
int vecobj_sort(vecobj *op,int (*fvcmp)())
{

	if (op == NULL) return SR_FAULT ;
	if (fvcmp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        const int	esize = sizeof(void *) ;
	        qsort(op->va,op->i,esize,fvcmp) ;
	    }
	}

	return op->c ;
}
/* end subroutine (vecobj_sort) */


/* set the object to indicate it is sorted (even if it isn't) */
int vecobj_setsorted(vecobj *op)
{

	if (op == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (op->magic != VECOBJ_MAGIC) return SR_NOTOPEN ;
#endif

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (vecobj_setsorted) */


int vecobj_curbegin(vecobj *op,vecobj_cur *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->c = -1 ;
	return SR_OK ;
}
/* end subroutine (vecobj_curbegin) */


int vecobj_curend(vecobj *op,vecobj_cur *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->c = -1 ;
	return SR_OK ;
}
/* end subroutine (vecobj_curend) */


/* fetch the next matching entry */
int vecobj_fetch(vecobj *op,void *ep,vecobj_cur *curp,int (*fvcmp)(),void *rp)
{
	vecobj_cur	cur ;
	int		rs = SR_OK ;
	int		i, j ;
	void		**rpp = (void **) rp ;
	void		*ep2 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (fvcmp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecobj_fetch: op=%08X\n",(int) op) ;
	debugprintf("vecobj_fetch: sort state=%d\n",
	    op->f.issorted) ;
#endif

#if	CF_DEBUGS
	debugprintf("vecobj_fetch: 2, va=%08X\n",op->va) ;
#endif

	if (curp == NULL) {
	    curp = &cur ;
	    curp->i = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("vecobj_fetch: continuing, ci=%d\n",curp->i) ;
#endif

	if (curp->i < 0) {

#if	CF_DEBUGS
	    debugprintf("vecobj_fetch: first time\n") ;
#endif

	    curp->c = 0 ;
	    if (rpp == NULL)
	        rpp = &ep2 ;

/* find any one (all of the others should be adjacent!) */

	    if ((rs = vecobj_search(op,ep,fvcmp,rpp)) >= 0) {

/* handle backing up if necessary in the case of a sorted policy */

	        if (op->f.osorted) {
	            void	*last = *rpp ;

	            for (j = (rs - 1) ;
	                (j >= 0) && ((rs = vecobj_iget(op,j,rpp)) >= 0) ; 
	                j -= 1) {

	                if (fvcmp(&ep,rpp) != 0)
	                    break ;

	                last = *rpp ;

	            } /* end for */

	            curp->i = j + 1 ;
	            rs = curp->i ;
	            *rpp = last ;

	        } else {
	            curp->i = rs ;
		}

#if	CF_DEBUGS
	        debugprintf("vecobj_fetch: search final i=%d\n",curp->i) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        curp->i = rs ;
	        curp->c = 1 ;
	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("vecobj_fetch: not first time\n") ;
#endif

	    if (op->f.osorted) {

/* using a sorted policy */

	        if (! op->f.issorted) {
	            int	si ;

	            op->f.issorted = TRUE ;
	            if (op->c > 1) {
	                const int	esize = sizeof(void *) ;
	                qsort(op->va,op->i,esize,fvcmp) ;
	            }

/* find any one */

	            if ((si = vecobj_search(op,ep,fvcmp,rpp)) >= 0) {

	                i = si ;

/* back up to the first one if necessary */

	                if (si > 0) {
	                    char	*last = *rpp ;

	                    for (j = (si - 1) ;
	                        (j >= 0) && 
	                        ((rs = vecobj_iget(op,j,rpp)) >= 0) ; 
	                        j -= 1) {

	                        if ((*fvcmp)(&ep,rpp) != 0)
	                            break ;

	                        last = *rpp ;

	                    } /* end for */

	                    i = j + 1 ;
	                    *rpp = last ;

#if	CF_DEBUGS
	                    debugprintf("vecobj_fetch: search final i=%d\n",
	                        curp->i) ;
#endif

	                } /* end if */

/* skip up to 'cp->c' count minus one number of matches */

	                i += (curp->c - 1) ;
	                curp->i = i ;

	            } else {
	                curp->i = op->i ;
		    }

	        } /* end if (it was out-of-order) */

/* return the next one */

	        i = curp->i + 1 ;
	        if ((rs = vecobj_iget(op,i,&ep2)) >= 0) {

	            if ((*fvcmp)(&ep,&ep2) != 0)
	                rs = SR_NOTFOUND ;

	        } /* end if */

	    } else {

/* not a sorted policy */

	        for (i = curp->i + 1 ; 
	            (rs = vecobj_iget(op,i,&ep2)) >= 0 ; i += 1) {
	            if (ep2 != NULL) {
	                if ((*fvcmp)(&ep,&ep2) == 0) break ;
		    }
	        } /* end for */

	    } /* end if (sorted policy or not) */

	    if (rs >= 0) {

	        rs = curp->i = i ;
	        curp->c += 1 ;
	        if (rpp != NULL)
	            *rpp = ep2 ;

	    } /* end if (ok) */

	} /* end if (first or subsequent fetch) */

	if (rs < 0) {

	    rs = SR_NOTFOUND ;
	    if (rpp != NULL)
	        *rpp = NULL ;

	} /* end if (error) */

#if	CF_DEBUGS
	debugprintf("vecobj_fetch: ret rs=%d ci=%d\n",
	    rs,curp->i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecobj_fetch) */


/* search for an entry in the vector object list */
int vecobj_search(vecobj *op,void *ep,int (*fvcmp)(),void *vrp)
{
	int		rs = SR_OK ;
	int		i ;
	void		**rpp = (void **) vrp ;
	void		**rpp2 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (fvcmp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->f.osorted && (! op->f.issorted)) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        const int	esize = sizeof(void *) ;
	        qsort(op->va,op->i,esize,fvcmp) ;
	    }
	} /* end if (sorting) */

	if (op->f.issorted) {
	    const int	esize = sizeof(void *) ;

	    rpp2 = (void **) bsearch(&ep,op->va,op->i,esize,fvcmp) ;

	    rs = SR_NOTFOUND ;
	    if (rpp2 != NULL) {
	        rs = SR_OK ;
	        i = ((char **) rpp2) - ((char **) op->va) ;
	    }

	} else {

	    for (i = 0 ; i < op->i ; i += 1) {
	        if (op->va[i] == NULL) continue ;
	        if ((*fvcmp)(&ep,(op->va + i)) == 0) break ;
	    } /* end for */

	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	} /* end if (sorted or not) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("vecobj_search: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecobj_search) */


/* find an entry in the vector list by memory comparison of entry objects */
int vecobj_find(vecobj *op,void *ep)
{
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] == NULL) continue ;
	    if (memcmp(ep,op->va[i],op->esize) == 0) break ;
	} /* end for */

	rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecobj_find) */


/* get the vector array address */
int vecobj_getvec(VECOBJ *op,void *rp)
{
	void		**rpp = (void **) rp ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	*rpp = op->va ;
	return op->i ;
}
/* end subroutine (vecobj_getvec) */


int vecobj_audit(VECOBJ *op)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	int		*ip ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
	        c += 1 ;
	        ip = (int *) op->va[i] ;
	        rs |= *ip ;		/* access might SEGFAULT */
	    }
	} /* end for */

	rs = (c == op->c) ? SR_OK : SR_BADFMT ;

	if (rs >= 0) {
	    rs = lookaside_audit(&op->la) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecobj_audit) */


/* private subroutines */


static int vecobj_setopts(VECOBJ *op,int opts)
{

	memset(&op->f,0,sizeof(struct vecobj_flags)) ;

	if (opts & VECOBJ_OREUSE)
	    op->f.oreuse = 1 ;

	if (opts & VECOBJ_OSWAP)
	    op->f.oswap = 1 ;

	if (opts & VECOBJ_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (opts & VECOBJ_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (opts & VECOBJ_OSORTED)
	    op->f.osorted = 1 ;

	if (opts & VECOBJ_OORDERED)
	    op->f.oordered = 1 ;

	if (opts & VECOBJ_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (vecobj_setopts) */


static int vecobj_extend(VECOBJ *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*np ;
	    if (op->va == NULL) {
	        nn = VECOBJ_DEFENTS ;
	        size = (nn + 1) * sizeof(void **) ;
	        rs = uc_malloc(size,&np) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * sizeof(void **) ;
	        rs = uc_realloc(op->va,size,&np) ;
	        op->va = NULL ;
	    }
	    if (rs >= 0) {
	        op->va = (void **) np ;
	        op->n = nn ;
	    }
	} /* end if (extension required) */

	return rs ;
}
/* end subroutine (vecobj_extend) */


static int vecobj_iget(vecobj *op,int i,void **spp)
{
	int		rs = SR_NOTFOUND ;

	if ((i >= 0) && (i < op->i)) {
	    *spp = (op->va)[i] ;
	    rs = i ;
	} else {
	    *spp = NULL ;
	}

	return rs ;
}
/* end subroutine (vecobj_iget) */


