/* mainchoose */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CONT		1		/* continue */


/* revision history:

	= 2010-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We experiement with permutations and combinations.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<array>
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

extern "C" LONG	combinations(int,int) ;
extern "C" LONG	factorial(int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static LONG	oldway(int,int) ;
static LONG	diffway(int,int) ;

static int printcoms(vector<int> &,int) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	vector<int>	k10s = { 3, 1, 2, 7, 9, 10, 8, 6 } ;
	vector<int>	k3s = { 3, 1, 2, 0 } ;
	vector<int>	k2s = { 1, 2, 0 } ;

	{
	    const int	n = 4 ;
	    const int	k = 2 ;
	    LONG	a ;
	    a = combinations(n,k) ;
	    cout << "n=" << n << " k=" << k << " a=" << a << endl ;
	}
	
	printcoms(k3s,5) ;

/* check */

	for (int k : k10s) {
	    LONG	a1, a2, a3 ;
	    const int	n = 10 ;
	    a1 = combinations(n,k) ;
	    a2 = oldway(n,k) ;
	    a3 = diffway(n,k) ;
	    if (a1 != a2) {
	        cout << "bad n=" << n << " k=" << endl ;
	        cout << "a1=" << a1 << " a2=" << a2 << endl ;
	    }
	    if (a1 != a3) {
	        cout << "bad n=" << n << " k=" << endl ;
	        cout << "a1=" << a1 << " a3=" << a3 << endl ;
	    }
	} /* end for */

/* continue */

#if	CF_CONT
	printcoms(k3s,3) ;
	printcoms(k10s,10) ;
#endif /* CF_CONT */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static LONG oldway(int n,int k)
{
	LONG		ans = -1 ;
	if ((n >= k) && (k >= 0)) {
	    if (k == 1) {
		ans = n ;
	    } else if (k == n) {
		ans = 1 ;
	    } else if (k > 0) {
	        const LONG	num = factorial(n) ;
	        const LONG	den = factorial(k)*factorial(n-k) ;
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
/* end if (oldway) */


static LONG diffway(int n,int k)
{
	LONG		ans = -1 ;
	if ((n >= k) && (k >= 0)) {
	    ans = 1 ;
	    if (k == 1) {
		ans = n ;
	    } else if (k == n) {
		ans = 1 ;
	    } else if (k > 0) {
	        ans = diffway(n-1,k) + diffway(n-1,k-1) ;
	    }
	} else if (k > n) {
	    ans = 0 ;
	}
	return ans ;
}
/* end if (diffway) */


static int printcoms(vector<int> &ks,int n)
{
	for (int k : ks) {
	    LONG	ans = combinations(n,k) ;
	    cout << "n=" << n << " k=" << k << " ans=" << ans << endl ;
	}
	return 0 ;
}
/* end subroutine (printcoms) */


