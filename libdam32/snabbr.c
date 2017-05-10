/* snabbr */

/* copy an abbreviation of a groups of words */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy an abbreviation of the given string to the destination.

	Synopsis:

	int snabbr(dp,dl,sp,sl)
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


	See-also:

	snwcpy(3dam),
	snwcpylatin(3dam), 
	snwcpyopaque(3dam), 
	snwcpycompact(3dam), 
	snwcpyclean(3dam), 
	snwcpyhyphen(3dam), 
	snwcpylc(3dam),
	snwcpyuc(3dam),
	snwcpyfc(3dam),


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpyfc(char *,int,const char *) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	touc(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strwcpyfc(char *,const char *,int) ;


/* external variables */


/* exported subroutines */


int snabbr(char *dp,int dl,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		i = 0 ;
	const char	*cp ;

	if (dl < 0) dl = INT_MAX ;
	if (sl < 0) sl = strlen(sp) ;

	while ((cl = sfnext(sp,sl,&cp)) > 0) {
	    if (i < dl) {
	        dp[i++] = touc(cp[0]) ;
	    } else {
		rs = SR_OVERFLOW ;
	    }
	    sl -= ((cp+cl)-sp) ;
	    sp = (cp+cl) ;
	    if (rs < 0) break ;
	} /* end while */
	dp[i] = '\0' ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snabbr) */


