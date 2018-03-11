/* strncpyblanks */

/* copy a string to a maximum extent */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy blank characters to a destination for the number of characters
	specified.  The resulting string is *not* NUL-terminated.

	Synopsis:

	char *strncpyblanks(dst,n)
	char		dst[] ;
	int		n ;

	Arguments:

	dst	string buffer that receives the copy
	n	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


/* exported subroutines */


char *strncpyblanks(char *dp,int n)
{
	while (n-- > 0) *dp++ = ' ' ;
	return dp ;
}
/* end subroutine (strncpyblanks) */


