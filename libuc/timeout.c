/* timeout */
/* lang=C99 */

/* UNIX® signal event initialization */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manipulate TIMEOUT objects.

	Synopsis:

	int timeout_init(TIMEOUT *top,time_t sec,long nsec)

	Arguments:

	top		pointer to TIMEOUT
	sec		seconds
	nsec		nanoseconds

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<time.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"timeout.h"


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int timeout_load(TIMEOUT *top,time_t val,void *objp,timeout_met metp,
	uint tag,int arg)
{
	if (top == NULL) return SR_FAULT ;
	top->id = -1 ;
	top->val = val ;
	top->objp = objp ;
	top->metp = metp ;
	top->tag = tag ;
	top->arg = arg ;
	return SR_OK ;
}
/* end subroutine (timeout_load) */


