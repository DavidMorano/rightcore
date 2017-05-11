/* maxvecaddr */

/* maximum core-address determination for a vector-array of strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<string.h>

const char *maxvecaddr(const char **va)
{
	const char	*maxp = NULL ;
	if (va != NULL) {
	    cchar	*lp ;
	    int		n ;
	    for (n = 0 ; va[n] != NULL ; n += 1) ;
	    lp = (const char *) (va+n+2) ;
	    if (lp > maxp) maxp = lp ;
	    {
		int	i ;
		cchar	*sp = va[0] ;
	        for (i = 1 ; i < n ; i += 1) {
		    if (va[n] > sp) sp = va[n] ;
	        } /* end for */
		lp = (strlen(sp) + 1) ;
		if (lp > maxp) maxp = lp ;
	    }
	} /* end if */
	return maxp ;
}
/* end subroutine (maxvecaddr) */


