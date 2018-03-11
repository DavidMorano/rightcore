/* pq */

/* a regular (no-frills) pointer queue */


#define	CF_SAFE		0		/* extra safety */


/* revision history:

	= 1998-03-03, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a regular, pointer based, no-frills doubly linked list queue.
        Note that this object CAN be moved (copied) since there are no pointers
        pointing back at the list head (located in the object). This object
        (header) is NOT circularly linked.


*******************************************************************************/


#define	PQ_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>

#include	"pq.h"


/* external subroutines */


/* external variables */


/* exported subroutines */


/* initialize a queue head */
int pq_start(PQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	qhp->head = NULL ;
	qhp->tail = NULL ;
	qhp->count = 0 ;
	return SR_OK ;
}
/* end subroutine (pq_start) */


/* free up a queue */
int pq_finish(PQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	qhp->head = NULL ;
	qhp->tail = NULL ;

	return SR_OK ;
}
/* end subroutine (pq_finish) */


/* insert into queue (at the tail) */
int pq_ins(PQ *qhp,PQ_ENT *ep)
{
	PQ_ENT		*pep ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (qhp->head != NULL) {

	    ep->next = NULL ;
	    ep->prev = qhp->tail ;

	    pep = qhp->tail ;
	    pep->next = ep ;

	    qhp->tail = ep ;

	} else {

	    ep->next = NULL ;
	    ep->prev = NULL ;

	    qhp->head = ep ;
	    qhp->tail = ep ;

	} /* end if */

	qhp->count += 1 ;
	return qhp->count ;
}
/* end subroutine (pq_ins) */


/* insert a group into queue (at the tail) */
int pq_insgroup(PQ *qhp,PQ_ENT *gp,int size,int n)
{
	PQ_ENT		*ep, *pep ;
	int		i ;
	caddr_t		p = (caddr_t) gp ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (n <= 0) return SR_OK ;

	ep = (PQ_ENT *) p ;
	if (qhp->head != NULL) {
	    pep = qhp->tail ;
	    pep->next = ep ;
	    ep->prev = pep ;
	} else {
	    ep->prev = NULL ;
	    qhp->head = ep ;
	} /* end if */

	pep = ep ;
	p += size ;
	for (i = 1 ; i < n ; i += 1) {
	    ep = (PQ_ENT *) p ;
	    pep->next = ep ;
	    ep->prev = pep ;
	    p += size ;
	    pep = ep ;
	} /* end for */

	pep->next = NULL ;
	qhp->tail = pep ;

	qhp->count += n ;
	return qhp->count ;
}
/* end subroutine (pq_insgroup) */


/* get the entry at the TAIL of queue */
int pq_gettail(PQ *qhp,PQ_ENT **epp)
{
	PQ_ENT		*ep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (qhp->head == NULL)
	    return SR_EMPTY ;

	ep = qhp->tail ;
	if (ep->next != NULL) rs = SR_NOANODE ;

	if (epp != NULL) *epp = (rs >= 0) ? ep : NULL ;
	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (pq_gettail) */


/* remove from queue (remove from head) */
int pq_rem(PQ *qhp,PQ_ENT **epp)
{
	PQ_ENT		*ep, *nep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (qhp->head == NULL)
	    return SR_EMPTY ;

	ep = qhp->head ;
	if (ep->prev == NULL) {
	    if (ep->next != NULL) {
	        nep = ep->next ;
	        nep->prev = NULL ;
	        qhp->head = nep ;
	    } else {
	        if (qhp->tail == ep) {
	            qhp->head =  NULL ;
	            qhp->tail =  NULL ;
	        } else {
		    rs = SR_NOANODE ;
		}
	    } /* end if */
	} else {
	    rs = SR_NOANODE ;
	}

	if (epp != NULL) *epp = (rs >= 0) ? ep : NULL ;
	if (rs >= 0) qhp->count -= 1 ;
	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (pq_rem) */


/* remove from the TAIL of queue (to get "stack-like" behavior) */
int pq_remtail(PQ *qhp,PQ_ENT **epp)
{
	PQ_ENT		*ep, *pep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (qhp->head == NULL)
	    return SR_EMPTY ;

	ep = qhp->tail ;
	if (ep->next == NULL) {
	    if (ep->prev != NULL) {
	        pep = ep->prev ;
	        pep->next = NULL ;
	        qhp->tail = pep ;
	    } else {
	        if (qhp->head == ep) {
	            qhp->head = NULL ;
	            qhp->tail = NULL ;
	        } else {
		    rs = SR_NOANODE ;
		}
	    } /* end if */
	} else {
	    rs = SR_NOANODE ;
	}

	if (epp != NULL) *epp = (rs >= 0) ? ep : NULL ;
	if (rs >= 0) qhp->count -= 1 ;
	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (pq_remtail) */


/* we apply some special care here to make sure we actually were in the Q */
int pq_unlink(PQ *qhp,PQ_ENT *ep)
{
	PQ_ENT		*nep, *pep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
#endif

	if (ep->next != NULL) {

	    if (ep->prev != NULL) {
	        nep = ep->next ;
	        nep->prev = ep->prev ;
		pep = ep->prev ;
		pep->next = ep->next ;
	    } else {
		if (qhp->head == ep) {
	            nep = ep->next ;
	            nep->prev = NULL ;
	            qhp->head = nep ;
		} else {
		    rs = SR_NOANODE ;
		}
	    }

	} else {

	    if (ep->prev != NULL) {
		if (qhp->tail == ep) {
		    pep = ep->prev ;
		    pep->next = NULL ;
	            qhp->tail =  pep ;
		} else {
		    rs = SR_NOANODE ;
		}
	    } else {
		if ((qhp->head == ep) && (qhp->tail == ep)) {
	            qhp->head =  NULL ;
	            qhp->tail =  NULL ;
		} else {
		    rs = SR_NOANODE ;
		}
	    }

	} /* end if */

	if (rs >= 0) qhp->count -= 1 ;
	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (pq_unlink) */


int pq_count(PQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	return qhp->count ;
}
/* end subroutine (pq_count) */


/* perform an audit */
int pq_audit(PQ *qhp)
{
	PQ_ENT		*ep, *pep ;
	int		rs = SR_OK ;

	if (qhp == NULL) return SR_FAULT ;

	if (qhp->head != NULL) {

	    ep = qhp->head ;
	    if (ep->prev != NULL)
	        rs = SR_BADFMT ;

	    if (rs >= 0) {

	        pep = ep ;
	        ep = ep->next ;
	        while (ep != NULL) {

	            if (ep->prev != pep) {
	                rs = SR_BADFMT ;
	                break ;
	            }

	            pep = ep ;
	            ep = ep->next ;

	        } /* end while */

		if ((rs >= 0) && (qhp->tail != pep)) {
	            rs = SR_BADFMT ;
		}

	    } /* end if (ok) */

	} else if (qhp->tail != NULL) {
	    rs = SR_BADFMT ;
	}

	return rs ;
}
/* end subroutine (pq_audit) */


int pq_curbegin(PQ *qhp,PQ_CUR *curp)
{
	if (qhp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->entp = NULL ;
	return SR_OK ;
}
/* end subroutine (pq_curbegin) */


int pq_curend(PQ *qhp,PQ_CUR *curp)
{
	if (qhp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->entp = NULL ;
	return SR_OK ;
}
/* end subroutine (pq_curend) */


int pq_enum(PQ *qhp,PQ_CUR *curp,PQ_ENT **rpp)
{
	int		rs = SR_NOTFOUND ;
	if (qhp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qhp->head != NULL) {
	    PQ_ENT	*nep ;
	    if (curp->entp != NULL) {
	        PQ_ENT	*ep = curp->entp ;
		nep = ep->next ;
	    } else {
	        nep = qhp->head ;
	    }
	    if (nep != NULL) rs = SR_OK ;
	    if (rpp != NULL) *rpp = nep ;
	    curp->entp = nep ;
	}
	return rs ;
}
/* end subroutine (pq_enum) */


