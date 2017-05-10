/* charq */

/* character queue module */


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
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"charq.h"


/* external subroutines */


/* local structures */


/* forward references */


/* exported subroutines */


int charq_start(cqp,size)
CHARQ		*cqp ;
int		size ;
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


int charq_finish(cqp)
CHARQ		*cqp ;
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
int charq_ins(cqp,ch)
CHARQ		*cqp ;
int		ch ;
{

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (cqp->count >= cqp->size)
	    return SR_OVERFLOW ;

	(cqp->buf)[cqp->wi] = ch ;

	cqp->wi += 1 ;
	if (cqp->wi == cqp->size)
	    cqp->wi = 0 ;

	cqp->count += 1 ;
	return cqp->count ;
}
/* end subroutine (charq_ins) */


/* remove from FIFO */
int charq_rem(cqp,cp)
CHARQ		*cqp ;
char		*cp ;
{

#if	CF_SAFE
	if (cqp == NULL) return SR_FAULT ;
#endif

	if (cqp->count == 0)
	    return SR_EMPTY ;

	if (cp != NULL)
	    *cp = (cqp->buf)[cqp->ri] ;

	cqp->ri += 1 ;
	if (cqp->ri == cqp->size)
	    cqp->ri = 0 ;

	cqp->count -= 1 ;
	return cqp->count ;
}
/* end subroutine (charq_rem) */


int charq_size(cqp)
CHARQ		*cqp ;
{

	if (cqp == NULL) return SR_FAULT ;

	return cqp->size ;
}
/* end subroutine (charq_size) */


int charq_count(cqp)
CHARQ		*cqp ;
{

	if (cqp == NULL) return SR_FAULT ;

	return cqp->count ;
}
/* end subroutine (charq_count) */


