/* hasParam */

/* does the parameter array have a given value? */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

	= 2017-12-16, David A­D­ Morano
	Updated.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We search to see if our given value is in the given parameter array.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int hasParam(const short *pp,int pl,int v)
{
	int		i ;
	int		f = FALSE ;
#if	CF_DEBUGS
	{
	    debugprintf("hasParam: ent pl=%d v=%u\n",pl,v) ;
	    for (i = 0 ; (i < pl) && (pp[i] != SHORT_MIN) ; i += 1) {
	        debugprintf("hasParam: pv[%2u]=%d\n",i,pp[i]) ;
	    }
	}
#endif
	for (i = 0 ; (i < pl) && (pp[i] >= 0) ; i += 1) {
	    const int	pv = (int) pp[i] ;
	    if (pv == SHORT_MIN) break ;
	    f = (pv == v) ;
	    if (f) break ;
	}
#if	CF_DEBUGS
	debugprintf("hasParam: ret f=%u\n",f) ;
#endif
	return f ;
}
/* end subroutine (hasParam) */


