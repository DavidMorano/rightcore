/* mailtrailing */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Fun and games.  We find the nunber of trailing zero digits (in decimal)
	for a given factorial value (itself computed from a given source).

	Keywords:

	fives, zeros, twos, tailing, digits

	Synopsis:

	$ trailing


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
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

extern "C" uint	factorial(uint) ;

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static int trailing(longlong_t) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	const int	nmax = 30 ;
	string		w ;
	int		n ;
	int		z ;

	for (n = 0 ; n < nmax ; n += 1) {
	    z = trailing(n) ;
	    cout << "n=" << n << " z=" << z << endl ;
	}
	
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int trailing(longlong_t n)
{
	longlong_t	v = n ;
	int		c = 0 ;
	
	while (v >= 5) {
	    longlong_t	t = (v/5) ;
	    c += t ;
	    v = t ;
	}

	return c ;
}
/* end subroutine (trailing) */


