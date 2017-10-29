/* strnlen */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine computes the length of a supplied string but will not
	examine the string for more than a specified length.

	Synopsis:

	int strnlen(s,n)
	const char	*s ;
	int		n ;

	Arguments:

	s	string to be examined
	n	maximum length of string to be examined

	Returns:

	len	mimimum of length of string or 'n' above


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int strnlen(const char *s,int n)
{
	int	i ;
	if (n >= 0) {
	    for (i = 0 ; (i < n) && *s ; i += 1) {
	        s += 1 ;
	    }
	} else
	    i = strlen(s) ;
	return i ;
}
/* end subroutine (strnlen) */


