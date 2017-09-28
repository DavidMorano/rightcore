/* mainrand */
/* lang=C++11 */

/* small test of the system random number generator */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform a small test of the system random number generator.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<climits>
#include	<cstdlib>
#include	<cinttypes>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	long		rn ;
	const int	n = 10 ;
	const int	m = 100 ;
	int		i ;
	int		v ;

	for (i = 0 ; i < n ; i += 1) {
	    rn = random() ;
	    v = (rn % m) ;
	    cout << v << endl ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


