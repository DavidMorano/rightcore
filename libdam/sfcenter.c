/* sfcenter */

/* string-find the center sub-string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was adapted from others programs that did similar
	types of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a cookie key in a string.  

	Synopsis:

	int sfcenter(sp,sl,ss,rpp)
	const char	*sp ;
	int		sl ;
	const char	ss[] ;
	const char	**rpp ;

	Arguments:

	sp	supplied string to test
	sl	length of supplied string to test
	ss	the delimiter
	rpp	pointer to store result "thing" pointer

	Returns:

	>=0	length of resulting key-name
	<0	no key found

	Notes:


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnsub(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfcenter(cchar *sp,int sl,cchar *ss,cchar **rpp)
{
	int		cl = -1 ;
	const char	*cp = NULL ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl >= 2) {
	    int	sch = MKCHAR(ss[0]) ;
	    const char	*tp ;
	    if ((tp = strnchr(sp,sl,sch)) != NULL) {
	        sch = MKCHAR(ss[1]) ;
		cp = (tp+1) ;
	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if ((tp = strnchr(sp,sl,sch)) != NULL) {
	            cl = (tp-sp) ;
	        }
	    }
	} /* end if */

	if (rpp != NULL) {
	    *rpp = (cl >= 0) ? cp : NULL ;
	}

	return cl ;
}
/* end subroutine (sfcenter) */


