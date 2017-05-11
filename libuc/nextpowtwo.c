/* nextpowtwo */

/* calculate the next higher power of two for a given number */


/* revision history:

	= 1998-03-21, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine calculates the next higher power of two for the value
        given as the argument. This is very useful in many memory mappings and
        other storage allocation schemes where a power of two is preferred for
        management simplicity.

	Synopsis:

	uint nextpowtwo(n)
	uint	n ;

	Arguments:

	n		supplied integer (unsigned) value

	Returns:

	-		the next higher power of two for the given value;
			it will be the same if the given value is already
			at a power of two


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>		/* unsigned types */


/* external subroutines */

extern int	flbsi(uint) ;


/* exported subroutines */


uint nextpowtwo(uint v)
{
	int		lb ;
	int		nn = 0 ;

	if ((lb = flbsi(v)) >= 0) {
	    uint	mask = ((1 << lb) - 1) ;
	    if ((v & mask) && (lb < 31)) lb += 1 ;
	    nn = (1 << lb) ;
	}

	return nn ;
}
/* end subroutine (nextpowtwo) */


