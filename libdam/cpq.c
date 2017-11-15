/* cpq */
/* Circular-Pointer-Queue */

/* a regular (no-frills) pointer queue */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This code was modeled after assembly.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a regular, pointer based, no-frills circular doubly linked
        list queue. Note that this object CANNOT be moved (copied) since there
        may be pointers pointing back at the list head (located in the object).


*******************************************************************************/


#define	CPQ_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>

#include	"cpq.h"


/* exported subroutines */


/* initialize the queue */
int cpq_start(CPQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	qhp->next = (CPQ_ENT *) qhp ;
	qhp->prev = (CPQ_ENT *) qhp ;

	return SR_OK ;
}
/* end subroutine (cpq_start) */


/* free up a queue (destroy it) */
int cpq_finish(CPQ *qhp)
{

	if (qhp == NULL) return SR_FAULT ;

	qhp->next = NULL ;
	qhp->prev = NULL ;

	return SR_OK ;
}
/* end subroutine (cpq_finish) */


/* insert into queue (at the tail) */
int cpq_ins(CPQ *qhp,CPQ_ENT *ep)
{
	CPQ_ENT		*hp = (CPQ_ENT *) qhp ;
	CPQ_ENT		*ep2 ;

	if (qhp == NULL) return SR_FAULT ;

	ep2 = hp->prev ;
	ep2->next = ep ;
	ep->next = hp ;
	ep->prev = ep2 ;
	qhp->prev = ep ;

	return SR_OK ;
}
/* end subroutine (cpq_ins) */


/* insert a group into queue (at the tail) */
int cpq_insgroup(CPQ *qhp,CPQ_ENT *gp,int esize,int n)
{

	if (qhp == NULL) return SR_FAULT ;

	if (n >= 1) {
	    CPQ_ENT	*hp = (CPQ_ENT *) qhp ;
	    CPQ_ENT	*ep, *pep, *nep ;
	    caddr_t	p = (caddr_t) gp ;
	    int		i ;
	    pep = hp->prev ;
	    for (i = 0 ; i < n ; i += 1) {
	        ep = (CPQ_ENT *) p ;
	        pep->next = ep ;
	        ep->prev = pep ;
	        pep = ep ;
	        p += esize ;
	    } /* end for */
	    pep->next = hp ;
	    qhp->prev = pep ;
	} /* end if */

	return SR_OK ;
}
/* end subroutine (cpq_insgroup) */


/* remove from queue (remove from head) */
int cpq_rem(CPQ *qhp,CPQ_ENT **epp)
{
	CPQ_ENT		*hp = (CPQ_ENT *) qhp ;
	CPQ_ENT		*ep, *ep2 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->next == hp) return SR_NOENT ;

	ep = qhp->next ;
	ep2 = ep->next ;
	ep2->prev = hp ;
	qhp->next = ep2 ;

	*epp = ep ;

	return (qhp->next != hp) ? 1 : 0 ;
}
/* end subroutine (cpq_rem) */


/* remove from the TAIL of queue (to get "stack-like" behavior) */
int cpq_remtail(CPQ *qhp,CPQ_ENT **epp)
{
	CPQ_ENT		*hp = (CPQ_ENT *) qhp ;
	CPQ_ENT		*ep, *ep2 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->next == hp) return SR_NOENT ;

	ep = qhp->prev ;
	ep2 = ep->prev ;
	ep2->next = hp ;
	qhp->prev = ep2 ;

	*epp = ep ;

	return (qhp->next != hp) ? 1 : 0 ;
}
/* end subroutine (cpq_remtail) */


/* get the pointer of the entry at the TAIL of queue */
int cpq_gettail(CPQ *qhp,CPQ_ENT **epp)
{
	CPQ_ENT		*hp = (CPQ_ENT *) qhp ;
	CPQ_ENT		*ep, *ep2 ;

	if (qhp == NULL) return SR_FAULT ;
	if (qhp->next == hp) return SR_NOENT ;

	*epp = qhp->prev ;

	return (qhp->next != hp) ? 1 : 0 ;
}
/* end subroutine (cpq_gettail) */


/* perform an audit */
int cpq_audit(CPQ *qhp)
{
	CPQ_ENT		*hp = (CPQ_ENT *) qhp ;
	CPQ_ENT		*ep, *ep2 ;
	int		rs = SR_OK ;

	if (qhp == NULL) return SR_FAULT ;

	if ((qhp->next == NULL) || (qhp->prev == NULL))
	    return SR_NOTOPEN ;

	ep = qhp->next ;
	ep2 = hp ;
	while ((rs >= 0) && (ep != hp)) {

	    if (ep->prev == ep2) {
	        ep2 = ep ;
	        ep = ep->next ;
	    } else {
		rs = SR_BADFMT ;
	    }

	} /* end while */

	if ((rs >= 0) && (ep->prev != ep2)) {
	    rs = SR_BADFMT ;
	}

	return rs ;
}
/* end subroutine (cpq_audit) */


