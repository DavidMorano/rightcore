/* strwcpyrev */

/* copy a string to a maximum extent */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Copy a source string to a destination until either the end of the source
        string is reached (by its end-marker) or the length of the source string
        is exhausted.

	Synopsis:

	char *strwcpyrev(dp,sp,sl)
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
#include	<stdlib.h>
#include	<string.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


char *strwcpyrev(char *dp,cchar *sp,int sl)
{
	int		i ;
	if (sl < 0) sl = strlen(sp) ;
	for (i = (sl-1) ; i >= 0 ; i += 1) {
	    *dp++ = sp[i] ;
	} /* end for */
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpyrev) */


