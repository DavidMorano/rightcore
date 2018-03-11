/* igcd */

/* integer Greastest Common Divisor (GCD) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine calculates the Greastest Common Divisor (GCD) of
        two integer numbers.

	Synopsis:

	int igcd(int a,int b)

	Arguments:

	a	number
	b	number

	Returns:

	-	GCD(a,b)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* more types */


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int igcd(int a,int b)
{
	int		r ;
	while ((r = (a % b)) > 0) {
	    a = b ;
	    b = r ;
	} /* end while */
	return b ;
}
/* end subroutine (igcd) */


