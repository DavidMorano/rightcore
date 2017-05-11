/* denpercents */

/* calculate certain percentages of accumulation from a density */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-02-16, David A­D­ Morano

	This was originally written for performing some statistical
	calculations for simulation work.


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Floating Point Percentage Accumulation Calculator (for Arrays
	of integers)

	This subroutine will calculate the mean and variance for an array
	of 32-bit integer values.  The results are floating point values.

	Synopsis:

	int denpercents[il](a,n,percents)
	uint	a[] ;
	int	n ;
	double	percents[] ;

	Arguments:

	a		array of 'uint' (32-bit) integers
	n		number of elements in array
	percents	pointer to array of 100 double float to get results

	Returns:

	>=0	OK
	<0	something bad


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int denpercentsi(a,n,percents)
uint	a[] ;
int	n ;
double	percents[] ;
{
	double	sum, pb ;
	double	thresh[100] ;

	ULONG	count ;

	int	i, j ;
	int	f_done ;


	if (a == NULL)
	    return SR_FAULT ;

	if (percents == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("denpercentsi: n=%d\n",n) ;
#endif

	for (j = 1 ; j < 101 ; j += 1)
	    percents[j % 100] = 0.0 ;

	if (n <= 0)
	    return SR_NOENT ;

	for (j = 1 ; j < 101 ; j += 1)
	    percents[j % 100] = (double) n ;

	count = 0 ;
	for (i = 0 ; i < n ; i += 1)
	    count += a[i] ;

	sum = (double) count ;

#if	CF_DEBUGS
	debugprintf("denpercentsi: count=%llu sum=%12.4f\n",
	    count,sum) ;
#endif

	for (j = 1 ; j < 101 ; j += 1) {

	    thresh[j % 100] = sum * 0.01 * ((double) j) ;

#if	CF_DEBUGS && 0
	    debugprintf("denpercentsi: thresh[%u]=%12.4f\n",
	        (j % 100),thresh[j % 100]) ;
#endif

	}

	sum = 0.0 ;
	f_done = FALSE ;
	j = 1 ;
	for (i = 0 ; (! f_done) && (i < n) ; i += 1) {

	    pb = (double) a[i] ;
	    sum += pb ;
	    while ((! f_done) && (sum >= thresh[j % 100])) {

	        percents[j % 100] = ((double) i) ;

#if	CF_DEBUGS
	        debugprintf("denpercentsi: cov=%2u i=%9u p=%12.4f\n",
	            j,i,percents[j % 100]) ;
#endif

	        j += 1 ;
	        f_done = (j >= 101) ;

	    } /* end if (reached a threshold) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("denpercentsi: 70=%12.4f 80=%12.4f 90=%12.4f\n",
	    percents[70],percents[80],percents[90]) ;
#endif

	return 0 ;
}
/* end subroutine (denpercentsi) */


int denpercentsl(a,n,percents)
ULONG	a[] ;
int	n ;
double	percents[] ;
{
	double	sum, pb ;
	double	thresh[100] ;

	ULONG	count ;

	int	i, j ;
	int	f_done ;


	if (a == NULL)
	    return SR_FAULT ;

	if (percents == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("denpercentsl: n=%d\n",n) ;
#endif

	for (j = 1 ; j < 101 ; j += 1)
	    percents[j % 100] = 0.0 ;

	if (n <= 0)
	    return SR_NOENT ;

	for (j = 1 ; j < 101 ; j += 1)
	    percents[j % 100] = (double) n ;

	count = 0 ;
	for (i = 0 ; i < n ; i += 1)
	    count += a[i] ;

	sum = (double) count ;

#if	CF_DEBUGS
	debugprintf("denpercentsl: count=%llu sum=%12.4f\n",
	    count,sum) ;
#endif

	for (j = 1 ; j < 101 ; j += 1) {

	    thresh[j % 100] = sum * 0.01 * ((double) j) ;

#if	CF_DEBUGS && 0
	    debugprintf("denpercentsl: thresh[%u]=%12.4f\n",
	        (j % 100),thresh[j % 100]) ;
#endif

	}

	sum = 0.0 ;
	f_done = FALSE ;
	j = 1 ;
	for (i = 0 ; (! f_done) && (i < n) ; i += 1) {

	    pb = (double) a[i] ;
	    sum += pb ;
	    while ((! f_done) && (sum >= thresh[j % 100])) {

	        percents[j % 100] = ((double) i) ;

#if	CF_DEBUGS
	        debugprintf("denpercentsl: cov=%2u i=%9u p=%12.4f\n",
	            j,i,percents[j % 100]) ;
#endif

	        j += 1 ;
	        f_done = (j >= 101) ;

	    } /* end if (reached a threshold) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("denpercentsl: 70=%12.4f 80=%12.4f 90=%12.4f\n",
	    percents[70],percents[80],percents[90]) ;
#endif

	return 0 ;
}
/* end subroutine (denpercentsl) */



