/* binexp */
/* lang=C99 */

/* binary-exponetial function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-10-09, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the binary exponetional of a given integral-valued number.

	Synopsis:

	double binexp(int n)

	Arguments:

	n	number to calculate the binary-exponetial for

	Returns:

	-	the result


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
	    } else {
		double t = (binexp(x,(n/2))*2.0) ;
		v = (n&1) ? v = t : (x*t) ;
	    }
	}
	return v ;
}
/* end subroutine (binexp) */


