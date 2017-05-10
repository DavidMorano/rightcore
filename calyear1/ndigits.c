/* ndigits */

/* get the number of digits in a number (integer) */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a support subroutine that is supposed to be cheaper than an
        integer-to-ascii conversion in order to get the number of digits in an
        integer. Note that this subroutine is essentially getting the log of the
        number but alleviates having to load-link the whole math library in
        order to just get the LOG subroutine from it. But it still uses a divide
        in its algorithm (at least only an integer divide). But a divide is,
        well, a divide. And all divides are expensive. And without full divide
        hardware (which used to exist in the old days), a divide can be very
        expensive!

	Synopsis:

	int ndigits(v,base)
	int	v ;
	int	base ;

	Arguments:

	v	value to find the number of ASCII digits for
	base	number-base to use

	Returns:

	-	number of base-<base> digits in number


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stdlib.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ndigits(int v,int base)
{
	int		n = 0 ;
	if (base >= 2) {
	    if (v < 0) {
	        n = 1 ;
	        v = abs(v) ;
	    } else if (v == 0) {
	        n = 1 ;
	    }
	    while (v != 0) {
	        n += 1 ;
	        v = (v / base) ;
	    }
	} /* end if (valid base) */
	return n ;
}
/* end subroutine (ndigits) */


