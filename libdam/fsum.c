/* fsum */

/* floating point sum */


/* revision history:

	= 2001-03-01, David A­D­ Morano
	I first created this for calculating stastics for Levo stuff.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine computes the sum on a set of numbers.

	Synopsis:

	double fsum(a,n)
	double	a[] ;
	int	n ;

	Arguments:

	a	array of double floats of the numbers to find the HM of
	n	number of elements in the array

	Returns:

		sum


*******************************************************************************/


/* exported subroutines */


double fsum(double *a,int n)
{
	double		fsum = 0.0 ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    fsum += a[i] ;
	}
	return fsum ;
}
/* end subroutine (fsum) */


