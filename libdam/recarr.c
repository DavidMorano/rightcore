/* recarr */

/* record-arrange management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used when the caller just wants to store their own
	pointer in a vector.  These routines will not copy the structure
	pointed to by the passed pointer.  The caller is responsible for
	keeping the original data in scope during the whole life span of the
	record-array.


*******************************************************************************/


#define	RECARR_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"recarr.h"


/* local defines */

#define	RECARR_DEFENTS	10


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	recarr_setopts(RECARR *,int) ;
static int	recarr_extend(RECARR *) ;


/* local variables */


/* exported subroutines */


int recarr_start(RECARR *op,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("recarr_start: ent n=%d opts=%02X\n",n,opts) ;
#endif

	if (n <= 1)
	    n = RECARR_DEFENTS ;

	memset(op,0,sizeof(RECARR)) ;

	if ((rs = recarr_setopts(op,opts)) >= 0) {
	    int		size = (n + 1) * sizeof(void **) ;
	    void	*p ;
	    if ((rs = uc_libmalloc(size,&p)) >= 0) {
		op->va = p ;
		op->n = n ;
	        op->va[0] = NULL ;
	    } /* end if (memory-allocation) */
	}

#if	CF_DEBUGS
	debugprintf("recarr_start: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recarr_start) */


int recarr_finish(RECARR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("recarr_finish: ent\n") ;
#endif

	rs1 = uc_libfree(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->i = 0 ;
	op->n = 0 ;

#if	CF_DEBUGS
	debugprintf("recarr_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (recarr_finish) */


/* add an entry to this record-array */
int recarr_add(RECARR *op,const void *sp)
{
	int		rs = SR_OK ;
	int		i ;
	int		f_done = FALSE ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* can we fit this new entry within the existing extent? */

	f = (op->f.oreuse || op->f.oconserve) && (! op->f.oordered) ;
	if (f && (op->c < op->i)) {

	    i = op->fi ;
	    while ((i < op->i) && (op->va[i] != NULL))
	        i += 1 ;

	    if (i < op->i) {
	        (op->va)[i] = (void *) sp ;
	        op->fi = i + 1 ;
	        f_done = TRUE ;
	    } else {
	        op->fi = i ;
	    }

	} /* end if (possible reuse strategy) */

/* do we have to grow the recarr array? */

	if (! f_done) {

/* do we have to grow the array? */

	    if ((op->i + 1) > op->n) {
	        rs = recarr_extend(op) ;
	    }

/* link into the list structure */

	    if (rs >= 0) {
	        i = op->i ;
	        (op->va)[(op->i)++] = sp ;
	        (op->va)[op->i] = NULL ;
	    }

	} /* end if (added elsewhere) */

	if (rs >= 0)
	    op->c += 1 ;			/* increment list count */

	op->f.issorted = FALSE ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (recarr_add) */


int recarr_audit(RECARR *op)
{
	int		rs = 0 ;
	int		c = 0 ;
	int		i ;
	int		*ip ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
	        ip = (int *) op->va[i] ;
		rs |= *ip ;		/* SEGFAULT? */
	    }
	} /* end for */

	rs = (c == op->c) ? SR_OK : SR_BADFMT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (recarr_audit) */


/* get an entry (enumerated) from the record-array */
int recarr_get(RECARR *op,int i,const void *vp)
{
	int		rs = SR_OK ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i)) rs = SR_NOTFOUND ;

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;
	}

	return rs ;
}
/* end subroutine (recarr_get) */


/* get the last entry */
int recarr_getlast(RECARR *op,const void *vp)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c > 0) {
	    i = (op->i-1) ;
	    while ((i >= 0) && ((op->va)[i] == NULL)) {
		i -= 1 ;
	    }
	    if (i < 0) rs = SR_BUGCHECK ;
	} else
	    rs = SR_NOTFOUND ;

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (recarr_getlast) */


int recarr_ent(RECARR *op,const void *vp)
{
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] == NULL) continue ;
	    if (op->va[i] == vp) break ;
	} /* end for */

	if (i == op->i)
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (recarr_ent) */


/* delete an entry */
int recarr_del(RECARR *op,int i)
{
	int		j ;
	int		f_fi = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i))
	    return SR_NOTFOUND ;

/* delete the entry */

	op->c -= 1 ;

/* apply the appropriate deletion based on management policy */

	if (op->f.ostationary) {

	    (op->va)[i] = NULL ;
	    if (i == (op->i - 1))
	        op->i -= 1 ;

	    f_fi = TRUE ;

	} else if (op->f.issorted || op->f.oordered) {

	    if (op->f.ocompact) {

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
	debugprintf("recarr_del: ret count=%d index=%d\n",
	    op->c,op->i) ;
#endif

	return op->c ;
}
/* end subroutine (recarr_del) */


int recarr_delhand(RECARR *op,const void *ep)
{
	int		rs ;
	if ((rs = recarr_ent(op,ep)) >= 0) {
	    rs = recarr_del(op,rs) ;
	}
	return rs ;
}
/* end subroutine (recarr_delhand) */


int recarr_delall(RECARR *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	op->i = 0 ;
	op->fi = 0 ;
	op->va[0] = NULL ;

	op->c = 0 ;
	return SR_OK ;
}
/* end subroutine (recarr_delall) */


/* return the count of the number of items in this list */
int recarr_count(RECARR *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (recarr_count) */


/* sort the entries in the record-array */
int recarr_sort(RECARR *op,int (*vcmpfunc)())
{

	if (op == NULL) return SR_FAULT ;
	if (vcmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        qsort(op->va,op->i,sizeof(void *),vcmpfunc) ;
	    }
	}

	return op->c ;
}
/* end subroutine (recarr_sort) */


/* set the object to indicate it is sorted (even if it isn't) */
int recarr_setsorted(RECARR *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (recarr_setsorted) */


/* search for an entry in the record-array */
int recarr_search(RECARR *op,const void *ep,int (*vcmpfunc)(),void *vp)
{
	const int	esize = sizeof(void *) ;
	int		rs ;
	int		i ;
	const void	*lep ;
	const void	**spp ;
	void		**rpp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (vcmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->f.osorted && (! op->f.issorted)) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
		qsort(op->va,op->i,esize,vcmpfunc) ;
	    }
	}

	if (op->f.issorted) {

	    spp = (const void **) bsearch(&ep,op->va,op->i,esize,vcmpfunc) ;

	    rs = SR_NOTFOUND ;
	    if (spp != NULL) {
	        i = (spp - op->va) ;
	        rs = SR_OK ;
	    }

	} else {

	    for (i = 0 ; i < op->i ; i += 1) {
	        lep = op->va[i] ;
	        if (lep != NULL) {
	            if ((*vcmpfunc)(&ep,&lep) == 0) break ;
		}
	    } /* end for */

	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	} /* end if */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ((void *)(op->va[i])) : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (recarr_search) */


/* get the vector array address */
int recarr_getvec(RECARR *op,void *rp)
{
	void		**rpp = (void **) rp ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	*rpp = (void *) op->va ;
	return op->i ;
}
/* end subroutine (recarr_getvec) */


/* return the extent of our allocations */
int recarr_extent(RECARR *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->n ;
}
/* end subroutine (recarr_extent) */


/* private subroutines */


static int recarr_setopts(RECARR *op,int opts)
{

#ifdef	OPTIONAL
	memset(&op->f,0,sizeof(struct recarr_flags)) ;
#endif

	if (opts & RECARR_OREUSE)
	    op->f.oreuse = 1 ;

	if (opts & RECARR_OSWAP)
	    op->f.oswap = 1 ;

	if (opts & RECARR_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (opts & RECARR_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (opts & RECARR_OSORTED)
	    op->f.osorted = 1 ;

	if (opts & RECARR_OORDERED)
	    op->f.oordered = 1 ;

	if (opts & RECARR_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (recarr_setopts) */


static int recarr_extend(RECARR *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*np ;

	    if (op->va == NULL) {
	        nn = RECARR_DEFENTS ;
	        size = (nn + 1) * sizeof(void **) ;
	        rs = uc_libmalloc(size,&np) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * sizeof(void **) ;
	        rs = uc_librealloc(op->va,size,&np) ;
	        op->va = NULL ;
	    }

	    if (rs >= 0) {
	        op->va = (const void **) np ;
	        op->n = nn ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (recarr_extend) */


