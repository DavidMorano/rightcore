/* isIOError */

/* determine if an I-O error has been given */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a return-status is an I-O error.

	Synopsis:

	int isIOError(int rs)

	Arguments:

	rs		return-status from a file access

	Returns:

	1		matched I-O error (TRUE)
	0		did not match (FALSE)


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

static const int	ioerrors[] = {
	SR_IO,
	SR_NXIO,
	SR_PIPE,
	0	
} ;


/* exported subroutines */


int isIOError(int rs)
{
	return isOneOf(ioerrors,rs) ;
}
/* end subroutine (isIOError) */


