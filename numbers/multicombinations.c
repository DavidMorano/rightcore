/* multicombinations */

/* n-choose-k function WITH repitition */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the multicombinations of the given number.

	Synopsis:

	int multicombinations(int n,int k)

	Arguments:

	n	number of items to choose from
	k	nuber of item to choose without repitition

	Returns:

	-	the Fibonacci number of the input

	Notes:

	Two immediate answers:

	a) combinations(n+k-1,k)
	b) combinations(n+k-1,n-1)

	also:

	c) factorial(n+l-1) / ( factorial(k) * factorial(n-1) )


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

extern int	factorial(int) ;
extern int	permutations(int,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int multicombinations(int n,int k)
{
	return combinations(n+k-1,k) ;
}
/* end subroutine (multicombinations) */


