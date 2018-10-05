/* ipowell */

/* return LONGLONG result from integer-power */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DYNAMIC	1		/* dynamic programming */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

	= 2018-10-05, David A.D. Morano
	This is the second time I had to create this subroutine |ipwell(3dam)|
	now. Somehow the first creation of this got lost! Happily the
	conversion from the old |ipow(3dam)| to |ipowell(3dam)| is not too
	complicated.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine calculates and returns a LONGLONG result from taking
	an integer power for a given base.

	Synopsis:

	longlong_t ipow(int b,int p)

	Arguments:

	b	base
	p	power

	Returns:

	-	result (b**p)

	Notes: 

	+ This function is only defined for positive exponents.
	+ We provide an optional dynamic-programming version, which uses
	recursion. Remember that recursion can be bad for stack depth, which we
	are now always on the lookout for seeing as we are now always on
	threads. The dynamic-programming version is enabled by a define at the
	top.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* 'LONG' type */


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	CF_DYNAMIC

longlong_t ipowell(int b,int n)
{
	longlong_t	r = 1 ;
	if (n == 1) {
	    r = b ;
	} else if (n == 2) { /* common case */
	    r = b*b ;
	} else if (n > 2) {
	    longlong_t	t = ipowell(b,(n/2)) ;
	    if ((n&1) == 0) {
		r = (t*t) ;
	    } else {
		r = b*(t*t) ;
	    }
	}
	return r ;
}
/* end subroutine (ipowell) */

#else /* CF_DYNAMIC */

longlong_t ipowell(int b,int n)
{
	longlong_t	r = 1 ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    r *= b ;
	}
	return r ;
}
/* end subroutine (ipowell) */

#endif /* CF_DYNAMIC */

