/* binexp */
/* lang=C99 */

/* binary-exponential function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-10-09, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the binary exponential of a given base (double float) and 
	an integral-valued exponent.

	Synopsis:

	double binexp(double x,int n)

	Arguments:

	x	base
	n	exponent

	Returns:

	-	the result


	This function just calculates:
		x ^^ n
	

*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<math.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local variables */


/* exported subroutines */


double binexp(double x,int n)
{
	double		v = -1 ;
	if (n >= 0) {
	    v = 1.0 ;
	    if (n == 1) {
		v = x ;
	    } else if (n == 2) {
		v = x*x ;
	    } else (n > 2) {
		double t = binexp(x,(n/2)) ;
		v = (n&1) ? x*t*t : t*t ;
	    }
	}
	return v ;
}
/* end subroutine (binexp) */

