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
#include	<stdio.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
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
	    priority_queue<int,vector<int>,greater<int>>	mh ;
	    const int	a[] = { 10, 3, 4, 5, 9, 11 } ;
	    const int	n = 3 ;
	    int		c = 0 ;
	    for (auto v : a ) {
		mh.push(v) ;
		if (c++ >= n) {
		    mh.pop() ;
		    c -= 1 ;
		}
	    }
	    cout << "largest numbers\n" ;
	    while (! mh.empty()) {
		int v = mh.top() ;
	        cout << " " << v ;
		mh.pop() ;
	    }
	    cout << endl ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


