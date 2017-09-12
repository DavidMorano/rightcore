/* mainprique */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

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
#include	<vector>
#include	<queue>
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
extern "C" int	mkrevstr(char *,int) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static int	printa(const int *,int) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	priority_queue<int,vector<int>,greater<int>>	pq ;
	string		s = "hello world!" ;
	int		rs = SR_OK ;

	cout << "orig order\n" ;
	for (auto sch : s) {
	   int	ch = MKCHAR(sch) ;
	   cout << " " << ch ;
	   pq.push(ch) ;
	}
	cout << endl ;

	if (rs >= 0) {
	    cout << "heap pre-order\n" ;
	    while (! pq.empty()) {
		const int ch = pq.top() ;
	        cout << " " << ch ;
		pq.pop() ;
	    }
	    cout << endl ;
	}

	if (rs < 0) cout << "error\n" ;

/* largest three numbers */

	if (rs >= 0) {
	    priority_queue<int,vector<int>,greater<int>>	minheap ;
	    const int	a[] = { 10, 3, 4, 5, 9, 11, 11, 21, 31 } ;
	    const int	n = 3 ; /* how many largest numbers */
	    int		ne ;
	    int		c = 0 ;
	    ne = nelem(a) ;
	    for (auto v : a ) {
		minheap.push(v) ;
		if (c++ >= n) {
		    minheap.pop() ;
		}
	    }
	    cout << "source list\n" ;
	    printa(a,ne) ;
	    cout << "largest " << n << " numbers\n" ;
	    while (! minheap.empty()) {
		int v = minheap.top() ;
	        cout << " " << v ;
		minheap.pop() ;
	    }
	    cout << endl ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int printa(const int *a,int n)
{
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    cout << " " << a[i] ;
	}
	cout << endl ;
	return i ;
}
/* end subroutine (printa) */


