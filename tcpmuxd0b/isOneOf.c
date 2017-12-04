/* isOneOf */

/* determine if a value is one of those in a given array  */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if a value (given by the second argument) is
        present within the array (given by the first argument).

	Synopsis:

	int isOneOf(const int *rets,int rs)

	Arguments:

	rets		array of (constant) integers to check against
	rs		return-status from a file access

	Returns:

	1		matched (TRUE)
	0		did not match (FALSE)


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isOneOf(const int *a,int rs)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; a[i] != 0 ; i += 1) {
	    f = (rs == a[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (isOneOf) */


