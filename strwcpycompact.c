/* strwcpycompact */

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

	1. white-space characters are removed
	2. upper-case characters are converted to lowercase

	Synopsis:

	char *strwcpycompact(dp,sp,sl)
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
#include	<string.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	nextfield(cchar *,int,cchar **) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* exported subroutines */


char *strwcpycompact(char *dp,cchar *sp,int sl)
{
	int		c = 0 ;
	int		cl ;
	cchar		*cp ;
	if (sl < 0) sl = strlen(sp) ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {
	    if (c++ > 0) {
	        *dp++ = CH_SP ;
	    }
	    dp = strwcpy(dp,cp,cl) ;
	    sl -= ((cp+cl) - sp) ;
	    sp = (cp+cl) ;
	} /* end while (looping through string pieces) */
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpycompact) */


