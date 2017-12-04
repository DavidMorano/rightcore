/* strnnlen */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine computes the length of a supplied string but will not
	examine the string for more than a specified length.

	Synopsis:

	int strnnlen(sp,sl,max)
	const char	*sp ;
	int		sl ;
	int		max ;

	Arguments:

	sp	string to be examined
	sl	length of string (or -1)
	nax	maximum length of string to be examined

	Returns:

	len	mimimum of length of string or MIN(slen,max)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int strnnlen(cchar *sp,int sl,int mlen)
{
	int		len = 0 ;

	if ((sp != NULL) && (sl != 0) && (mlen != 0)) {
	    if (sl >= 0) {
	        if (mlen >= 0) sl = MIN(sl,mlen) ;
	    } else {
	        sl = mlen ;
	    }
	    len = strnlen(sp,sl) ;
	} /* end if */

	return len ;
}
/* end subroutine (strnnlen) */


