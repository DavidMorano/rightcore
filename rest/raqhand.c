/* raqhand */

/* random-access queue operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These routines are used when the caller just wants to store their own
        pointer in a FIFO-ordered vector. These routines will not copy the
        structure pointed to by the passed pointer. The caller is responsible
        for keeping the original data in scope during the whole life span of the
        vector list.


*******************************************************************************/


#define	RAQHAND_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"raqhand.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	raqhand_setopts(RAQHAND *,int) ;
static int	raqhand_valid(raqhand *,int) ;


/* local variables */


/* exported subroutines */


int raqhand_start(raqhand *op,int n,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("raqhand_start: ent n=%d\n",n) ;
#endif

	if (n <= 1)
	    n = RAQHAND_DEFENTS ;

	memset(op,0,sizeof(RAQHAND)) ;

	if ((rs = raqhand_setopts(op,opts)) >= 0) {
	    const int	size = ((n+1) * sizeof(void *)) ;
	    void	*va ;
	    if ((rs = uc_libmalloc(size,&va)) >= 0) {
		memset(va,0,size) ;
		op->va = va ;
	        op->n = n ;
	    } /* end if (memory-allocation) */
	} /* end if (options) */

#if	CF_DEBUGS
	debugprintf("raqhand_start: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (raqhand_start) */


/* free up the entire list object structure */
int raqhand_finish(raqhand *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("raqhand_finish: ent\n") ;
#endif

	rs1 = uc_libfree(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->c = 0 ;
	op->n = 0 ;

#if	CF_DEBUGS
	debugprintf("raqhand_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (raqhand_finish) */


int raqhand_ins(raqhand *op,const void *ep)
{
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c < op->n) {
	    i = op->ti ;
	    op->va[i] = ep ;
	    op->ti = ((op->ti+1)%op->n) ;
	    op->c += 1 ;
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (raqhand_ins) */


int raqhand_acc(raqhand *op,int ai,void *vp)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	if (ai < op->c) {
	    i = ((ai+op->hi)%op->n) ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (raqhand_acc) */


/* access the last entry */
int raqhand_acclast(raqhand *op,void *vp)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c > 0) {
	    i = (op->ti-1) ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (raqhand_acclast) */


int raqhand_get(raqhand *op,int i,void *vp)
{
	int		rs ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	rs = raqhand_valid(op,i) ;

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ((void *)(op->va)[i]) : NULL ;
	}

	return rs ;
}
/* end subroutine (raqhand_get) */


int raqhand_rem(raqhand *op,void *vp)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	void		**epp = (void **) vp ;

	if (op == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c > 0) {
	    i = op->hi ;
	    op->hi = ((op->hi+1)%op->n) ;
	    op->c -= 1 ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (epp != NULL) {
	    if (rs >= 0) {
	        *epp = ((void *)(op->va)[i]) ;
	        op->va[i] = NULL ;
	    } else {
	        *epp = NULL ;
	    }
	} /* end if (non-null) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (raqhand_rem) */


int raqhand_ent(raqhand *op,void *vp)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c > 0) {
	    int	f = FALSE ;
	    int	j ;
	    for (j = 0 ; j < op->c ; j += 1) {
		i = op->hi ;
	        if (op->va[i] != NULL) {
	            f = (op->va[i] == vp) ;
		    if (f) break ;
	        }
		i = ((i+1)%op->n) ;
	    } /* end for */
	    if (! f) rs = SR_NOTFOUND ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (raqhand_ent) */


/* delete an entry */
int raqhand_del(raqhand *op,int i)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((rs = raqhand_valid(op,i)) >= 0) {
	    op->va[i] = NULL ;
	} /* end if (valid) */

	return rs ;
}
/* end subroutine (raqhand_del) */


int raqhand_delall(raqhand *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	op->hi = 0 ;
	op->ti = 0 ;
	op->c = 0 ;
	return SR_OK ;
}
/* end subroutine (raqhand_delall) */


/* return the count of the number of items in this list */
int raqhand_count(raqhand *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (raqhand_count) */


/* private subroutines */


/* ARGSUSED */
static int raqhand_setopts(RAQHAND *op,int opts)
{

	memset(&op->f,0,sizeof(struct raqhand_flags)) ;
	return SR_OK ;
}
/* end subroutine (raqhand_setopts) */


static int raqhand_valid(raqhand *op,int i)
{
	int		rs = SR_OK ;
	if (op->c > 0) {
	    if (op->hi != op->ti) {
		if ((i < op->hi) || (i >= op->ti)) rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}
	return rs ;
}
/* end subroutine (raqhand_valid) */


#ifdef	COMMENT
static int raqhand_extend(RAQHAND *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*np ;

	    if (op->va == NULL) {
	        nn = RAQHAND_DEFENTS ;
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
/* end subroutine (raqhand_extend) */
#endif /* COMMENT */


