/* main (testfindbit) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little program tests the 'findbit(3dam)' set of subroutines.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdio.h>

#include	<ascii.h>
#include	<findbit.h>
#include	<xfindbit.h>
#include	<random.h>

#include	<localmisc.h>


/* local defines */

#define	NLOOPS		(1*1024*1024)


/* forward references */

static int	testffbs(RANDOM *) ;
static int	testflbs(RANDOM *) ;
static int	testffbc(RANDOM *) ;
static int	testflbc(RANDOM *) ;


/* exported subroutines */


int main()
{
	RANDOM	r ;
	int	rs ;

	if ((rs = random_start(&r,1,0)) >= 0) {

	    testffbs(&r) ;
	    testflbs(&r) ;

	    testffbc(&r) ;
	    testflbc(&r) ;

	    random_finish(&r) ;
	} /* end if (random) */

	fprintf(stdout,"done\n") ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int testffbs(RANDOM *rp)
{
	uint	v ;

	int	i ;
	int	n1, n2 ;


	for (i = 0 ; i < NLOOPS ; i += 1) {

	    random_getuint(rp,&v) ;

	    n1 = ffbsi(v) ;

	    n2 = xffbsi(v) ;

	    if (n1 != n2)
		fprintf(stdout,"FFBS bad v=%08lx n1=%u n2=%u\n",v,n1,n2) ;

	} /* end for */

	return 0 ;
}
/* end subroutine (testffbs) */


static int testflbs(RANDOM *rp)
{
	uint	v ;

	int	i ;
	int	n1, n2 ;


	for (i = 0 ; i < NLOOPS ; i += 1) {

	    random_getuint(rp,&v) ;

	    n1 = flbsi(v) ;

	    n2 = xflbsi(v) ;

	    if (n1 != n2)
		fprintf(stdout,"FLBS bad v=%08lx n1=%u n2=%u\n",v,n1,n2) ;

	} /* end for */

	return 0 ;
}
/* end subroutine (testflbs) */


static int testffbc(RANDOM *rp)
{
	uint	v ;

	int	i ;
	int	n1, n2 ;


	for (i = 0 ; i < NLOOPS ; i += 1) {

	    random_getuint(rp,&v) ;

	    n1 = ffbci(v) ;

	    n2 = xffbci(v) ;

	    if (n1 != n2)
		fprintf(stdout,"FFBC bad v=%08lx n1=%u n2=%u\n",v,n1,n2) ;

	} /* end for */

	return 0 ;
}
/* end subroutine (testffbc) */


static int testflbc(RANDOM *rp)
{
	uint	v ;

	int	i ;
	int	n1, n2 ;


	for (i = 0 ; i < NLOOPS ; i += 1) {

	    random_getuint(rp,&v) ;

	    n1 = flbci(v) ;

	    n2 = xflbci(v) ;

	    if (n1 != n2)
		fprintf(stdout,"FLBC bad v=%08lx n1=%u n2=%u\n",v,n1,n2) ;

	} /* end for */

	return 0 ;
}
/* end subroutine (testflbc) */



