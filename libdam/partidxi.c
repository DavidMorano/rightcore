/* partidxi */
/* lang=C99 */

/* integer array partitioning function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Partition an array of integers.

	Synopsis:

	int partidxi(int *a,int first,int last,int idx)

	Arguments:

	a		array
	fist		starting index
	last		over-last index
	idx		index to use as pivot

	Returns:

	pi		resulting pivot index


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* typedefs */


/* external subroutines */

extern void	arrswapi(int *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,cchar *,int) ;
#endif


/* forward references */


/* local variables */


int partidxi(int *a,int first,int last,int idx)
{
	const int	pv = a[idx] ;
	int		pi = first ;
	int		i ;
	if (idx != (last-1)) arrswapi(a,idx,(last-1)) ;
	for (i = first ; i < (last-1) ; i += 1) {
	    if (a[i] < pv) {
		if (i != pi) arrswapi(a,i,pi) ;
		pi += 1 ;
	    }
	} /* end for */
	if (pi != (last-1)) arrswapi(a,pi,(last-1)) ;
	return pi ;
}
/* end subroutine (partidxi) */


