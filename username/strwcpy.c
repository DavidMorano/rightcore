/* strwcpy */

/* copy a string to a maximum extent */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Copy a source string to a destination until either the end of the source
        string is reached (by its end-marker) or the length of the source string
        is exhausted.

	Synopsis:

	char *strwcpy(dp,sp,sl)
	char		*dp ;
	const char	*sp ;
	int		sl ;

	Arguments:

	dp	string buffer that receives the copy
	sp	the source string that is to be copied
	sl	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* MUST be first to configure */


/* exported subroutines */


char *strwcpy(char *dp,cchar *sp,int sl)
{

	if (sl >= 0) {
	    while (sl-- && *sp) *dp++ = *sp++ ;
	} else {
	    while (*sp) *dp++ = *sp++ ;
	} /* end if */

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpy) */


