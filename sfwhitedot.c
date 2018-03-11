/* sfwhitedot */

/* get a substring present before the first dot */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
        This subroutine gets the leading string before the first dot character
        (white-space cleaned up).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine retrieves the substring before the first dot character.

	Synopsis:

	int sfwhitedot(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		given string
	sl		given string length
	rpp		pointer to hold result pointer

	Returns:

	<0		error
	>=0		length of retrieved nodename


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,const char *,
			const char *,const char *) ;
extern int	snwcpylc(const char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfwhitedot(cchar *sp,int sl,cchar **rpp)
{
	const char	*tp ;

	if (sl < 0) sl = strlen(sp) ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if ((tp = strnchr(sp,sl,'.')) != NULL) {
	    sl = (tp - sp) ;
	}

	while (sl && CHAR_ISWHITE(sp[sl - 1])) {
	    sl -= 1 ;
	}

	if (rpp != NULL) {
	    *rpp = sp ;
	}

	return sl ;
}
/* end subroutine (sfwhitedot) */


