/* cfdecf */

/* convert a decimal digit string to its binary floating value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was written adapted from assembly.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutines to convert decimal strings to binary floating values.

	Synopsis:

	int cfdecf(cchar *sp,int sl,double *rp)

	Arguments:

	sp		source string
	sl		source string length
	rp		pointer to hold result

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int cfdecf(cchar *sp,int sl,double *rp)
{
	int		rs = SR_OK ;
	int		bl ;
	const char	*bp ;
	char		tmpbuf[DIGBUFLEN + 1] ;
	char		*ep ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	bp = sp ;
	bl = sl ;
	if ((sl >= 0) && sp[sl]) {
	    bp = tmpbuf ;
	    rs = snwcpy(tmpbuf,DIGBUFLEN,sp,sl) ;
	    bl = rs ;
	}

	if ((rs >= 0) && bl) {
	    if ((! isdigit(*bp)) && (*bp != '.'))
	        rs = SR_INVALID ;
	}

	if (rs >= 0) {
	    rs = uc_strtod(bp,&ep,rp) ;
	}

	return (rs >= 0) ? (ep - bp) : rs ;
}
/* end subroutine (cfdecf) */


