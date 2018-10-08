/* dfactorial */
/* lang=C++98 */

/* floating-pint factorial function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-10-09, David A­D­ Morano
	This was originally written.

	= 2018-10-06, David A.D. Morano
	Refactor and modified for C++.

*/

/* Copyright © 2004,2018 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the (floating) factorial of the given number.

	Synopsis:

	double dfactorial(int n)

	Arguments:

	n	number to return factorial value for

	Returns:

	-	the factorial of the input


	Notes:

	The original factorial function:

	double dfactorial(double n)
	{
	    return (n * dfactorial(n-1)) ;
	}

	Floating-point:

	f(n) = floor(1.0 / sqrt(5.0)) * pow(((1.0 + sqrt(5.0)) / 2.0),(n+1))
	

*******************************************************************************/


#include	<envstandards.h>
#include	<climits>
#include	<cmath>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern "C" double	binexp(double,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif


/* local variables */


/* foward references */

extern "C" int	dfactorial(double,int) ;


/* exported subroutines */


double dfactorial(int n)
{	
	double		v = -1.0 ;
	if (n >= 0) {
	    v = 1.0 ;
	    if (n == 2) {
		v = 2.0 ;
	    } else if (n > 2) {
		constexpr double	m = floor(1.0 / sqrt(5.0)) ;
		constexpr double	b = ((1.0 + sqrt(5.0)) / 2.0) ;
		v = m * binexp(b,(n+1)) ;
	    }
	}
	return v ;
}
/* end subroutine (dfactorial) */

