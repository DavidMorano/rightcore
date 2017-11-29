/* strnwcpy */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Ths subroutine is similar to 'strncpy(3c)' (in that it zero-fills out
        any remainder of the destiation string buffer which is not otherwise
        filled up by characters from the source string) but it takes a counted
        string for the source rather than only a NUL-terminated string. The
        arguments to this are also different than from 'strncpy(3c)'. It also
        returns a pointer to the end of the filled in (non-NUL) part of the
        destination string (created) rather than to the beginning of it. No NUL
        character is placed at the end (overlast-end) of the destination string.

	Synopsis:

	char *strnwcpy(dp,dl,sp,sl)
	char		*dp ;
	int		dl ;
	const char	*sp ;
	int		sl ;

	Arguments:

	dp		destination string buffer
	dl		destination string buffer length
	sp		source string
	sl		source string length

	Returns:

	-		pointer to the end of the filled in part
			of the created destination string


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


char *strnwcpy(char *dp,int dl,cchar *sp,int sl)
{

	if (dl >= 0) {
	    if (sl >= 0) {
		const int	ml = MIN(sl,dl) ;
	        strncpy(dp,sp,ml) ;
		dp += ml  ;
		dl -= ml ;
	    } else {
		while (dl && *sp) {
		    *dp++ = *sp++ ;
		    dl -= 1 ;
		}
	    }
	    if (dl > 0) {
		memset(dp,0,dl) ;
	    }
	} else {
	    dp = strwcpy(dp,sp,sl) ;
	}

	return dp ;
}
/* end subroutine (strnwcpy) */


