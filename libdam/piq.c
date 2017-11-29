/* piq */

/* container interlocked queue */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* extra safety */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This is a pointer Q. The caller must supply entries with the first two
        (pointer) compnents of the entry available for pointer use by this
        object.


******************************************************************************/


#define	PIQ_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<pq.h>
#include	<localmisc.h>

#include	"piq.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int	pq_finishup(PQ *) ;


/* exported subroutines */


int piq_start(PIQ *qhp)
{
	int		rs ;

	if (qhp == NULL) return SR_FAULT ;

	if ((rs = ptm_create(&qhp->m,NULL)) >= 0) {
	    if ((rs = pq_start(&qhp->frees)) >= 0) {
		qhp->magic = PIQ_MAGIC ;
	    }
	    if (rs < 0)
		ptm_destroy(&qhp->m) ;
	}

	return rs ;
}
/* end subroutine (piq_start) */


int piq_finish(PIQ *qhp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PIQ_MAGIC) return SR_NOTOPEN ;

	rs1 = pq_finishup(&qhp->frees) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&qhp->m) ;
	if (rs >= 0) rs = rs1 ;

	qhp->magic = 0 ;
	return rs ;
}
/* end subroutine (piq_finish) */


/* insert into queue (at the tail) */
int piq_ins(PIQ *qhp,void *vp)
{
	int		rs ;
	int		c = 0 ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    {
		PQ_ENT	*pep = (PQ_ENT *) vp ;
	        rs = pq_ins(&qhp->frees,pep) ;
	        c = rs ;
	    }
	    ptm_unlock(&qhp->m) ;
	} /* end if (mutex) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (piq_ins) */


/* remove from queue (remove from head) */
int piq_rem(PIQ *qhp,void *vp)
{
	PQ_ENT		*pep ;
	int		rs ;
	int		c = 0 ;
	void		**vpp = (void **) vp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    if ((rs = pq_remtail(&qhp->frees,&pep)) >= 0) {
	        c = rs ;
	        if (vpp != NULL) *vpp = pep ;
	    }
	    ptm_unlock(&qhp->m) ;
	} /* end if (mutex) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (piq_rem) */


int piq_count(PIQ *qhp)
{
	int		rs ;
	int		c = 0 ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    {
	        rs = pq_count(&qhp->frees) ;
	        c = rs ;
	    }
	    ptm_unlock(&qhp->m) ;
	} /* end if (mutex) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (piq_count) */


/* perform an audit */
int piq_audit(PIQ *qhp)
{
	int		rs ;
	int		c = 0 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PIQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    {
	        rs = pq_audit(&qhp->frees) ;
		c = rs ;
    	    }
	    ptm_unlock(&qhp->m) ;
	} /* end if (mutex) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (piq_audit) */


/* private subroutines */


static int pq_finishup(PQ *qp)
{
	PQ_ENT		*pep ;
	int		rs = SR_OK ;
	int		rs1 ;

	while ((rs1 = pq_rem(qp,&pep)) >= 0) {
	    rs1 = uc_free(pep) ;
	    if (rs >= 0) rs = rs1 ;
	}
	if ((rs >= 0) && (rs1 != SR_EMPTY)) rs = rs1 ;

	rs1 = pq_finish(qp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pq_finishup) */


