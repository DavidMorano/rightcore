/* isNotLib */

/* determine some kind of "not a library" condition exists */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if a library file is not present from its
        return-status.

	Synopsis:

	int isNotLib(int rs)

	Arguments:

	rs		return-status from a file access

	Returns:

	1		condition found
	1		condition not found


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

static const int	nolibs[] = {
	SR_NOENT,
	SR_ACCESS,
	SR_LIBACC,
	SR_NOTDIR,
	SR_STALE,
	0	
} ;


/* exported subroutines */


int isNotLib(int rs)
{
	return isOneOf(nolibs,rs) ;
}
/* end subroutine (isNotLib) */


