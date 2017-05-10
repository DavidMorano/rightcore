/* hexval */

/* get the value of a single hexadecimal digit */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was adapted from assembly.  The original assembly goes
	wa...ay back.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We examine a single character, supposedly a hexadecimal digit, and
	return either an error or the value of the symbolic hexadecimal digit.

	Synopsis:

	int hexval(int ch)

	Arguments:

	ch		character to evaluate

	Outputs:

	<0		error
	>=0		value of symbolic hexadecimal digit


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* exported subroutines */


int hexval(int ch)
{
	int		v = SR_DOM ;
	if ((ch >= '0') && (ch <= '9')) {
	    v = (ch-'0') ;
	} else {
	    const int	lch = CHAR_TOLC(ch) ;
	    if ((lch >= 'a') && (lch <= 'f')) {
	        v = ((lch-'a')+10) ;
	    }
	}
	return v ;
}
/* end subroutine (hexval) */


