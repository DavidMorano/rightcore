/* strwcpyblanks */

/* copy a string to a maximum extent */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy blank characters to a destination for the number of characters
	specified.  The resulting string is NUL-terminated.

	Synopsis:

	char *strwcpyblanks(dp,w)
	char		*dp ;
	int		w ;

	Arguments:

	dp	string buffer that receives the copy
	w	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


/* exported subroutines */


char *strwcpyblanks(char *dp,int w)
{
	while (w-- > 0) *dp++ = ' ' ;
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpyblanks) */


