/* vechand */

/* vector list operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used when the caller just wants to store their own
	pointer in a vector.  These routines will not copy the structure
	pointed to by the passed pointer.  The caller is responsible for
	keeping the original data in scope during the whole life span of the
	vector list.


*******************************************************************************/


#define	VECHAND_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vechand.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	vechand_setopts(VECHAND *,int) ;
static int	vechand_extend(VECHAND *) ;


/* local variables */


/* exported subroutines */


int vechand_start(vechand *op,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vechand_start: ent n=%d opts=%02X\n",n,opts) ;
#endif

	if (n <= 1)
	    n = VECHAND_DEFENTS ;

	memset(op,0,sizeof(VECHAND)) ;

	if ((rs = vechand_setopts(op,opts)) >= 0) {
	    const int	size = (n + 1) * sizeof(void **) ;
	    void	*va ;
	    if ((rs = uc_libmalloc(size,&va)) >= 0) {
		op->va = va ;
	        op->va[0] = NULL ;
	        op->n = n ;
	        op->f.issorted = FALSE ;
	    } /* end if (memory-allocation) */
	} /* end if (options) */

#if	CF_DEBUGS
	debugprintf("vechand_start: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vechand_start) */


/* free up the entire list object structure */
int vechand_finish(vechand *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vechand_finish: ent\n") ;
#endif

	rs1 = uc_libfree(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->i = 0 ;
	op->n = 0 ;

#if	CF_DEBUGS
	debugprintf("vechand_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (vechand_finish) */


/* add an entry to this vector list */
int vechand_add(vechand *op,const void *sp)
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
	    while ((i < op->i) && (op->va[i] != NULL)) {
	        i += 1 ;
	    }

	    if (i < op->i) {
	        (op->va)[i] = (void *) sp ;
	        op->fi = i + 1 ;
	        f_done = TRUE ;
	    } else {
	        op->fi = i ;
	    }

	} /* end if (possible reuse strategy) */

/* do we have to grow the vechand array? */

	if (! f_done) {

/* do we have to grow the array? */

	    if ((op->i + 1) > op->n) {
	        rs = vechand_extend(op) ;
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
/* end subroutine (vechand_add) */


/* get an entry (enumerated) from the vector list */
int vechand_get(vechand *op,int i,const void *vp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i)) rs = SR_NOTFOUND ;

	if (vp != NULL) {
	    void	**epp = (void **) vp ;
	    *epp = (rs >= 0) ? ((void *)op->va[i]) : NULL ;
	}

	return rs ;
}
/* end subroutine (vechand_get) */


/* get the last entry */
int vechand_getlast(vechand *op,const void *vp)
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

	if (epp != NULL)
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vechand_getlast) */


/* find an entry by its address */
int vechand_ent(vechand *op,const void *vp)
{
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
	        if (op->va[i] == vp) break ;
	    }
	} /* end for */

	if (i == op->i)
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vechand_ent) */


/* delete an entry */
int vechand_del(vechand *op,int i)
{
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
		int	j ;

	        op->i -= 1 ;
	        for (j = i ; j < op->i ; j += 1)
	            (op->va)[j] = (op->va)[j + 1] ;

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

	if (f_fi && (i < op->fi))
	    op->fi = i ;

#if	CF_DEBUGS
	debugprintf("vechand_del: ret count=%d index=%d\n",
	    op->c,op->i) ;
#endif

	return op->c ;
}
/* end subroutine (vechand_del) */


int vechand_delhand(vechand *op,const void *ep)
{
	int		rs ;
	if ((rs = vechand_ent(op,ep)) >= 0) {
	    rs = vechand_del(op,rs) ;
	}
	return rs ;
}
/* end subroutine (vechand_delhand) */


int vechand_delall(vechand *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	op->i = 0 ;
	op->fi = 0 ;
	op->va[0] = NULL ;

	op->c = 0 ;
	return SR_OK ;
}
/* end subroutine (vechand_delall) */


/* return the count of the number of items in this list */
int vechand_count(vechand *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (vechand_count) */


/* sort the entries in the vector list */
int vechand_sort(vechand *op,int (*vcmpfunc)())
{

	if (op == NULL) return SR_FAULT ;
	if (vcmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1)
	        qsort(op->va,op->i,sizeof(void *),vcmpfunc) ;
	}

	return op->c ;
}
/* end subroutine (vechand_sort) */


int vechand_issorted(vechand *op)
{
	if (op == NULL) return SR_FAULT ;
	return op->f.issorted ;
}
/* end subroutine (vechand_issorted) */


/* set the object to indicate it is sorted (even if it isn't) */
int vechand_setsorted(vechand *op)
{
	if (op == NULL) return SR_FAULT ;
	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (vechand_setsorted) */


/* search for an entry in the vector list */
int vechand_search(vechand *op,const void *ep,int (*vcmpfunc)(),void *vp)
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
/* end subroutine (vechand_search) */


/* get the vector array address */
int vechand_getvec(VECHAND *op,void *rp)
{
	void		**rpp = (void **) rp ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	*rpp = (void *) op->va ;
	return op->i ;
}
/* end subroutine (vechand_getvec) */


/* return the extent of our allocations */
int vechand_extent(vechand *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->n ;
}
/* end subroutine (vechand_extent) */


int vechand_audit(vechand *op)
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
		rs |= *ip ;		/* SEGFAULT? */
	    }
	} /* end for */

	rs = (c == op->c) ? SR_OK : SR_BADFMT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vechand_audit) */


/* private subroutines */


static int vechand_setopts(VECHAND *op,int opts)
{

	memset(&op->f,0,sizeof(struct vechand_flags)) ;
	if (opts & VECHAND_OREUSE) op->f.oreuse = 1 ;
	if (opts & VECHAND_OSWAP) op->f.oswap = 1 ;
	if (opts & VECHAND_OSTATIONARY) op->f.ostationary = 1 ;
	if (opts & VECHAND_OCOMPACT) op->f.ocompact = 1 ;
	if (opts & VECHAND_OSORTED) op->f.osorted = 1 ;
	if (opts & VECHAND_OORDERED) op->f.oordered = 1 ;
	if (opts & VECHAND_OCONSERVE) op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (vechand_setopts) */


static int vechand_extend(VECHAND *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*np ;

	    if (op->va == NULL) {
	        nn = VECHAND_DEFENTS ;
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

	} /* end if (extension required) */

	return rs ;
}
/* end subroutine (vechand_extend) */


