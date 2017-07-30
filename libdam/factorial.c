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

	int factorial(unsigned int n)

	Arguments:

	n	number to return factorial value for

	Returns:

	-	the Fibonacci number of the input


	The original Fibonacci function:

	int factorial(int n)
	{
	    return (n * factorial(n-1)) ;
	}

        Note that when putting the result into a 32-bit unsigned integer (which
        is what we are doing here) the largest valued input (domain) of the
        Factorial function that can be represented in the result is 12. An input
        value of 13 overflows the 32-bit unsigned integer result. 


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


/* exported subroutines */


int factorial(unsigned int n)
{
	int		v = 1 ;
	if (n > 1) {
	    int	prev = factorial((n-1)) ;
	    if ((n <= 10) || ((prev % 10) == 0)) {
	        v = n * prev ;
		if ((n >= 10) && ((v % 10) != 0)) v = -1 ;
	    } else {
		v = -1 ;
	    }
	}
	return v ;
}
/* end subroutine (factorial) */


int factorialkill(volatile int *kfp,unsigned int n)
{
	int		v = -1 ;
	if ((kfp != NULL) && *kfp) {
	    v = 0 ;
	} else {
	    v = 1 ;
	    if (n > 1) {
		int	prev = factorialkill(kfp,(n-1)) ;
#if	CF_DEBUGS
		debugprintf("factorialkill: prev=%d rem=%d\n",
		prev,(prev%10)) ;
#endif
	        if ((n <= 10) || ((prev % 10) == 0)) {
	            v = n * prev ;
		    if ((n >= 10) && ((v % 10) != 0)) v = -1 ;
		} else {
		    v = -1 ;
		}
	    }
	}
	return v ;
}
/* end subroutine (factorialkill) */


