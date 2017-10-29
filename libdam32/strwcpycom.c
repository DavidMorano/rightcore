/* strwcpycom */

/* copy a string to a maximum extent w/ white-space compression */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We copy a source string to a destination until either the end of the
	source string is reached (by its end-marker) or the length of the
	source string is exhausted.  During the copy two additional steps are
	performed:

	1) white-space characters are removed
	2) upper-case characters are converted to lowercase

	Synopsis:

	char *strwcpycom(dp,sp,sl)
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

#include	<char.h>


/* local defines */

#if	defined(CF_CHAR) && (CF_CHAR > 0)
#define	CF_CHAREXTERN	0
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#define	tofc(c)		CHAR_TOFC(c)
#else
#define	CF_CHAREXTERN	1
#endif


/* external subroutines */

#if	CF_CHAREXTERN
extern int	tolc(int) ;
extern int	touc(int) ;
extern int	tofc(int) ;
#endif


/* exported subroutines */


char *strwcpycom(char *dp,const char *sp,int sl)
{

	if (sl >= 0) {
	    while (sl && (*sp != '\0')) {
		if (! CHAR_ISWHITE(*sp)) *dp++ = tolc(*sp) ;
		sp += 1 ;
		sl -= 1 ;
	    }
	} else {
	    while (*sp != '\0') {
		if (! CHAR_ISWHITE(*sp)) *dp++ = tolc(*sp) ;
		sp += 1 ;
	    }
	} /* end if */

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpycom) */


