/* plainq */

/* some sort of queue object for OS stuff! */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* extra safety */
#define	CF_SAFE2	1		/* extra safety (level 2) */
#define	CF_AUDIT	1		/* extended audit */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This object implements a self-relative doublely linked list for queue
        operations. This type of queue can be used in shared memory areas that
        are mapped to different addresses in different address spaces! That is
        the whole idea of this type of a queue. This means that all queue
        entries are stored with relative positions from the head of the queue.
        This is done so that multiple processes (in different address spaces)
        can access the queue (once proper mutual exclusion is provided).

        This object CAN be used in applications where the object is moved
        (copied) in memory since no pointers (whatever) point back to the
        header.

	Important note:

        This type of queue (used for system OS or other executive type purposes)
        requires that all entries start with the structure 'PLAINQ_ENT'. This is
        necessary because that structure is used for the linking of the entries
        into the Q. This is not a container object in the normal sense since
        access inside entries is required for operation! If you want a container
        Q that does not have to access the entry object then pick some other Q
        object to use instead of this one!


******************************************************************************/


#define	PLAINQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>

#include	"plainq.h"


/* local defines */

#define	PLAINQ_MAGIC	0x76925634


/* exported subroutines */


/* initialize a queue head */
int plainq_start(PLAINQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	qhp->head = 0 ;
	qhp->tail = 0 ;
	qhp->count = 0 ;
	qhp->magic = PLAINQ_MAGIC ;
	return SR_OK ;
}
/* end subroutine (plainq_start) */


/* free up a queue */
int plainq_finish(PLAINQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;

	qhp->head = 0 ;
	qhp->tail = 0 ;
	qhp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (plainq_finish) */


/* insert into queue (at the tail) */
int plainq_ins(PLAINQ *qhp,PLAINQ_ENT *ep)
{
	PLAINQ_ENT	*pep ;		/* previous entry at tail */

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (ep == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	if (qhp->head == 0) {

	    qhp->head = ((long) ep) - ((long) qhp) ;
	    ep->prev = 0 ;

	} else {

	    pep = (PLAINQ_ENT *) (qhp->tail + ((long) qhp)) ;
	    pep->next = ((long) ep) - ((long) qhp) ;
	    ep->prev = ((long) pep) - ((long) qhp) ;

	} /* end if */

	ep->next = 0 ;
	qhp->tail = ((long) ep) - ((long) qhp) ;
	qhp->count += 1 ;
	return qhp->count ;
}
/* end subroutine (plainq_ins) */


/* insert a group of entries into the queue (at the tail) */
int plainq_insgroup(PLAINQ *qhp,PLAINQ_ENT *gp,int esize,int n)
{
	PLAINQ_ENT	*ep ;
	int		i ;
	caddr_t		p = (caddr_t) gp ; /* convert for addition */

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (gp == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	for (i = 0 ; i < n ; i += 1) {
	    ep = (PLAINQ_ENT *) p ;
	    plainq_ins(qhp,ep) ;
	    p += esize ;
	} /* end for */

	return qhp->count ;
}
/* end subroutine (plainq_insgroup) */


/* we apply a little care here to make sure that the entry was in the Q */
int plainq_unlink(PLAINQ *qhp,PLAINQ_ENT *ep)
{
	PLAINQ_ENT	*nep, *pep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (ep == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	if (ep->next != 0) {

	    nep = ((PLAINQ_ENT *) qhp) + ep->next ;
	    if (ep->prev != 0) {
	        nep->prev = ep->prev ;
		pep = ((PLAINQ_ENT *) qhp) + ep->prev ;
		pep->next = ep->next ;
	    } else {
		long	eo = ep - ((PLAINQ_ENT *) qhp) ;
		if (qhp->head == eo) {
	            nep->prev = ep->prev ;
	            qhp->head = nep - ((PLAINQ_ENT *) qhp) ;
		} else {
		    rs = SR_NOANODE ;
		}
	    }

	} else {
	    long	eo = ep - ((PLAINQ_ENT *) qhp) ;

	    if (ep->prev != 0) {
		pep = ((PLAINQ_ENT *) qhp) + ep->prev ;
		if (qhp->tail == eo) {
		    pep->next = 0 ;
	            qhp->tail =  pep - ((PLAINQ_ENT *) qhp) ;
		} else {
		    rs = SR_NOANODE ;
		}
	    } else {
		if ((qhp->head == eo) && (qhp->tail == eo)) {
	            qhp->head =  0 ;
	            qhp->tail =  0 ;
		} else {
		    rs = SR_NOANODE ;
		}
	    }

	} /* end if */

	if (rs >= 0) {
	    ep->next = 0 ;
	    ep->prev = 0 ;
	    qhp->count -= 1 ;
	}

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_unlink) */


/* remove from queue (remove from head) */
int plainq_rem(PLAINQ *qhp,PLAINQ_ENT **epp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (epp == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	if (qhp->head != 0) {
	    PLAINQ_ENT	*ep = (PLAINQ_ENT *) (qhp->head + ((long) qhp)) ;
	    if (ep->prev == 0) {
	        *epp = ep ;
	        qhp->head = (*epp)->next ;
	        if (qhp->head == 0) qhp->tail = 0 ;
	        ep->next = 0 ;
	        qhp->count -= 1 ;
	    } else {
	        rs = SR_NOANODE ;
	    }
	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_rem) */


/* get the pointer to the head entry */
int plainq_gethead(PLAINQ *qhp,PLAINQ_ENT **epp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (epp == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	*epp = NULL ;
	if (qhp->head != 0) {
	    PLAINQ_ENT	*ep = (PLAINQ_ENT *) (qhp->head + ((long) qhp)) ;
	    if (ep->prev == 0) {
	        *epp = ep ;
	    } else {
		rs = SR_NOANODE ;
	    }
	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_gethead) */


/* remove from queue (remove from tail) */
int plainq_remtail(PLAINQ *qhp,PLAINQ_ENT **epp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (epp == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	*epp = NULL ;
	if (qhp->head != 0) {
	    PLAINQ_ENT	*ep = (PLAINQ_ENT *) (qhp->tail + ((long) qhp)) ;
	    if (ep->next == 0) {
	        *epp = ep ;
	        qhp->tail = (*epp)->prev ;
	        if (qhp->tail == 0) qhp->head = 0 ;
	        ep->prev = 0 ;
	        qhp->count -= 1 ;
	    } else {
	        rs = SR_NOANODE ;
	    }
	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_remtail) */


/* get the pointer to the tail entry */
int plainq_gettail(PLAINQ *qhp,PLAINQ_ENT **epp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_SAFE2
	if (epp == NULL) return SR_FAULT ;
#endif /* CF_SAFE2 */

	*epp = NULL ;
	if (qhp->head != 0) {
	    PLAINQ_ENT	*ep = (PLAINQ_ENT *) (qhp->tail + ((long) qhp)) ;
	    if (ep->next == 0) {
		*epp = ep ;
	    } else {
		rs = SR_NOANODE ;
	    }
	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_gettail) */


/* how many */
int plainq_count(PLAINQ *qhp)
{

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return qhp->count ;
}
/* end subroutine (plainq_count) */


/* audit */
int plainq_audit(PLAINQ *qhp)
{
	PLAINQ_ENT	*ep, *nep ;
	long		next ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != PLAINQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (qhp->head != 0) {
	    int	c ;

#if	CF_AUDIT

	    c = 1 ;
	    next = qhp->head ;
	    nep = (PLAINQ_ENT *) (next + ((long) qhp)) ;

	    while (next != 0) {

		ep = nep ;
		c += 1 ;

	        next = ep->next ;
	        nep = (PLAINQ_ENT *) (next + ((long) qhp)) ;

	    } /* end while */

	    if (qhp->count != c) {
	        rs = SR_BADFMT ;
	    }

#endif /* CF_AUDIT */

	} else if (qhp->count != 0) {
	    rs = SR_BADFMT ;
	}

#if	CF_DEBUGS
	debugprintf("plainq_audit: rs=%d c=%u\n",rs,qhp->count) ;
#endif

	return (rs >= 0) ? qhp->count : rs ;
}
/* end subroutine (plainq_audit) */


