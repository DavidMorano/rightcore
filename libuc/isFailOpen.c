/* isFailOpen */

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

	int isFailOpen(int rs)

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
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const int	rsnoopen[] = {
	SR_BADF,
	0	
} ;


/* exported subroutines */


int isFailOpen(int rs)
{
	int		f = FALSE ;
	if (rs < 0) {
	    f = f || isNotPresent(rs) ;
	    f = f || isOneOf(rsnoopen,rs) ;
	}
	return f ;
}
/* end subroutine (isFailOpen) */


