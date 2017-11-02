/* minmaxelem (include-header) */
/* lang=C++98 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2018-09-15, David A­D­ Morano
        This was separated out from where it was to make a stand-alone
        (exported) subroutine.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We find both the minimum and the maximum element in a range of a
	container.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* name-spaces */


/* type-defs */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */

template<typename I,typename T>
pair<T,T> minmaxelem(const I &bit,const I &eit) 
{
	pair<T,T>	res ;
	I		it ;
	int		min = INT_MAX ;
	int		max = 0 ;

	for (it = bit ; it != eit ; it += 1) {
	    int	e = (*it) ;
	    if (e < min) min = e ;
	    if (e > max) max = e ;
	}
	res.first = min ;
	res.second = max ;
	return res ;
}
/* end subroutine (minmaxelem) */


