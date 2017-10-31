/* vecelem */

/* vector element-list operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGAUDIT	0		/* debug auditing */
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
	characters-bytes.


*******************************************************************************/


#define	VECELEM_MASTER		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecelem.h"


/* local defines */


/* forward references */

int		vecelem_add(vecelem *,const void *) ;

static int	vecelem_extend(vecelem *) ;
static int	vecelem_setopts(vecelem *,int) ;


/* local variables */


/* exported subroutines */


int vecelem_start(vecelem *op,int esize,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (esize <= 0) return SR_INVALID ;

	if (n < 0)
	    n = VECELEM_DEFENTS ;

	memset(op,0,sizeof(vecelem)) ;
	op->esize = esize ;

	if ((rs = vecelem_setopts(op,opts)) >= 0) {
	    if (n > 0) {
	        const int	size = (n + 1) * op->esize ;
	        char	*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            op->va = p ;
	    	    op->n = n ;
		    op->magic = VECELEM_MAGIC ;
	        }
	    }
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecelem_start) */


int vecelem_finish(vecelem *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
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
/* end subroutine (vecelem_finish) */


int vecelem_add(vecelem *op,const void *ep)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

/* do we have to grow the array? */

	if ((op->i + 1) > op->n) {
	    rs = vecelem_extend(op) ;
	}

	if (rs >= 0) {
	    caddr_t	vep = op->va ;
	    i = op->i ;
	    vep += (i * op->esize) ;
	    memcpy(vep,ep,op->esize) ;
	    op->i = (i+1) ;
	    op->c += 1 ;			/* increment list count */
	    op->f.issorted = FALSE ;
	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecelem_add) */


int vecelem_adduniq(vecelem *op,const void *ep)
{
	int		rs = INT_MAX ;
	int		i ;
	int		esize ;
	caddr_t		vep ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

/* first, search for this value */

	esize = op->esize ;
	for (i = 0 ; i < op->i ; i += 1) {
	    vep = (caddr_t) op->va ;
	    vep += (i * esize) ;
	    if (memcmp(vep,ep,esize) == 0) break ;
	} /* end for */

	if (i >= op->i) {
	    rs = vecelem_add(op,ep) ;
	}

	return rs ;
}
/* end subroutine (vecelem_adduniq) */


int vecelem_get(vecelem *op,int i,void *vpp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (vpp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	if ((i >= 0) && (i < op->i)) {
	    caddr_t	*cvpp = (caddr_t *) vpp ;
	    caddr_t	vep = op->va ;
	    vep += (i * op->esize) ;
	    *cvpp = vep ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	return rs ;
}
/* end subroutine (vecelem_get) */


int vecelem_getval(vecelem *op,int i,void *vp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	if ((i >= 0) && (i < op->i)) {
	    caddr_t	cvp = (caddr_t) vp ;
	    caddr_t	vep = op->va ;
	    vep += (i * op->esize) ;
	    memcpy(cvp,vep,op->esize) ;
	} else
	    rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (vecelem_getval) */


/* return the count of the number of items in this list */
int vecelem_count(vecelem *op)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	return op->c ;
}
/* end subroutine (vecelem_count) */


/* sort the entries in the list */
int vecelem_sort(op,fcmp)
vecelem		*op ;
int		(*fcmp)() ;
{

	if (op == NULL) return SR_FAULT ;
	if (fcmp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
		const int	esize = op->esize ;
	        qsort(op->va,op->i,esize,fcmp) ;
	    }
	}

	return op->c ;
}
/* end subroutine (vecelem_sort) */


/* set the object to indicate it is sorted (even if it isn't) */
int vecelem_setsorted(vecelem *op)
{

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (vecelem_setsorted) */


/* get the vector array address */
int vecelem_getvec(op,rpp)
VECELEM		*op ;
void		**rpp ;
{

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;
#endif

	*rpp = op->va ;
	return op->i ;
}
/* end subroutine (vecelem_getvec) */


/* audit the object */
int vecelem_audit(VECELEM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VECELEM_MAGIC) return SR_NOTOPEN ;

	if (op->va != NULL) {

	    {
	        long	v = (long) op->va ;
	        if ((v & 3) != 0) rs = SR_BADFMT ;
	    }

	    if (rs >= 0) {
		void		*ep = NULL ;
	        const int	esize = op->esize ;
	        int		i ;

	        if ((rs = uc_malloc(esize,&ep)) >= 0) {
	            caddr_t	cap ;

	            for (i = 0 ; i < op->i ; i += 1) {
		        cap = op->va ;
		        cap += (i * esize) ;
	                memcpy(ep,cap,esize) ;
	            } /* end for */
	            c = op->c ;
	            rs = (i == c) ? SR_OK : SR_BADFMT ;

	            rs1 = uc_free(ep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (memory-allocation) */

	    } /* end if */

	} /* end if (non-NULL va) */

	if (rs >= 0) {
	    if ((op->i > op->n) || (op->c > op->i)) rs = SR_BADFMT ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecelem_audit) */


/* private subroutines */


static int vecelem_setopts(VECELEM *op,int options)
{

	memset(&op->f,0,sizeof(struct vecelem_flags)) ;

	if (options & VECELEM_OREUSE)
	    op->f.oreuse = 1 ;

	if (options & VECELEM_OSWAP)
	    op->f.oswap = 1 ;

	if (options & VECELEM_OSTATIONARY)
	    op->f.ostationary = 1 ;

	if (options & VECELEM_OCOMPACT)
	    op->f.ocompact = 1 ;

	if (options & VECELEM_OSORTED)
	    op->f.osorted = 1 ;

	if (options & VECELEM_OORDERED)
	    op->f.oordered = 1 ;

	if (options & VECELEM_OCONSERVE)
	    op->f.oconserve = 1 ;

	return SR_OK ;
}
/* end subroutine (vecelem_setopts) */


static int vecelem_extend(vecelem *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*va ;

	    if (op->va == NULL) {
	        nn = VECELEM_DEFENTS ;
	        size = (nn + 1) * op->esize ;
	        rs = uc_malloc(size,&va) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * op->esize ;
	        rs = uc_realloc(op->va,size,&va) ;
	    } /* end if */

	    if (rs >= 0) {
	        op->va = va ;
	        op->n = nn ;
	    }

	} /* end if (extension required) */

	return rs ;
}
/* end subroutine (vecelem_extend) */


