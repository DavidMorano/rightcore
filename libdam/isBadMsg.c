/* isBadMsg */

/* determine if a return-status specifies a NotValid condition */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if an operation resulted in a bad message.

	Synopsis:

	int isBadMsg(int rs)

	Arguments:

	rs		return-status

	Returns:

	1		operation returned NotValid
	0		condition not found


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isOneOf(const int *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const int	ourerrors[] = {
	SR_BADMSG,
	SR_DOM,
	SR_RANGE,
	0	
} ;


/* exported subroutines */


int isBadMsg(int rs)
{
	return isOneOf(ourerrors,rs) ;
}
/* end subroutine (isBadMsg) */


