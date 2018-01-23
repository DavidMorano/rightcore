/* combinations */

/* n-choose-k function WITHOUT repitition */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-03-01, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the combinations n-C-k *without* repititions.

	Synopsis:
	LONG combinations(int n,int k)

	Arguments:
	n	number of items to choose from
	k	nuber of item to choose without repitition

	Returns:
	-	the Fibonacci number of the input

	= Notes:

	· Combinations *without* repetition (this subroutine):

	special cases
	
	+ k == 0 -> ans = 1
	+ k == 1 -> ans = n 
	+ k == n -> ans = 1
	+ k > n  -> ans = 0		(sort of a special case)

	general (classic n-choose-k)

		{     n!      }
		{------------ }
		{ k! · (n-k)! }

	· Note for combinations *with* repitition:

	For combinations *with* repitition the result is:

		 (n-k-1)!	  { n-k-1 }   { n-k-1 }
	        -----------	= {       } = {       }
		k! · (n-1)!	  {   k   }   {  n-1  }

	Combination *with* repitition are also called "multicombinations."
	A subroutine for multicombinations is provied (below) and is:

	Synopsis:
	LONG multicombinations(int n,int k)


*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external subroutines */

extern LONG	factorial(int) ;
extern LONG	permutations(int,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


LONG combinations(int n,int k)
{
	LONG		ans = -1 ;
	if ((n >= 0) && (k >= 0)) {
	    ans = 1 ;
	    if (k == 1) {
		ans = n ;
	    } else if (k == n) {
		ans = 1 ;
	    } else if (k > 0) {
	        const LONG	num = permutations(n,k) ;
	        const LONG	den = factorial(k) ;
		if ((num >= 0) && (den > 0)) {
	            ans = num / den ;
		} else {
		    ans = -1 ;
		}
	    }
	} else if (k > n) {
	    ans = 0 ;
	}
	return ans ;
}
/* end subroutine (combinations) */


LONG multicombinations(int n,int k)
{
	return combinations(n+k-1,k) ;
}
/* end subroutine (multicombinations) */


