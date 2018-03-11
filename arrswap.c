/* arrswap1 */
/* lang=C99 */

/* array swap */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

	= 2017-09-15, David A­D­ Morano
	changed name of this subroutine (never really used much).

*/

/* Copyright © 2001,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We swap two elements in an array of integers.

	Synopsis:

	void arrswapi(int *a,int i1,int i2)

	Arguments:

	a	array
	i1	element to swap
	i2	element to swap

	Returns:

	-


*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* forward references */


/* local variables */


void arrswapi(int *a,int i1,int i2)
{
	int	t = a[i1] ;
	a[i1] = a[i2] ;
	a[i2] = t ;
}
/* end subroutine (arrswap) */


