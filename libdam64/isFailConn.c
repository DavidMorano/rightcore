/* isFailConn */

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

	int isFailConn(int rs)

	Arguments:

	rs		return-status

	Returns:

	1		condition found
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

static const int	rsnoconn[] = {
	SR_NETDOWN,
	SR_NETUNREACH,
	SR_HOSTDOWN,
	SR_HOSTUNREACH,
	SR_CONNREFUSED,
	SR_NOTCONN,
	SR_CONNRESET,
	SR_NOENT,
	SR_PIPE,
	SR_TIMEDOUT,
	0	
} ;


/* exported subroutines */


int isFailConn(int rs)
{
	int		f = FALSE ;
	if (rs < 0) {
	    f = isOneOf(rsnoconn,rs) ;
	}
	return f ;
}
/* end subroutine (isFailConn) */


