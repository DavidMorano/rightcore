/* minmax */

/* minimum and maximum */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines calculate the minimum or the maximum (respectively) of
        two (integer) values.

	Synopsis:

	int min(int a,int b)

	Arguments:

	a		value-1
	b		value-2

	Returns:

	-		the minimum of the two values


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int min(int a,int b)
{
	int	v = a ;
	if (b < v) v = b ;
	return v ;
}
/* end subroutine (min) */


int max(int a,int b)
{
	int	v = a ;
	if (b > v) v = b ;
	return v ;
}
/* end subroutine (max) */


long lmin(long a,long b)
{
	int	v = a ;
	if (b < v) v = b ;
	return v ;
}
/* end subroutine (lmin) */


long lmax(long a,long b)
{
	int	v = a ;
	if (b > v) v = b ;
	return v ;
}
/* end subroutine (lmax) */


LONG llmin(LONG a,LONG b)
{
	int	v = a ;
	if (b < v) v = b ;
	return v ;
}
/* end subroutine (llmin) */


LONG llmax(LONG a,LONG b)
{
	int	v = a ;
	if (b > v) v = b ;
	return v ;
}
/* end subroutine (llmax) */


