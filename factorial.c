/* factorial */

/* factorial function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the factorial of the given number.

	Synopsis:

	LONG factorial(int n)

	Arguments:

	n	number to return factorial value for

	Returns:

	-	the Fibonacci number of the input


	Notes:

	The original Fibonacci function:

	LONG factorial(int n)
	{
	    return (n * factorial(n-1)) ;
	}

        Note that when putting the result into a 32-bit unsigned integer (which
        is what we are doing here) the largest valued input (domain) of the
        Factorial function that can be represented in the result is 12. An input
        value of 13 overflows the 32-bit unsigned integer result. 


	Floating-point:

	f(n) = floor(1.0 / sqrt(5.0)) * pow(((1.0 + sqrt(5.0)) / 2.0),(n+1))
	

*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local variables */

static const LONG	facttab[] = {
	0x0000000000000001, 0x0000000000000001, 
	0x0000000000000002, 0x0000000000000006,
	0x0000000000000018, 0x0000000000000078, 
	0x00000000000002d0, 0x00000000000013b0,
	0x0000000000009d80, 0x0000000000058980, 
	0x0000000000375f00, 0x0000000002611500,
	0x000000001c8cfc00, 0x000000017328cc00, 
	0x000000144c3b2800, 0x0000013077775800,
	0x0000130777758000, 0x0001437eeecd8000, 
	0x0016beecca730000, 0x01b02b9306890000,
	0x21c3677c82b40000
} ;


/* exported subroutines */


LONG factorial(int n)
{
	LONG		v = -1 ;
	if (n >= 0) {
	    const int	ne = nelem(facttab) ;
	    if (n < ne) {
	        v = facttab[n] ;
	    }
	}
	return v ;
}
/* end subroutine (factorial) */


