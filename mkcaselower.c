/* mkcaselower */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	This was written for Rightcore Network Services (RNS).

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We make a character (an 8-bit entity) out of an integer.  We do this
	quite simply.

	Synopsis:

	int mkcaselower(char *rp,int rl)

	Arguments:

	rp		source and destination buffer
	rl		length of source string

	Returns:

	<0		error
	>=0		length of result


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int mkcaselower(char *rp,int rl)
{
	int	i ;
	if (rl < 0) rl = INT_MAX ;
	for (i = 0 ; (i < rl) && *rp ; i += 1) {
	    if (CHAR_ISUC(rp[i])) rp[i] = CHAR_TOLC(rp[i]) ;
	}
	return i ;
}
/* end subroutine (mkcaselower) */


