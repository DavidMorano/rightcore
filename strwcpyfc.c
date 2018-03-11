/* strwcpyfc */

/* copy a string to a maximum extent */


#define	CF_CHAR		1		/* use 'char(3dam)' */


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

	char *strwcpyfc(dp,sp,sl)
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
#include	<localmisc.h>


/* local defines */

#if	defined(CF_CHAR) && (CF_CHAR == 1)
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


char *strwcpyfc(char *dp,cchar *sp,int sl)
{
	if (sl >= 0) {
	    while (sl-- && *sp) *dp++ = tofc(*sp++) ;
	} else {
	    while (*sp) *dp++ = tofc(*sp++) ;
	} /* end if */
	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpyfc) */


