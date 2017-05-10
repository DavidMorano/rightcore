/* strnset */

/* set a string to a number of characters */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy a specified character to a destination for the number of
	characters specified.  The resulting string is *not* NUL-terminated.

	Synopsis:

	char *strnset(s1,ch,n)
	char		s1[] ;
	int		ch ;
	int		n ;

	Arguments:

	s1	string buffer that receives the copy
	ch	character to set in the destination string
	n	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


	Note: This subroutine is similar to:
		void *memset(s,ch,n) ;
	except that a pointer to the end of the string is returned
	instead of a pointer to the beginning of the string!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


char *strnset(char *dp,int ch,int n)
{
	while (n-- > 0) *dp++ = ch ;
	return dp ;
}
/* end subroutine (strnset) */


