/* cfdouble */

/* convert a floating point digit string to its double value */


/* revision history:

	= 2007-01-88, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2007 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine also ignores white space at the front or back of the
	digit string and handles a minus sign.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	LOCBUFLEN	100


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int cfdouble(cchar *sbuf,int slen,double *rp)
{
	int		rs = SR_OK ;
	cchar		*sp ;
	char		locbuf[LOCBUFLEN + 1] ;

	if (sbuf == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	sp = sbuf ;
	if (slen >= 0) { /* because we need string to be NUL terminated */
	    sp = locbuf ;
	    rs = snwcpy(locbuf,LOCBUFLEN,sbuf,slen) ;
	    if (rs == SR_OVERFLOW) rs = SR_INVALID ;
	}

	if (rs >= 0) {
	    char	*bp ;
	    errno = 0 ;
	    *rp = strtod(sp,&bp) ;
	    if (errno != 0) rs = (- errno) ;
	}

	return rs ;
}
/* end subroutine (cfdouble) */


