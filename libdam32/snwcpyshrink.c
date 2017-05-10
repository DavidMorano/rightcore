/* snwcpyshrink */

/* copy the shrunken part of a source string */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a destination string from a shrunken source
	string.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ascii.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strnchr(const char *,int,int) ;


/* exported subroutines */


int snwcpyshrink(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*tp, *cp ;

	dbuf[0] = '\0' ;
	if ((tp = strnchr(sp,sl,CH_NL)) != NULL) {
	    sl = (tp-sp) ;
	}
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    rs = snwcpy(dbuf,dlen,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (snwcpyshrink) */


