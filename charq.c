/* charq */

/* character queue module */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* extra safety */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This obejct module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implement a character queue object.


*******************************************************************************/


#define	CHARQ_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"charq.h"


/* external subroutines */


/* local structures */


/* forward references */


/* exported subroutines */


int charq_start(CHARQ *cqp,int size)
{
	int		rs ;

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (size <= 1)
	    return SR_INVALID ;

	if ((rs = uc_malloc(size,&cqp->buf)) >= 0) {
	    cqp->size = size ;
	    cqp->count = 0 ;
	    cqp->ri = cqp->wi = 0 ;
	}

	return rs ;
}
/* end subroutine (charq_start) */


int charq_finish(CHARQ *cqp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (cqp->buf != NULL) {
	    rs1 = uc_free(cqp->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    cqp->buf = NULL ;
	}

	cqp->size = 0 ;
	cqp->count = 0 ;
	return rs ;
}
/* end subroutine (charq_finish) */


/* subroutine to insert into FIFO */
int charq_ins(CHARQ *cqp,int ch)
{
	int		rs = SR_OVERFLOW ;

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (cqp->count < cqp->size) {
	    (cqp->buf)[cqp->wi] = ch ;
	    cqp->wi += 1 ;
	    if (cqp->wi == cqp->size) cqp->wi = 0 ;
	    cqp->count += 1 ;
	    rs = cqp->count ;
	}

	return rs ;
}
/* end subroutine (charq_ins) */


/* remove from FIFO */
int charq_rem(CHARQ *cqp,char *cp)
{
	int		rs = SR_EMPTY ;

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (cqp->count > 0) {
	    if (cp != NULL) *cp = (cqp->buf)[cqp->ri] ;
	    cqp->ri += 1 ;
	    if (cqp->ri == cqp->size) cqp->ri = 0 ;
	    cqp->count -= 1 ;
	    rs = cqp->count ;
	}

	return rs ;
}
/* end subroutine (charq_rem) */


int charq_size(CHARQ *cqp)
{

	if (cqp == NULL) return SR_FAULT ;

	return cqp->size ;
}
/* end subroutine (charq_size) */


int charq_count(CHARQ *cqp)
{

	if (cqp == NULL) return SR_FAULT ;

	return cqp->count ;
}
/* end subroutine (charq_count) */


