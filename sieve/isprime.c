/* isprime */

/* determine if a number is prime or not */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little subroutine detemines if the given number is prime or not.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<math.h>

#include	<localmisc.h>


/* exported subroutines */


int isprime(uint n)
{
	double		fn, fr ;
	uint		i ;
	uint		root ;
	int		f = TRUE ;

	if (n == 2)
	    return TRUE ;

	if ((n == 1) || ((n & 1) == 0))
	    return FALSE ;

	fn = n ;
	fr = sqrt(fn) ;

	root = (uint) ceil(fr) ;	/* safety due to floating errors */

	for (i = 3 ; i <= root ; i += 1) {
	    f = ((n % i) != 0) ;
	    if (!f) break ;
	} /* end for */

	return f ;
}
/* end subroutine (isprime) */


