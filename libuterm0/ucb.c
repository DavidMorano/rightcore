/* ucb */

/* Unit Control Block (a "driver" thing) */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Audix Database Processor (DBP) work.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<charq.h>

#include	"ucb.h"


/* local defines */

#define	UCB_MAGIC	0x33229161


/* exported subroutines */


int ucb_start(op,fd)
UCB	*op ;
int	fd ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (fd < 0)
	    return SR_INVALID ;

	memset(op,0,sizeof(UCB)) ;

	op->fd = fd ;
	op->magic = UCB_MAGIC ;
	return SR_OK ;
}
/* end subroutine (ucb_start) */


int ucb_finish(op)
UCB	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UCB_MAGIC)
	    return SR_NOTOPEN ;

	charq_finish(&op->taq) ;

	charq_finish(&op->ecq) ;

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (ucb_finish) */



