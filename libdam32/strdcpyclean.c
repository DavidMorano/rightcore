/* strdcpyclean */

/* copy a source string to a destination while cleaning it up */


/* revision history:

	= 1998-07-08, David A­D­ Morano
	This was written to clean up some problems with printing garbage
	characters in a few places in some PCS utilities.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is essentially the same as the 'strdcpy(3dam)' subroutine except
	that garbage characters are replaced with a specified substitute
	character.

	Synopsis:

	int strdcpyclean(dp,dl,sp,sl)
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

	-		pointer to NUL at end of destination string


	See-also:

	strdcpy(3dam),
	strdwcpyopaque(3dam), 
	strdcpycompact(3dam), 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	isprintlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

static int	isour(int) ;


/* exported subroutines */


char *strdcpyclean(char *dbuf,int dlen,int sch,cchar *sp,int sl)
{
	int		ch ;
	int		dl = 0 ;
	while (dlen-- && sl-- && *sp) {
	    ch = MKCHAR(*sp) ;
	    if (isour(ch)) {
		dbuf[dl++] = (char) ch ;
	    } else if (sch != 0) {
		dbuf[dl++] = (char) sch ;
	    }
	    sp += 1 ;
	} /* end while */
	dbuf[dl] = '\0' ;
	return (dbuf+dl) ;
}
/* end subroutine (strdcpyclean) */


/* local subroutines */


static int isour(int ch)
{
	int		f = FALSE ;
	f = f || isprintlatin(ch) ;
	f = f || (ch == CH_NL) ;
	f = f || (ch == CH_BS) ;
	f = f || (ch == CH_BEL) ;
	return f ;
}
/* end subroutine (isour) */


