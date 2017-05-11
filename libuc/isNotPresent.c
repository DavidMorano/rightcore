/* isNotPresent */

/* determine if a file is not present from its return-status */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a file is not present from its
	return-status.

	Synopsis:

	int isNotPresent(int rs)

	Arguments:

	rs		return-status from a file access

	Returns:

	1		file is *not* present (TRUE)
	0		file is present (FALSE)


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

static const int	nofiles[] = {
	SR_NOENT,
	SR_ACCESS,
	SR_NETUNREACH,
	SR_NETDOWN,
	SR_HOSTUNREACH,
	SR_HOSTDOWN,
	SR_TIMEDOUT,
	SR_CONNREFUSED,
	SR_LIBACC,			/* libs can be "files" also! */
	0	
} ;


/* exported subroutines */


int isNotPresent(int rs)
{
	return isOneOf(nofiles,rs) ;
}
/* end subroutine (isNotPresent) */


