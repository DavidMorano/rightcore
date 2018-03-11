/* snwcpyopaque */

/* special (excellent) string-copy type of subroutine! */


/* revision history:

	= 1998-07-08, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is essentially the same as the 'snwcpy(3dam)' subroutine except
        that white-space characters not copied over to the result.

	Synopsis:
	int snwcpyopaque(dp,dl,sp,sl)
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
	snwcpyhyphen(3dam)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* exported subroutines */


int snwcpyopaque(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		ch ;
	int		dl = 0 ;
	int		rs = SR_OK ;
	while (sl && *sp) {
	    ch = MKCHAR(*sp) ;
	    if (! CHAR_ISWHITE(ch)) {
		if (dlen-- == 0) break ;
		dbuf[dl++] = (char) ch ;
	    } /* end if */
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	if ((sl != 0) && (*sp != '\0')) rs = SR_OVERFLOW ;
	dbuf[dl] = '\0' ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (snwcpyopaque) */


