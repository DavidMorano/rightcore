/* fhm */

/* floating point harmonic mean */


/* revision history:

	= 2001-03-01, David A­D­ Morano

	I first created this for calculating HMs for Levo stuff.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine computes the harmonic mean on a set of numbers.

	Synopsis:

	double fhm(a,n)
	double	a[] ;
	int	n ;

	Arguments:

	a	array of double floats of the numbers to find the HM of
	n	number of elements in the array

	Returns:

		the harmonic mean


*******************************************************************************/


/* exported subroutines */


double fhm(double *a,int n)
{
	double		fn = (double) n ;
	double		d = 0.0 ;
	int		i ;

	if (n == 0)
	    return 0.0 ;

	for (i = 0 ; i < n ; i += 1) {

	    if (a[i] < 0.00001)
	        return 0.0 ;

	    d += (1.0 / a[i]) ;

	} /* end for */

	if (d < 0.00001)
	    return 0.0 ;

	return (fn / d) ;
}
/* end subroutine (fhm) */


