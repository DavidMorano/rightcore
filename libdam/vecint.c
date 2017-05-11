/* vecint */

/* vector integer operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* pointer safety */


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
	accessed later. 


*******************************************************************************/


#define	VECINT_MASTER		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecint.h"


/* local defines */


/* forward references */

int		vecint_add(vecint *,int) ;

static int	vecint_extend(vecint *) ;
static int	vecint_setopts(vecint *,int) ;

static int	defintcmp(const int *,const int *) ;


/* local variables */


/* exported subroutines */


int vecint_start(vecint *op,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n < 0) n = VECINT_DEFENTS ;

	memset(op,0,sizeof(vecint)) ;

	if ((rs = vecint_setopts(op,opts)) >= 0) {
	    op->n = n ;
	    if (n > 0) {
	        const int	size = (n + 1) * sizeof(int) ;
		void	*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
		    op->va = (int *) p ;
	    	    op->va[0] = INT_MIN ;
		}
	    }
	    if (rs >= 0)
		op->magic = VECINT_MAGIC ;
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecint_start) */


int vecint_finish(vecint *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->va != NULL) {
	    rs1 = uc_free(op->va) ;
	    if (rs >= 0) rs = rs1 ;
	    op->va = NULL ;
	}

	op->c = 0 ;
	op->i = 0 ;
	op->n = 0 ;
	op->magic = 0 ;

	return rs ;
}
/* end subroutine (vecint_finish) */


int vecint_add(vecint *op,int v)
{
	int		rs = SR_OK ;
	int		i = 0 ; /* ¥ GCC false complaint */
	int		f_done = FALSE ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

/* can we fit this new entry within the existing extent? */

	f = (op->f.oreuse || op->f.oconserve) && (! op->f.oordered) ;
	if (f && (op->c < op->i)) {

	    i = op->fi ;
	    while ((i < op->i) && (op->va[i] != INT_MIN))
	        i += 1 ;

	    if (i < op->i) {
	        (op->va)[i] = v ;
	        op->fi = (i + 1) ;
	        f_done = TRUE ;
	    } else
	        op->fi = i ;

	} /* end if (possible reuse strategy) */

/* do we have to grow the vector array? */

	if (! f_done) {

/* do we have to grow the array? */

	    if ((op->i + 1) > op->n) {
	        rs = vecint_extend(op) ;
	    }

/* link into the list structure */

	    if (rs >= 0) {
	        i = op->i ;
	        (op->va)[(op->i)++] = v ;
	        (op->va)[op->i] = INT_MIN ;
	    }

	} /* end if */

	if (rs >= 0) {
	    op->c += 1 ;		/* increment list count */
	    op->f.issorted = FALSE ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecint_add) */


int vecint_adduniq(vecint *op,int v)
{
	int		rs = INT_MAX ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

/* first, search for this value */

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] == v) break ;
	} /* end for */

	if (i >= op->i) {
	    rs = vecint_add(op,v) ;
	}

	return rs ;
}
/* end subroutine (vecint_adduniq) */


int vecint_getval(vecint *op,int i,int *rp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if ((i < 0) || (i >= op->i)) rs = SR_NOTFOUND ;

	if (rp != NULL) {
	    *rp = (rs >= 0) ? op->va[i] : INT_MIN ;
	}

	return rs ;
}
/* end subroutine (vecint_getval) */


/* delete an element from the list */
int vecint_del(vecint *op,int i)
{
	int		f_fi = FALSE ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if ((i < 0) || (i >= op->i))
	    return SR_NOTFOUND ;

/* delete the entry */

	op->c -= 1 ;			/* decrement list count */

/* apply the appropriate deletion based on management policy */

	if (op->f.ostationary) {

	    (op->va)[i] = INT_MIN ;
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
	        (op->va)[op->i] = INT_MIN ;

	    } else {

	        (op->va)[i] = INT_MIN ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} else {

	    if ((op->f.oswap || op->f.ocompact) && (i < (op->i - 1))) {

	        (op->va)[i] = (op->va)[op->i - 1] ;
	        (op->va)[--op->i] = INT_MIN ;
	        op->f.issorted = FALSE ;

	    } else {

	        (op->va)[i] = INT_MIN ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} /* end if */

	if (f_fi && (i < op->fi)) {
	    op->fi = i ;
	}

#if	CF_DEBUGS
	debugprintf("vecint_del: ret count=%d index=%d\n",
	    op->c,op->i) ;
#endif

	return op->c ;
}
/* end subroutine (vecint_del) */


/* return the count of the number of items in this list */
int vecint_count(vecint *op)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	return op->c ;
}
/* end subroutine (vecint_count) */


/* return the extent of the number of items in this list */
int vecint_extent(vecint *op)
{
	if (op == NULL) return SR_FAULT ;
#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif
	return op->i ;
}
/* end subroutine (vecint_extent) */


/* sort the entries in the list */
int vecint_sort(vecint *op,int (*fcmp)())
{

	if (op == NULL) return SR_FAULT ;
	if (fcmp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        qsort(op->va,op->i,sizeof(int),fcmp) ;
	    }
	}

	return op->c ;
}
/* end subroutine (vecint_sort) */


/* set the object to indicate it is sorted (even if it isn't) */
int vecint_setsorted(vecint *op)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (vecint_setsorted) */


/* find an entry in the vector list by memory comparison of entry elements */
int vecint_find(vecint *op,int v)
{
	int		*rpp2 ;
	int		rs = SR_OK ;
	int		(*fcmp)(const void *,const void *) ;
	int		i = 0 ; /* ¥ GCC false complaint */

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (op->f.issorted) {
	    fcmp = (int (*)(const void *,const void *)) defintcmp ;
	    rpp2 = (int *) bsearch(&v,op->va,op->i,sizeof(int),fcmp) ;
	    rs = SR_NOTFOUND ;
	    if (rpp2 != NULL) {
	        i = rpp2 - op->va ;
	        rs = SR_OK ;
	    }
	} else {
	    for (i = 0 ; i < op->i ; i += 1) {
	        if (op->va[i] == v) break ;
	    } /* end for */
	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;
	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecint_find) */


int vecint_match(vecint *op,int v)
{
	int		rs ;
	if ((rs = vecint_find(op,v)) >= 0) {
	    rs = TRUE ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (vecint_match) */


/* get the vector array address */
int vecint_getvec(vecint *op,int **rpp)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if (rpp != NULL) *rpp = op->va ;

	return op->i ;
}
/* end subroutine (vecint_getvec) */


/* for compatibility with other objects */
int vecint_mkvec(vecint *op,int *va)
{
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
#endif

	if (va != NULL) {
	    const int	n = op->i ;
	    int		i ;
	    int		v ;
	    for (i = 0 ; i < n ; i += 1) {
		v = (op->va)[i] ;
		if (v != INT_MIN) {
		    va[c++] = (op->va)[i] ;
		}
	    } /* end for */
	} /* end if */

	return c ;
}
/* end subroutine (vecint_mkvec) */


int vecint_curbegin(vecint *op,vecint_cur *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
	curp->i = 0 ;
	return SR_OK ;
}
/* end subroutine (vecint_curend) */


int vecint_curend(vecint *op,vecint_cur *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
	curp->i = 0 ;
	return SR_OK ;
}
/* end subroutine (vecint_end) */


int vecint_enum(vecint *op,vecint_cur *curp,int *rp)
{
	int		rs = SR_OK ;
	int		i ;
	int		v = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;
	i = curp->i ;
	if ((i >= 0) && (i < op->i)) {
	    v = (op->va)[i] ;
	    curp->i = (i+1) ;
	} else {
	    rs = SR_NOTFOUND ;
	}
	if (rp != NULL) *rp = (rs >= 0) ? v : INT_MIN ;
	return rs ;
}
/* end subroutine (vecint_enum) */


/* audit the object */
int vecint_audit(vecint *op)
{
	int		rs ;
	int		i, c ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VECINT_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    c = op->va[i] ;
	}

	c = op->c ;
	rs = (i == c) ? SR_OK : SR_BADFMT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecint_audit) */


/* private subroutines */


static int vecint_setopts(vecint *op,int options)
{

	memset(&op->f,0,sizeof(struct vecint_flags)) ;

	if (options & VECINT_OREUSE)
	    op->f.oreuse = 1 ;

	if (options & VECINT_OSWAP)
	    op->f.oswap = 1 ;

	if (options & VECINT_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (options & VECINT_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (options & VECINT_OSORTED)
	    op->f.osorted = 1 ;

	if (options & VECINT_OORDERED)
	    op->f.oordered = 1 ;

	if (options & VECINT_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (vecint_setopts) */


static int vecint_extend(vecint *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    const int	esize = sizeof(int) ;
	    int		nn, size ;
	    int		*va ;

	    if (op->va == NULL) {
	        nn = VECINT_DEFENTS ;
	        size = ((nn + 1) * esize) ;
	        rs = uc_malloc(size,&va) ;
	    } else {
	        nn = ((op->n + 1) * 2) ;
	        size = ((nn + 1) * esize) ;
	        rs = uc_realloc(op->va,size,&va) ;
	    } /* end if */

	    if (rs >= 0) {
	        op->va = va ;
	        op->n = nn ;
	    }

	} /* end if (extension required) */

	return rs ;
}
/* end subroutine (vecint_extend) */


static int defintcmp(const int *l1p,const int *l2p)
{
	return (*l1p - *l2p) ;
}
/* end subroutine (defintcmp) */


