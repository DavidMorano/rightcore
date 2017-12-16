/* ciq */
/* Container-Interlocked-Queue */

/* container interlocked queue */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_SAFE		1		/* extra safety */


/* revision history:

	= 1998-03-27, David A­D­ Morano
	This module was adapted from the PPI/LPPI OS code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a container object that implements FIFO access. It is
        interlocked for multi-thread use.

	Usage note:

        Note that we delete all of the entries upon the object itself being
        freed. If the entries are not opaque (as they usually are not), this
        will result in lost memory (memory leaks). It is the responsibility of
	the caller the delete any non-opaque element objects.


*******************************************************************************/


#define	CIQ_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<pq.h>
#include	<localmisc.h>

#include	"ciq.h"


/* local defines */

#define	CIQ_ENT		struct ciq_ent


/* external subroutines */


/* external variables */


/* local structures */

struct ciq_ent {
	PQ_ENT		dummy ;		/* meta */
	void		*vp ;		/* caller supplied pointer */
} ;


/* forward references */

static int	pq_finishup(PQ *) ;
static int	ciq_findhand(CIQ *,PQ_ENT **,const void *) ;


/* exported subroutines */


int ciq_start(CIQ *qhp)
{
	int		rs ;

	if (qhp == NULL) return SR_FAULT ;

	if ((rs = ptm_create(&qhp->m,NULL)) >= 0) {
	    if ((rs = pq_start(&qhp->frees)) >= 0) {
		if ((rs = pq_start(&qhp->fifo)) >= 0) {
		    qhp->magic = CIQ_MAGIC ;
		}
		if (rs < 0)
		    pq_finish(&qhp->frees) ;
	    } /* end if (pq_start) */
	    if (rs < 0)
		ptm_destroy(&qhp->m) ;
	} /* end if (ptm_create) */

	return rs ;
}
/* end subroutine (ciq_start) */


/* free up a queue */
int ciq_finish(CIQ *qhp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;

	rs1 = pq_finishup(&qhp->fifo) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pq_finishup(&qhp->frees) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&qhp->m) ;
	if (rs >= 0) rs = rs1 ;

	qhp->magic = 0 ;
	return rs ;
}
/* end subroutine (ciq_finish) */


/* insert into queue (at the tail) */
int ciq_ins(CIQ *qhp,void *vp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    PQ_ENT	*pep ;
	    const int	rse = SR_EMPTY ;

	    if ((rs = pq_remtail(&qhp->frees,&pep)) == rse) {
		const int	esize = sizeof(CIQ_ENT) ;
	        rs = uc_libmalloc(esize,&pep) ;
	    }

	    if (rs >= 0) {
	        CIQ_ENT	*cep = (CIQ_ENT *) pep ;
	        cep->vp = vp ;
	        rs = pq_ins(&qhp->fifo,pep) ;
	        c = rs ;
	    } /* end if */

	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_ins) */


/* remove from queue (remove from head) */
int ciq_rem(CIQ *qhp,void *vp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	void		**vpp = (void **) vp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    PQ_ENT	*pep ;
	    if ((rs = pq_rem(&qhp->fifo,&pep)) >= 0) {
	        c = rs ;
	        if (vpp != NULL) {
		    CIQ_ENT	*cep = (CIQ_ENT *) pep ;
	            *vpp = cep->vp ;
	        } /* end if */
		rs1 = pq_ins(&qhp->frees,pep) ;
		if (rs1 < 0)
		    uc_libfree(pep) ;
	    } /* end if (pq_rem) */
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

#if	CF_DEBUGS
	debugprintf("ciq_rem: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_rem) */


/* get the entry at the TAIL of queue */
int ciq_gettail(CIQ *qhp,void *vp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	void		**rpp = (void **) vp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    PQ_ENT	*pep ;
	    if ((rs = pq_gettail(&qhp->fifo,&pep)) >= 0) {
	        c = rs ;
	        if (rpp != NULL) {
		    CIQ_ENT	*cep = (CIQ_ENT *) pep ;
	            *rpp = cep->vp ;
	        }
	    } /* end if (pq_gettail) */
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_gettail) */


/* remove from the TAIL of queue (to get "stack-like" behavior) */
int ciq_remtail(CIQ *qhp,void *vp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	void		**rpp = (void **) vp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    PQ_ENT	*pep ;
	    if ((rs = pq_remtail(&qhp->fifo,&pep)) >= 0) {
	        c = rs ;
	        if (rpp != NULL) {
		    CIQ_ENT	*cep = (CIQ_ENT *) pep ;
	            *rpp = cep->vp ;
		}
		rs = pq_ins(&qhp->frees,pep) ;
		if (rs < 0)
		    uc_libfree(pep) ;
	    } /* end if (pq_remtail) */
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_remtail) */


/* remove by handle (pointer address) */
int ciq_remhand(CIQ *qhp,void *vp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	void		**rpp = (void **) vp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    PQ_ENT	*pep ;
	    if ((rs = ciq_findhand(qhp,&pep,vp)) > 0) {
	        if ((rs = pq_unlink(&qhp->fifo,pep)) >= 0) {
	            c = rs ;
	            if (rpp != NULL) {
		        CIQ_ENT	*cep = (CIQ_ENT *) pep ;
	                *rpp = cep->vp ;
		    }
		    rs = pq_ins(&qhp->frees,pep) ;
		    if (rs < 0)
			uc_libfree(pep) ;
	        } /* end if (pq_remtail) */
	    } else if (rs == 0) {
		rs = SR_NOTFOUND ;
	    }
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_remhand) */


int ciq_count(CIQ *qhp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    {
	        rs = pq_count(&qhp->fifo) ;
	        c = rs ;
	    }
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ciq_count) */


/* perform an audit */
int ciq_audit(CIQ *qhp)
{
	int		rs ;
	int		rs1 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != CIQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&qhp->m)) >= 0) {
	    if ((rs = pq_audit(&qhp->frees)) >= 0) {
	        rs = pq_audit(&qhp->fifo) ;
	    }
	    rs1 = ptm_unlock(&qhp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (ciq_audit) */


/* private subroutines */


static int ciq_findhand(CIQ *qhp,PQ_ENT **rpp,const void *vp)
{
	PQ		*pqp = &qhp->fifo ;
	PQ_CUR		cur ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	if ((rs = pq_curbegin(pqp,&cur)) >= 0) {
	    PQ_ENT	*pep ;
	    while ((rs1 = pq_enum(pqp,&cur,&pep)) >= 0) {
		CIQ_ENT	*cep = (CIQ_ENT *) pep ;
		f = (cep->vp == vp) ;
		if (f) break ;
	    } /* end while */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    if (rpp != NULL) {
		*rpp = (f) ? pep : NULL ;
	    }
	    rs1 = pq_curend(pqp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pq-cur) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ciq_findhand) */


static int pq_finishup(PQ *qp)
{
	PQ_ENT		*pep ;
	int		rs = SR_OK ;
	int		rs1 ;

	while ((rs1 = pq_rem(qp,&pep)) >= 0) {
	    rs1 = uc_libfree(pep) ;
	    if (rs >= 0) rs = rs1 ;
	}
	if ((rs >= 0) && (rs1 != SR_EMPTY)) rs = rs1 ;

	rs1 = pq_finish(qp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pq_finishup) */


