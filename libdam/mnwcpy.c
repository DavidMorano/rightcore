/* mnwcpy */

/* special (excellent) memory-copy type of subroutine! */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Ths subroutine is a cross between 'sncpy1(3dam)' and 'memcpy(3c)' but it
        takes a counted string for the source rather than only a NUL-terminated
        string.

	Synopsis:

	int mnwcpy(dp,dl,sp,sl)
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

	>=0		number of bytes in result
	<0		error


	Notes:

        This subroutine just calls either 'memcpy(3c)', 'sncpy1(3dam)', or
        'strwcpy(3dam)' based on the arguments. The advantage of this subroutine
        over the others is that the logic needed to figure out just what
        subroutine to call is coded into this subroutine already. We try to use
        the most efficient string-copy operation that we can given the passed
        arguments, while also tracking whether a buffer overflow could occur.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>		/* extra types */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpyfc(char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strwcpyfc(char *,const char *,int) ;


/* external variables */


/* exported subroutines */


int mnwcpy(char *dp,int dl,cchar *sp,int sl)
{
	int		rs ;

	if (dl >= 0) {
	    if (sl >= 0) {
	        if (sl <= dl) {
		    memcpy(dp,sp,sl) ;
		    rs = sl ;
	        } else {
	            rs = SR_OVERFLOW ;
		}
	    } else {
	        rs = sncpy1(dp,dl,sp) ;
	    }
	} else {
	    if (sl >= 0) {
		memcpy(dp,sp,sl) ;
		rs = sl ;
	    } else {
	        rs = strwcpy(dp,sp,-1) - dp ;
	    }
	}

	return rs ;
}
/* end subroutine (mnwcpy) */



