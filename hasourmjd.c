/* hasourmjd */

/* test whether a string is composed of our MJD specifiction */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Does the given string contain a Modified-Julian-Day (MJD) specification?

	Synopsis:

	int hasourmjd(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp		string to test
	sl		length of strin to test

	Returns:

	<0		error
	0		no MJD found (not ours anyway)
	>0		MJD


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	hasalldig(const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasourmjd(const char *sp,int sl)
{
	int		rs = SR_OK ;

	if (sl < 0) sl = strlen(sp) ;

	if ((sl > 1) && (CHAR_TOLC(sp[0]) == 'm')) {
	    sp += 1 ;
	    sl -= 1 ;
	    if (hasalldig(sp,sl)) {
		int	v ;
		if ((rs = cfdeci(sp,sl,&v)) >= 0) {
		    rs = v ;
		}
	    }
	} /* end if (has our 'm' marker) */

	return rs ;
}
/* end subroutine (hasourmjd) */


