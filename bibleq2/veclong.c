/* veclong */

/* vector long-integer operations */


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
	accessed later.  Element data (unlike string data) can contain NULL
	characters/bytes.


*******************************************************************************/


#define	VECLONG_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"veclong.h"


/* local defines */


/* forward references */

int		veclong_add(veclong *,LONG) ;

static int	veclong_extend(veclong *) ;
static int	veclong_setopts(veclong *,int) ;

static int	deflongcmp(const LONG *,const LONG *) ;


/* local variables */


/* exported subroutines */


int veclong_start(op,n,opts)
veclong		*op ;
int		n ;
int		opts ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (n < 0)
	    n = VECLONG_DEFENTS ;

	memset(op,0,sizeof(veclong)) ;

	if ((rs = veclong_setopts(op,opts)) >= 0) {
	    op->n = n ;
	    op->magic = VECLONG_MAGIC ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (veclong_start) */


int veclong_finish(op)
veclong		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
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
/* end subroutine (veclong_finish) */


int veclong_add(op,v)
veclong		*op ;
LONG		v ;
{
	int	rs = SR_OK ;
	int	i ;
	int	f_done = FALSE ;
	int	f ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

/* can we fit this new entry within the existing extent? */

	f = (op->f.oreuse || op->f.oconserve) && (! op->f.oordered) ;
	if (f && (op->c < op->i)) {

	    i = op->fi ;
	    while ((i < op->i) && (op->va[i] != NULL))
	        i += 1 ;

	    if (i < op->i) {

	        (op->va)[i] = v ;
	        op->fi = i + 1 ;
	        f_done = TRUE ;

	    } else
	        op->fi = i ;

	} /* end if (possible reuse strategy) */

/* do we have to grow the vechand array? */

	if (! f_done) {

/* do we have to grow the array? */

	    if ((op->i + 1) > op->n) {
	        rs = veclong_extend(op) ;
	    }

/* link into the list structure */

	    if (rs >= 0) {
	        i = op->i ;
	        (op->va)[(op->i)++] = v ;
	        (op->va)[op->i] = NULL ;
	    }

	} /* end if (added elsewhere) */

	if (rs >= 0) {
	    op->c += 1 ;			/* increment list count */
	    op->f.issorted = FALSE ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (veclong_add) */


int veclong_adduniq(op,v)
veclong		*op ;
LONG		v ;
{
	int	rs = INT_MAX ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] == v) break ;
	}

	if (i >= op->i)
	    rs = veclong_add(op,v) ;

	return rs ;
}
/* end subroutine (veclong_adduniq) */


int veclong_getval(op,i,rp)
veclong		*op ;
int		i ;
LONG		*rp ;
{


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	*rp = NULL ;
	if ((i < 0) || (i >= op->i))
	    return SR_NOTFOUND ;

	*rp = (op->va)[i] ;
	return SR_OK ;
}
/* end subroutine (veclong_getval) */


/* delete an element from the list */
int veclong_del(op,i)
veclong		*op ;
int		i ;
{
	int	f_fi = FALSE ;

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if ((i < 0) || (i >= op->i))
	    return SR_NOTFOUND ;

/* delete the entry */

	    op->c -= 1 ;		/* decrement list count */

/* apply the appropriate deletion based on management policy */

	if (op->f.ostationary) {

	    (op->va)[i] = 0 ;
	    if (i == (op->i - 1))
	        op->i -= 1 ;

	    f_fi = TRUE ;

	} else if (op->f.issorted || op->f.oordered) {

	    if (op->f.ocompact) {
		int	j ;

	        op->i -= 1 ;
	        for (j = i ; j < op->i ; j += 1)
	            (op->va)[j] = (op->va)[j + 1] ;

	        (op->va)[op->i] = 0 ;

	    } else {

	        (op->va)[i] = 0 ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} else {

	    if ((op->f.oswap || op->f.ocompact) && (i < (op->i - 1))) {

	        (op->va)[i] = (op->va)[op->i - 1] ;
	        (op->va)[--op->i] = 0 ;
	        op->f.issorted = FALSE ;

	    } else {

	        (op->va)[i] = 0 ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} /* end if */

	if (f_fi && (i < op->fi))
	    op->fi = i ;

#if	CF_DEBUGS
	debugprintf("veclong_del: ret count=%d index=%d\n",
	    op->c,op->i) ;
#endif

	return op->c ;
}
/* end subroutine (veclong_del) */


/* return the count of the number of items in this list */
int veclong_count(op)
veclong		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	return op->c ;
}
/* end subroutine (veclong_count) */


/* sort the entries in the list */
int veclong_sort(op,fcmp)
veclong		*op ;
int		(*fcmp)() ;
{


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (fcmp == NULL)
	    return SR_FAULT ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1)
	        qsort(op->va,op->i,sizeof(LONG),fcmp) ;
	}

	return op->c ;
}
/* end subroutine (veclong_sort) */


/* set the object to indicate it is sorted (even if it isn't) */
int veclong_setsorted(op)
veclong		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (veclong_setsorted) */


/* find an entry in the vector list by memory comparison of entry elements */
int veclong_find(op,v)
veclong		*op ;
LONG		v ;
{
	LONG	*rpp2 ;

	int	rs = SR_OK ;
	int	(*fcmp)(const void *,const void *) ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (op->f.issorted) {

	    fcmp = (int (*)(const void *,const void *)) deflongcmp ;
	    rpp2 = (LONG *) bsearch(&v,op->va,op->i,sizeof(LONG),fcmp) ;

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
/* end subroutine (veclong_find) */


/* get the vector array address */
int veclong_getvec(op,rpp)
VECLONG		*op ;
LONG		**rpp ;
{


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

	*rpp = op->va ;
	return op->i ;
}
/* end subroutine (veclong_getvec) */


/* audit the object */
int veclong_audit(op)
VECLONG		*op ;
{
	long	v = 0 ;

	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VECLONG_MAGIC)
	    return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    c += 1 ;
	    v |= op->va[i] ;
	}

	rs = (c == op->c) ? SR_OK : SR_BADFMT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (veclong_audit) */


/* private subroutines */


static int veclong_setopts(op,options)
VECLONG		*op ;
int		options ;
{


	memset(&op->f,0,sizeof(struct veclong_flags)) ;

	if (options & VECLONG_OREUSE)
	    op->f.oreuse = 1 ;

	if (options & VECLONG_OSWAP)
	    op->f.oswap = 1 ;

	if (options & VECLONG_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (options & VECLONG_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (options & VECLONG_OSORTED)
	    op->f.osorted = 1 ;

	if (options & VECLONG_OORDERED)
	    op->f.oordered = 1 ;

	if (options & VECLONG_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (veclong_setopts) */


static int veclong_extend(op)
veclong		*op ;
{
	int	rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    const int	esize = sizeof(LONG) ;
	    int		nn, size ;
	    LONG	*va ;

	    if (op->va == NULL) {
	        nn = VECLONG_DEFENTS ;
	        size = (nn + 1) * esize ;
	        rs = uc_malloc(size,&va) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * esize ;
	        rs = uc_realloc(op->va,size,&va) ;
	    }

	    if (rs >= 0) {
	        op->va = va ;
	        op->n = nn ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (veclong_extend) */


static int deflongcmp(l1p,l2p)
const LONG	*l1p, *l2p ;
{


	return (*l1p - *l2p) ;
}
/* end subroutine (deflongcmp) */


