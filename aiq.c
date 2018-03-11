/* aiq */

/* Asynchronous Interrupt Queue - some sort of queue object for OS stuff! */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This module was originally written.

	= 1998-07-01, David A­D­ Morano
	This module was enhanced by adding the POSIX thread mutex calls.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This object implements a self-relative doublely linked list for queue
        operations. This type of queue can be used in shared memory area that
        are mapped to different addresses in different address spaces! That is
        the whole idea of this type of a queue. This queue is NOT circularly
        linked, so therefore the object (header) CAN be moved without problems.

	Important note:

        This type of queue (used for system OS or other executive type purposes)
        requires that all entries start with the structure 'AIQ_ENT'. This is
        necessary because that structure is used for the linking of the entries
        into the Q. This is not a container object in the normal sense in that
        access inside entries is required for operation! If you want a container
        Q that does not have to access the entry object then pick some other Q
        object to use instead of this one!


******************************************************************************/


#define	AIQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"aiq.h"


/* local defines */


/* exported subroutines */


/* initialize a queue head */
int aiq_start(AIQ *qhp,int type)
{
	PTMA		ma ;
	int		rs ;

	if (qhp == NULL) return SR_FAULT ;

	qhp->magic = 0 ;
	qhp->head = qhp->tail = 0 ;
	qhp->count = 0 ;
	if ((rs = ptma_create(&ma)) >= 0) {
	    int	matype = PTHREAD_PROCESS_PRIVATE ;

	    if (type > 0) matype = PTHREAD_PROCESS_SHARED ;
	    if ((rs = ptma_setpshared(&ma,matype)) >= 0) {

	        if ((rs = ptm_create(&qhp->lock,&ma)) >= 0) {
	    	    qhp->magic = AIQ_MAGIC ;
		}

	    } /* end if (ptma_setpshared) */

	    ptma_destroy(&ma) ;
	} /* end if (mutex-attributes) */

	return rs ;
}
/* end subroutine (aiq_start) */


int aiq_finish(AIQ *qhp)
{
	int		rs ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != AIQ_MAGIC) return SR_NOTOPEN ;

	rs = ptm_destroy(&qhp->lock) ;

	qhp->magic = 0 ;
	return rs ;
}
/* end subroutine (aiq_finish) */


/* insert into queue (at the tail) */
int aiq_ins(AIQ *qhp,AIQ_ENT *ep)
{
	AIQ_ENT		*ep2 ;
	SIGBLOCK	blocker ;
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;

	if (qhp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (qhp->magic != AIQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	    if ((rs = ptm_lock(&qhp->lock)) >= 0) {

	        if (qhp->head == 0) {

	            ep->next = ep->prev = 0 ;

	            qhp->head = qhp->tail = ((long) ep) - ((long) qhp) ;

	        } else {

	            ep2 = (AIQ_ENT *) (qhp->tail + ((long) qhp)) ;
	            ep2->next = ((long) ep) - ((long) qhp) ;

	            ep->prev = ((long) ep2) - ((long) qhp) ;
	            ep->next = 0 ;

	            qhp->tail = ((long) ep) - ((long) qhp) ;

	        } /* end if */

	        qhp->count += 1 ;
	        rc = qhp->count ;

	        rs1 = ptm_unlock(&qhp->lock) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (ptm) */

	    rs1 = sigblock_finish(&blocker) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (aiq_ins) */


/* remove from queue (remove from head) */
int aiq_rem(AIQ *qhp,AIQ_ENT **epp)
{
	SIGBLOCK	blocker ;
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;

	if (qhp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;
	if (qhp->magic != AIQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	    if ((rs = ptm_lock(&qhp->lock)) >= 0) {

	        if (qhp->head > 0) {
	            *epp = (AIQ_ENT *) (qhp->head + ((long) qhp)) ;
	            qhp->head = (*epp)->next ;
	            if (qhp->head == 0) {
	                qhp->tail = 0 ;
	            }
	            qhp->count -= 1 ;
	            rc = qhp->count ;
	        } else {
	            rs = SR_NOENT ;
		}

	        rs1 = ptm_unlock(&qhp->lock) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (ptm) */

	    rs1 = sigblock_finish(&blocker) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (aiq_rem) */


int aiq_count(AIQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->magic != AIQ_MAGIC) return SR_NOTOPEN ;

	return qhp->count ;
}
/* end subroutine (aiq_count) */


