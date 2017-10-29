/* fibonacci */

/* Fibonacci function */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine implements the Fibonacci function.  We calculate
	the result using recursion (so watch out for large inputs -> stack 
	overflow!).

	Synopsis:

	LONG fibonacci(unsigned int n)

	Arguments:

	n	number to return Fibonacci value for

	Returns:

	-	the Fibonacci number of the input


	The original Fibonacci function:

	LONG fibonacci(int n)
	{
	    int	v = 0 ;
	    if ((n == 1) || (n == 2)) {
	        n = 1 ;
	    } else (n > 2) {
	        v = fibonacci(n - 1) + fibonacci(n - 2) ;
	    }
	    return v ;
	}

        Note that when putting the result into a 32-bit unsigned integer (which
        is what we are doing here) the largest valued input (domain) of the
        Fibonacci function that can be represented in the result is 47. An input
        value of 48 overflows the 32-bit unsigned integer result. For this
        reason, the table of values below is only 48 entries, with the last
        entry (#47) storing the last representable result for a 32-bit unsigned
        integer.


*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<localmisc.h>


/* local variables */

static const unsigned int fibotab[] = {
	0x00000000, 0x00000001, 0x00000001, 0x00000002,
	0x00000003, 0x00000005, 0x00000008, 0x0000000d,
	0x00000015, 0x00000022, 0x00000037, 0x00000059,
	0x00000090, 0x000000e9, 0x00000179, 0x00000262,
	0x000003db, 0x0000063d, 0x00000a18, 0x00001055,
	0x00001a6d, 0x00002ac2, 0x0000452f, 0x00006ff1,
	0x0000b520, 0x00012511, 0x0001da31, 0x0002ff42,
	0x0004d973, 0x0007d8b5, 0x000cb228, 0x00148add,
	0x00213d05, 0x0035c7e2, 0x005704e7, 0x008cccc9,
	0x00e3d1b0, 0x01709e79, 0x02547029, 0x03c50ea2,
	0x06197ecb, 0x09de8d6d, 0x0ff80c38, 0x19d699a5,
	0x29cea5dd, 0x43a53f82, 0x6d73e55f, 0xb11924e1
} ;


/* exported subroutines */


LONG fibonacci(int n)
{
	const int	ntab = nelem(fibotab) ;
	LONG		v = -1 ;
	if (n < ntab) {
	    v = fibotab[n] & UINT_MAX ;
	} else {
	    v = fibonacci(n-1) * fibonacci(n-2) ;
	}
	return v ;
}
/* end subroutine (fibonacci) */


