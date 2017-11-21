/* cq */
/* Container-Queue */

/* container Q */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-17, David A­D­ Morano
	Oh what a cheap Q!  I do not know why I am doing this!

	= 2017-11-21, David A­D­ Morano
	Added new method |cq_unlink()|.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implement a simple cheap queue (CQ).


*******************************************************************************/


#define	CQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<localmisc.h>

#include	"cq.h"


/* local defines */


/* exported subroutines */


int cq_start(CQ *cqp)
{
	const int	vo = (VECHAND_OORDERED | VECHAND_OCOMPACT) ;
	const int	de = CQ_DEFENTS ;
	int		rs ;

	if (cqp == NULL) return SR_FAULT ;

	if ((rs = vechand_start(&cqp->q,de,vo)) >= 0) {
	    cqp->magic = CQ_MAGIC ;
	}

	return rs ;
}
/* end subroutine (cq_start) */


int cq_finish(CQ *cqp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cqp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	rs1 = vechand_finish(&cqp->q) ;
	if (rs >= 0) rs = rs1 ;

	cqp->magic = 0 ;
	return rs ;
}
/* end subroutine (cq_finish) */


/* insert at tail */
int cq_ins(CQ *cqp,void *ep)
{
	int		rs ;

	if (cqp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	rs = vechand_add(&cqp->q,ep) ;

	return rs ;
}
/* end subroutine (cq_ins) */


/* remove from head */
int cq_rem(CQ *cqp,void *vp)
{
	int		rs ;
	int		count = 0 ;

	if (cqp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = vechand_get(&cqp->q,0,vp)) >= 0) {
	    vechand_del(&cqp->q,0) ;
	    count = vechand_count(&cqp->q) ;
	}

	return (rs >= 0) ? count : rs ;
}
/* end subroutine (cq_rem) */


/* unlink entry from where ever it is (if it is) */
int cq_unlink(CQ *cqp,void *vp)
{
	int		rs ;
	int		count = 0 ;

	if (cqp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	if ((rs = vechand_ent(&cqp->q,vp)) >= 0) {
	    vechand_del(&cqp->q,0) ;
	    count = vechand_count(&cqp->q) ;
	}

	return (rs >= 0) ? count : rs ;
}
/* end subroutine (cq_unlink) */


int cq_count(CQ *cqp)
{
	int		rs ;

	if (cqp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	rs = vechand_count(&cqp->q) ;

	return rs ;
}
/* end subroutine (cq_count) */


int cq_curbegin(CQ *cqp,CQ_CUR *curp)
{

	if (cqp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (cq_curbegin) */


int cq_curend(CQ *cqp,CQ_CUR *curp)
{

	if (cqp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (cq_curend) */


int cq_enum(CQ *cqp,CQ_CUR *curp,void *vp)
{
	int		rs ;
	int		i ;
	void		*rp ;

	if (cqp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (cqp->magic != CQ_MAGIC) return SR_NOTOPEN ;

	if (vp == NULL)
	    vp = &rp ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	if ((rs = vechand_get(&cqp->q,i,vp)) >= 0) {
	    curp->i = i ;
	}

	return rs ;
}
/* end subroutine (cq_enum) */


