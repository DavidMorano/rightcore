/* varray */

/* variable array object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */

#ifndef	VARRAY_LIBMEMALLOC
#define	VARRAY_LIBMEMALLOC	0	/* default is |uc_memalloc(3uc)| */
#endif


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object attempts to implement a sort of variable-length array of
        elements.


*******************************************************************************/


#define	VARRAY_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<lookaside.h>
#include	<localmisc.h>

#include	"varray.h"


/* local defines */

#if	VARRAY_LIBMEMALLOC
#define	OURMALLOC(size,pointer)		uc_libmalloc((size),(pointer))
#define	OURREALLOC(p1,size,p2)		uc_librealloc((p1),(size),(p2))
#define	OURFREE(pointer)		uc_libfree((pointer))
#else /* VARRAY_LIBMEMALLOC */
#define	OURMALLOC(size,pointer)		uc_malloc((size),(pointer))
#define	OURREALLOC(p1,size,p2)		uc_realloc((p1),(size),(p2))
#define	OURFREE(pointer)		uc_free((pointer))
#endif /* VARRAY_LIBMEMALLOC */


/* forward references */

static int	varray_extend(VARRAY *,int) ;

int		varray_search(varray *,void *,int (*)(),void *) ;


/* local variables */


/* exported subroutines */


int varray_start(varray *op,int esize,int n)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (esize <= 0) return SR_INVALID ;

	if (n <= 0) n = VARRAY_DEFENTS ;

	memset(op,0,sizeof(VARRAY)) ;
	op->esize = esize ;

	{
	    const int	size = (n + 1) * sizeof(void **) ;
	    void	*p ;
	    if ((rs = OURMALLOC(size,&p)) >= 0) {
	        memset(p,0,size) ;
	        op->va = p ;
	        op->n = n ;
	        rs = lookaside_start(&op->la,esize,n) ;
	        if (rs < 0)
	            OURFREE(p) ;
	    } /* end if (m-a) */
	} /* end block */

#if	CF_DEBUGS
	debugprintf("varray_start: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (varray_start) */


int varray_finish(varray *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	rs1 = lookaside_finish(&op->la) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = OURFREE(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->n = 0 ;
	return rs ;
}
/* end subroutine (varray_finish) */


int varray_enum(varray *op,int i,void *rp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("varray_enum: ent i=%d\n",i) ;
#endif

	if (i < (op->imax+1)) {
	    if (op->va[i] != NULL) rs = 1 ;
	} else
	    rs = SR_NOTFOUND ;

	if (rp != NULL) {
	    void	**rpp = (void **) rp ;
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	} /* end if (response wanted) */

#if	CF_DEBUGS
	debugprintf("varray_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varray_enum) */


int varray_acc(varray *op,int i,void *rp)
{
	int		rs = SR_OK ;
	void		*ep = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("varray_acc: ent i=%d\n",i) ;
#endif

	if (i < op->n) {
	    ep = (op->va)[i] ;
	    rs = (ep != NULL) ;
	}

	if (rp != NULL) {
	    void	**rpp = (void **) rp ;
	    *rpp = ep ;
	}

#if	CF_DEBUGS
	debugprintf("varray_acc: ret rs=%d ep{%p}\n",rs,ep) ;
#endif

	return rs ;
}
/* end subroutine (varray_acc) */


int varray_mk(varray *op,int i,void *rp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (i < 0) return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("varray_mk: ent i=%d\n",i) ;
#endif

	if (i >= op->n) {
	    rs = varray_extend(op,i) ;
	}

	if ((rs >= 0) && (op->va[i] == NULL)) {
	    void	*ep ;
	    if ((rs = lookaside_get(&op->la,&ep)) >= 0) {
	        if (i > op->imax) op->imax = i ;
	        op->c += 1 ;
	        op->va[i] = ep ;
#if	CF_DEBUGS
	        debugprintf("varray_mk: ep=%p\n",ep) ;
#endif
	    }
	}

	if (rp != NULL) {
	    void	**rpp = (void **) rp ;
	    *rpp = (rs >= 0) ? (op->va)[i] : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varray_mk: ret rs=%d\n",rs) ;
	if (rp != NULL) {
	    void	**rpp = (void **) rp ;
	    debugprintf("varray_mk: ret rep=%p\n",*rpp) ;
	}
#endif

	return (rs >= 0) ? op->c : rs ;
}
/* end subroutine (varray_mk) */


/* delete an object from the list */
int varray_del(varray *op,int i)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->n))
	    return SR_NOTFOUND ;

/* delete the entry */

	if ((op->va)[i] != NULL) {
	    op->c -= 1 ;		/* decrement list count */
	    rs = lookaside_release(&op->la,(op->va)[i]) ;
	    op->va[i] = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varray_del: ret rs=%d c=%d\n", rs,op->c) ;
#endif

	return (rs >= 0) ? op->c : rs ;
}
/* end subroutine (varray_del) */


/* delete all objects from the list */
int varray_delall(varray *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->n ; i += 1) {
	    if (op->va[i] != NULL) {
	        rs1 = lookaside_release(&op->la,(op->va)[i]) ;
	        if (rs >= 0) rs = rs1 ;
	        op->va[i] = NULL ;
	    }
	} /* end for */

	op->c = 0 ;
	return rs ;
}
/* end subroutine (varray_delall) */


/* return the count of the number of items in this list */
int varray_count(varray *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (varray_count) */


/* search for an entry in the vector object list */
int varray_search(varray *op,void *ep,int (*fvcmp)(),void *vrp)
{
	int		rs = SR_OK ;
	int		i ;
	void		**rpp = (void **) vrp ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (fvcmp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	{
	    for (i = 0 ; i < op->n ; i += 1) {
	        if (op->va[i] != NULL) {
	            if ((*fvcmp)(&ep,(op->va + i)) == 0) break ;
	        }
	    } /* end for */
	    rs = (i < op->n) ? SR_OK : SR_NOTFOUND ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varray_search: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (varray_search) */


/* find an entry in the vector list by memory comparison of entry objects */
int varray_find(varray *op,void *ep)
{
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->n ; i += 1) {
	    if (op->va[i] != NULL) {
	        if (memcmp(ep,op->va[i],op->esize) == 0) break ;
	    }
	} /* end for */

	rs = (i < op->n) ? SR_OK : SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (varray_find) */


int varray_audit(varray *op)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	int		*ip ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->n ; i += 1) {
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
/* end subroutine (varray_audit) */


/* private subroutines */


static int varray_extend(VARRAY *op,int ni)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("varray_extend: ent n=%u ni=%u\n",op->n,ni) ;
#endif

	if (ni >= op->n) {
	    const int	ninc = VARRAY_DEFENTS ;
	    const int	ndif = ((ni+1)-op->n) ;
	    int		nn, size ;
	    void	*np ;
	    nn = (op->n + MAX(ndif,ninc)) ;
	    size = nn * sizeof(void **) ;
	    if (op->va == NULL) {
	        if ((rs = OURMALLOC(size,&np)) >= 0) {
	            memset(np,0,size) ;
	        }
	    } else {
	        if ((rs = OURREALLOC(op->va,size,&np)) >= 0) {
	            void	**nva = np ;
	            const int	nndif = (nn-op->n) ;
	            int		dsize ;
	            dsize = (nndif * sizeof(void **)) ;
	            memset((nva+op->n),0,dsize) ;
	            op->va = NULL ;
	        }
	    }
	    if (rs >= 0) {
	        op->va = (void **) np ;
	        op->n = nn ;
	    }
	} /* end if (reallocation needed) */

#if	CF_DEBUGS
	debugprintf("varray_extend: ret rs=%d nn=%u\n",rs,op->n) ;
#endif

	return rs ;
}
/* end subroutine (varray_extend) */


