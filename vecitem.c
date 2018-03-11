/* vecitem */

/* vector item operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller wants to store a COPY of the
	passed element data into a vector.  These routines will copy and store
	the copied data in the list.  The advantage is that the caller does not
	have to keep the orginal data around in order for the list data to be
	accessed later.  Element data (unlike string data) can contain NULL
	characters-bytes.


*******************************************************************************/


#define	VECITEM_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecitem.h"


/* local defines */


/* forward references */

static int	vecitem_setopts(VECITEM *,int) ;
static int	vecitem_extend(VECITEM *) ;
static int	vecitem_iget(vecitem *,int,void **) ;

int		vecitem_search(vecitem *,void *,int (*)(),void *) ;


/* local variables */


/* exported subroutines */


int vecitem_start(VECITEM *op,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n < 0)
	    n = VECITEM_DEFENTS ;

	memset(op,0,sizeof(VECITEM)) ;

	if ((rs = vecitem_setopts(op,opts)) >= 0) {
	    const int	size = (n + 1) * sizeof(char **) ;
	    if ((rs = uc_malloc(size,&op->va)) >= 0) {
	        op->va[0] = NULL ;
	        op->n = n ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecitem_start) */


int vecitem_finish(VECITEM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if ((op->va)[i] != NULL) {
	        rs1 = uc_free((op->va)[i]) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = uc_free(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->i = 0 ;
	op->n = 0 ;

	return rs ;
}
/* end subroutine (vecitem_finish) */


int vecitem_add(VECITEM *op,void *ep,int el)
{
	int		rs ;
	int		i = 0 ;
	int		f_done = FALSE ;
	int		f ;
	char		*bp = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (el < 0) el = strlen((cchar *) ep) ;

/* allocate memory for the object to add */

	if ((rs = uc_malloc((el+1),&bp)) >= 0) {

	    memcpy(bp,ep,el) ;
	    bp[el] = '\0' ;

/* can we fit this new entry within the existing extent? */

	    f = (op->f.oreuse || op->f.oconserve) && (! op->f.oordered) ;
	    if (f && (op->c < op->i)) {

	        i = op->fi ;
	        while ((i < op->i) && (op->va[i] != NULL)) {
	            i += 1 ;
		}

	        if (i < op->i) {
	            (op->va)[i] = bp ;
	            op->fi = (i + 1) ;
	            f_done = TRUE ;
	        } else {
	            op->fi = i ;
		}

	    } /* end if (possible reuse strategy) */

/* do we have to grow the vector array? */

	    if (! f_done) {

/* do we have to grow the array? */

	        if ((op->i + 1) > op->n) {
	            rs = vecitem_extend(op) ;
	        }

/* link into the list structure */

	        if (rs >= 0) {
	            i = op->i ;
	            (op->va)[(op->i)++] = bp ;
	            (op->va)[op->i] = NULL ;
	        }

	    } /* end if (added elsewhere) */

	    if (rs >= 0) {
	        op->c += 1 ;			/* increment list count */
	    } else {
	        if (bp != NULL) {
	            uc_free(bp) ;
	        }
	    }

	    op->f.issorted = FALSE ;
	} /* end if (m-a) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecitem_add) */


int vecitem_get(VECITEM *op,int i,void *rp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i)) rs = SR_NOTFOUND ;

	if (rp != NULL) {
	    void	**rpp = (void **) rp ;
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	}

	return rs ;
}
/* end subroutine (vecitem_get) */


/* delete an element from the list */
int vecitem_del(VECITEM *op,int i)
{
	int		f_fi = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecitem_del: index=%d\n",op->i) ;
#endif

	if ((i < 0) || (i >= op->i)) return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("vecitem_del: 2\n") ;
#endif

	if ((op->va)[i] != NULL) {
	    op->c -= 1 ;		/* decrement list count */
	    uc_free((op->va)[i]) ;
	} /* end if */

/* apply the appropriate deletion based on management policy */

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

#if	CF_DEBUGS
	debugprintf("vecitem_del: ret count=%d index=%d\n",
	    op->c,op->i) ;
#endif

	return op->c ;
}
/* end subroutine (vecitem_del) */


/* return the count of the number of items in this list */
int vecitem_count(VECITEM *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (vecitem_count) */


/* sort the entries in the list */
int vecitem_sort(VECITEM *op,int (*cmpfunc)())
{

	if (op == NULL) return SR_FAULT ;
	if (cmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
		const int	esize = sizeof(void *) ;
	        qsort(op->va,op->i,esize,cmpfunc) ;
	    }
	}

	return op->c ;
}
/* end subroutine (vecitem_sort) */


int vecitem_curbegin(VECITEM *op,VECITEM_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->c = -1 ;
	return SR_OK ;
}
/* end subroutine (vecitem_curbegin) */


int vecitem_curend(VECITEM *op,VECITEM_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	curp->i = -1 ;
	curp->c = -1 ;
	return SR_OK ;
}
/* end subroutine (vecitem_curend) */


/* fetch the next matching entry */
int vecitem_fetch(op,ep,cp,cmpfunc,rp)
VECITEM		*op ;
void		*ep ;
vecitem_cur	*cp ;
int		(*cmpfunc)() ;
void		*rp ;
{
	vecitem_cur	cur ;
	int		rs = SR_OK ;
	int		i = 0 ;
	int		j ;
	void		**rpp = (void **) rp ;
	void		*ep2 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (cmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecitem_fetch: op=%08X\n",(int) op) ;
	debugprintf("vecitem_fetch: sort state=%d\n",
	    op->f.issorted) ;
#endif

#if	CF_DEBUGS
	debugprintf("vecitem_fetch: 2, va=%08X\n",op->va) ;
#endif

	if (cp == NULL) {
	    cp = &cur ;
	    cp->i = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("vecitem_fetch: continuing, ci=%d\n",cp->i) ;
#endif

	if (cp->i < 0) {

#if	CF_DEBUGS
	    debugprintf("vecitem_fetch: first time\n") ;
#endif

	    cp->c = 0 ;
	    if (rpp == NULL)
	        rpp = &ep2 ;

/* find any one (all of the others should be adjacent!) */

	    if ((rs = vecitem_search(op,ep,cmpfunc,rpp)) > 0) {

/* handle backing up if necessary in the case of a sorted policy */

	        if (op->f.osorted) {
	            void	*last = *rpp ;

	            for (j = (rs - 1) ;
	                (j >= 0) && ((rs = vecitem_iget(op,j,rpp)) >= 0) ; 
	                j -= 1) {

	                if (cmpfunc(&ep,rpp) != 0)
	                    break ;

	                last = *rpp ;

	            } /* end for */

	            cp->i = (j + 1) ;
	            rs = cp->i ;
	            *rpp = last ;

	        } else {
	            cp->i = rs ;
		}

#if	CF_DEBUGS
	        debugprintf("vecitem_fetch: search final i=%d\n",cp->i) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        cp->i = rs ;
	        cp->c = 1 ;
	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("vecitem_fetch: not first time\n") ;
#endif

	    if (op->f.osorted) {

/* using a sorted policy */

	        if (! op->f.issorted) {

	            op->f.issorted = TRUE ;
	            if (op->c > 1) {
	                qsort(op->va,op->i,sizeof(void *),cmpfunc) ;
		    }

/* find any one */

	            if ((rs = vecitem_search(op,ep,cmpfunc,rpp)) >= 0) {
	                int	si = rs ;

	                i = rs ;

/* back up to the first one if necessary */

	                if (si > 0) {
	                    char	*last = *rpp ;

	                    for (j = (si - 1) ;
	                        (j >= 0) && 
	                        ((rs = vecitem_iget(op,j,rpp)) >= 0) ; 
	                        j -= 1) {

	                        if ((*cmpfunc)(&ep,rpp) != 0)
	                            break ;

	                        last = *rpp ;

	                    } /* end for */

	                    i = (j + 1) ;
	                    *rpp = last ;

#if	CF_DEBUGS
	                    debugprintf("vecitem_fetch: search final i=%d\n",
	                        cp->i) ;
#endif

	                } /* end if */

/* skip up to 'cp->c' count minus one number of matches */

	                i += (cp->c - 1) ;
	                cp->i = i ;

	            } else {
	                cp->i = op->i ;
		    }

	        } /* end if (it was out-of-order) */

/* return the next one */

	        i = (cp->i + 1) ;
	        if ((rs = vecitem_iget(op,i,&ep2)) >= 0) {
	            if ((*cmpfunc)(&ep,&ep2) != 0) {
	                rs = SR_NOTFOUND ;
		    }
	        } /* end if */

	    } else {

/* not a sorted policy */

	        for (i = cp->i + 1 ; 
	            (rs = vecitem_iget(op,i,&ep2)) >= 0 ; i += 1) {

	            if (ep2 != NULL) {
	                if ((*cmpfunc)(&ep,&ep2) == 0) break ;
	            }

	        } /* end for */

	    } /* end if (sorted policy or not) */

	    if (rs >= 0) {
	        rs = (cp->i = i) ;
	        cp->c += 1 ;
	        if (rpp != NULL)
	            *rpp = ep2 ;
	    }

	} /* end if (first or subsequent fetch) */

	    if (rpp != NULL) {
	        if (rs < 0) *rpp = NULL ;
	    }

#if	CF_DEBUGS
	debugprintf("vecitem_fetch: ret rs=%d ci=%d\n",rs,cp->i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecitem_fetch) */


/* search for an entry in the vector element list */
int vecitem_search(op,ep,cmpfunc,rp)
VECITEM		*op ;
void		*ep ;
int		(*cmpfunc)() ;
void		*rp ;
{
	int		rs = SR_OK ;
	int		i = 0 ;
	void		**rpp = (void **) rp, **rpp2 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (cmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->f.osorted && (! op->f.issorted)) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        qsort(op->va,op->i,sizeof(void *),cmpfunc) ;
	    }
	} /* end if (sorting) */

	if (op->f.issorted) {
	    const int	esize = sizeof(void *) ;

	    rpp2 = (void **) bsearch(&ep,op->va,op->i,esize,cmpfunc) ;

	    rs = SR_NOTFOUND ;
	    if (rpp2 != NULL) {
	        i = ((char **) rpp2) - ((char **) op->va) ;
	        rs = SR_OK ;
	    }

	} else {

	    for (i = 0 ; i < op->i ; i += 1) {
	        if (op->va[i] != NULL) {
	            if ((*cmpfunc)(&ep,&op->va[i]) == 0) break ;
	        }
	    } /* end for */
	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	} /* end if (sorted or not) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("vecitem_search: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecitem_search) */


/* find an entry in the vector list by memory comparison of entry elements */
int vecitem_find(VECITEM *op,void *ep,int len)
{
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] == NULL) continue ;
	    if (memcmp(ep,op->va[i],len) == 0) break ;
	} /* end for */

	rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecitem_find) */


/* get the vector array address */
int vecitem_getvec(VECITEM *op,void *rp)
{
	void	**rpp = (void **) rp ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	*rpp = op->va ;
	return op->i ;
}
/* end subroutine (vecitem_getvec) */


/* audit the object */
int vecitem_audit(VECITEM *op)
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
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecitem_audit) */


/* private subroutines */


static int vecitem_setopts(VECITEM *op,int options)
{

	memset(&op->f,0,sizeof(struct vecitem_flags)) ;

	if (options & VECITEM_OREUSE)
	    op->f.oreuse = 1 ;

	if (options & VECITEM_OSWAP)
	    op->f.oswap = 1 ;

	if (options & VECITEM_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (options & VECITEM_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (options & VECITEM_OSORTED)
	    op->f.osorted = 1 ;

	if (options & VECITEM_OORDERED)
	    op->f.oordered = 1 ;

	if (options & VECITEM_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (vecitem_setopts) */


static int vecitem_extend(VECITEM *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    const int	esize = sizeof(char **) ;
	    int		nn, size ;
	    void	*np ;

	    if (op->va == NULL) {
	        nn = VECITEM_DEFENTS ;
	        size = (nn + 1) * esize ;
	        rs = uc_malloc(size,&np) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * esize ;
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
/* end subroutine (vecitem_extend) */


static int vecitem_iget(VECITEM *op,int i,void **spp)
{
	int		rs = SR_OK ;

#ifdef	COMMENT
	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;
#endif /* COMMENT */

	*spp = NULL ;
	if ((i >= 0) && (i < op->i)) {
	    *spp = (op->va)[i] ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	return rs ;
}
/* end subroutine (vecitem_iget) */


