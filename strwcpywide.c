/* strwcpywide */

/* copy a wide-string to narrow string buffer */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Copy a source wide-string to a destination narrow-string buffer until
        either the end of the source wide-string is reached (by its end-marker)
        or the length of the source wide-string is exhausted.

	Synopsis:

	char *strwcpywide(dp,sp,sl)
	char		*dp ;
	const wchar_t	*sp ;
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
#include	<limits.h>
#include	<stddef.h>
#include	<localmisc.h>


/* exported subroutines */


char *strwcpywide(char *dp,const wchar_t *sp,int sl)
{
	int		ch ;
	if (sl >= 0) {
	    while (sl-- && *sp) {
		if ((ch = (int) *sp++) >= UCHAR_MAX) ch = '¿' ;
	        *dp++ = (char) ch ;
	    }
	} else {
	    while (*sp) {
		if ((ch = (int) *sp++) >= UCHAR_MAX) ch = '¿' ;
	        *dp++ = (char) ch ;
	    }
	} /* end if */
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpywide) */


