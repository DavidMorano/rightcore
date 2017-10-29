/* snwcpyshrink */

/* copy the shrunken part of a source string */


#define	CF_ONEOVER	1		/* try one-pass over the string */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-23, David A­D­ Morano
	I added the experimental one-pass version.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a destination string from a shrunken source
	string.

	Synopsis:

	int snwcpyshrink(cchar *sp,int sl,cchar **rpp)

	Arguments:

	sp		source string
	sl		length of source string
	rpp		pointer to pointer to hold result

	Returns:

	-		length of found string

	Notes:

	Q. Should be bother with ignoring all data after an NL character?
	A. Good question.  I do not know the answer.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<ascii.h>
#include	<char.h>
#include	<strmgr.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	siskipwhite(cchar *,int) ;
extern int	snwcpy(char *,int,cchar *,int) ;

extern char	*strnchr(cchar *,int,int) ;


/* exported subroutines */


#if	CF_ONEOVER
int snwcpyshrink(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	cchar		*cp ;

	dbuf[0] = '\0' ;
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    strmgr	d ;
	    if ((rs = strmgr_start(&d,dbuf,dlen)) >= 0) {
		while ((rs >= 0) && cl--) {
		    if (*cp == CH_NL) break ;
		    rs = strmgr_char(&d,*cp) ;
		    cp += 1 ;
		} /* end while */
		rs1 = strmgr_finish(&d) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (strmgr) */
	} /* end if (sfshrink) */

	return rs ;
}
/* end subroutine (snwcpyshrink) */
#else /* CF_ONEOVER */
int snwcpyshrink(char *dbuf,int dlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*tp, *cp ;

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
#endif /* CF_ONEOVER */


