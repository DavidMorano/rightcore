/* densitystati */

/* calculate the mean and variance on some integral numbers */


/* revision history:

	= 2002-02-16, David A­D­ Morano
        This was originally written for performing some statistical calculations
        for simulation related work.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Floating Point Mean-Variance Calculator

        This subroutine will calculate the mean and variance for an array of
        32-bit integer values. The results are floating point values.

	Synopsis:

	int densitystati(a,n,mp,vp)
	uint		a[] ;
	int		n ;
	double		*mp, *vp ;

	Arguments:

	a	array of LONG (64-bit) integers
	n	number of elements in array
	mp	pointer to double to hold the 'mean' result
	vp	pointer to double to hold the 'variance' result

	Returns:

	>=0	OK
	<0	something bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int densitystati(uint a[],int n,double *mp,double *vp)
{

	if (a == NULL) return SR_FAULT ;

	if (mp != NULL) *mp = 0.0 ;

	if (vp != NULL) *vp = 0.0 ;

	if (n > 0) {
	    double	s1 = 0.0 ;
	    double	s2 = 0.0 ;
	    double	m1, m2 ;
	    double	x, p, fnp ;
	    ULONG	np = 0 ;
	    int		i ;

	    for (i = 0 ; i < n ; i += 1) {
	        x = (double) i ;
	        p = (double) a[i] ;
	        np += a[i] ;
	        s1 += (p * x) ;
	        s2 += (p * x * x) ;
	    } /* end for */

	    if (np > 0) {
	        fnp = (double) np ;
	        m1 = s1 / fnp ;
	        if (mp != NULL) *mp = m1 ;
	        if (vp != NULL) {
	            m2 = s2 / fnp ;
	            *vp = m2 - (m1 * m1) ;
	        }
	    } /* end if */

	} /* end if (positive) */

	return SR_OK ;
}
/* end subroutine (densitystati) */


