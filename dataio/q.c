/* q */

/* some sort of queue object for OS stuff! */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object implements a self relative queue for shared memory
	applications where the shared memory segments have different starting
	addresses in two or more address spaces.  This does NOT use any
	pointers back to the queue-header, so therefore the object (header) CAN
	be moved without problems.

	+ self-relative
	+ relocatable head


*******************************************************************************/


#define	Q_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptma.h>

#include	"q.h"


/* exported subroutines */


int q_start(Q *qhp,int type)
{
	PTMA		ma ;
	int		rs = SR_OK ;

	qhp->head = qhp->tail = 0 ;
	qhp->head = qhp->tail = 0 ;
	if ((rs = ptma_create(&ma)) >= 0) {
	    int		matype = PTHREAD_PROCESS_PRIVATE ;
	    if (type > 0) matype = PTHREAD_PROCESS_SHARED ;
	    if ((rs = ptma_setpshared(&ma,matype)) >= 0) {
	        rs = ptm_create(&qhp->lock,&ma) ;
	    }
	    ptma_destroy(&ma) ;
	} /* end if (mutex-attributes) */

	return rs ;
}
/* end subroutine (q_start) */


/* free up a queue */
int q_finish(Q *qhp)
{
	Q_ENT		*ep ;
	int		rs ;

	while (qhp->head != 0) {
	    q_rem(qhp,&ep) ;
	}

	rs = ptm_destroy(&qhp->lock) ;

	return rs ;
}
/* end subroutine (q_finish) */


/* insert into queue (at the tail) */
int q_ins(Q *qhp,Q_ENT *ep)
{
	Q_ENT		*ep2 ;
	int		rs ;
	int		rc = 0 ;

	if ((rs = ptm_lock(&qhp->lock)) >= 0) {

	    if (qhp->head == 0) {

	        ep->next = ep->prev = 0 ;

	        qhp->head = qhp->tail = (ep - ((Q_ENT *) qhp)) ;
	        rc = 1 ;

	    } else {

	        ep2 = qhp->tail + ((Q_ENT *) qhp) ;
	        ep2->next = ep - ((Q_ENT *) qhp) ;

	        ep->prev = ep2 - ((Q_ENT *) qhp) ;
	        ep->next = 0 ;

	        qhp->tail = ep - ((Q_ENT *) qhp) ;
	        rc = 2 ;

	    } /* end if */

	    ptm_unlock(&qhp->lock) ;
	} /* end if (mutex lock) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (q_ins) */


/* remove from queue (remove from head) */
int q_rem(Q *qhp,Q_ENT **epp)
{
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (qhp->head != 0) {
	    if ((rs = ptm_lock(&qhp->lock)) >= 0) {
	        *epp = qhp->head + ((Q_ENT *) qhp) ;
	        qhp->head = (*epp)->next ;
	        if (qhp->head == 0) {
	            rc = 0 ;
	            qhp->tail = 0 ;
	        } else {
	            rc = 1 ;
	        }
	        ptm_unlock(&qhp->lock) ;
	    } /* end if (mutex lock) */
	} else {
	    rs = SR_NOENT ;
	}

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (q_rem) */


