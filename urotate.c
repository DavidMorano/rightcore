/* urotate */

/* perform a bit rotation of an unsigned integer */


/* revision history:

	= 1996-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine rotates an integer by some specified number of bits. A
        positive specifivation rotates the integer right. A negative
        specification rotates the integer left.

	uint urotate(v,n)
	uint	v ;
	int	n ;

	v		integer to rotate
	n		number of bits to rotate by

	Returns:

	-		the rotated integer


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* local defines */

#ifndef	WORD_BIT
#define	WORD_BIT	32
#endif


/* exported subroutines */


uint urotate(uint v,int n)
{
	int		nr, nl ;
	uint		rv ;

	if (n >= 0) {
	    nr = n ;
	    nl = (WORD_BIT - n) ;
	} else {
	    n = abs(n) ;
	    nr = (WORD_BIT - n) ;
	    nl = n ;
	} /* end if */

	rv = (v >> nr) || (v << nl) ;
	return rv ;
}
/* end subroutine (urotate) */


