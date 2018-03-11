/* fam */

/* floating point arithmetic mean */


/* revision history:

	= 2001-03-01, David A­D­ Morano
	I first created this for calculating stastics for Levo stuff.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine computes the arithmetic mean on a set of numbers.

	Synopsis:

	double fam(a,n)
	double	a[] ;
	int	n ;

	Arguments:

	a	array of double floats of the numbers to find the HM of
	n	number of elements in the array

	Returns:

		the harmonic mean


*******************************************************************************/


/* exported subroutines */


double fam(double *a,int n)
{
	double		ans = 0.0 ;
	if (n > 0) {
	    int		i ;
	    double	d = (double) n ;
	    double 	sum = 0.0 ;
	    for (i = 0 ; i < n ; i += 1) {
	        sum += a[i] ;
	    }
	    ans = (sum/d) ;
	}
	return ans ;
}
/* end subroutine (fam) */


