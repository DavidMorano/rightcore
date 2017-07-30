/* mainheap */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2010-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2010 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
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
extern "C" int	mkrevstr(char *,int) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static int heapleft(int) ;
static int heapright(int) ;
static int printnodes(vector<int> &,int) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	vector<int>	v ;
	string		s = "hello world!" ;
	int		rs = SR_OK ;

	cout << s << endl ;
	for (auto ch : s) {
	   ch &= 0xff ;
	   v.push_back(ch) ;
	}

	{
	    for (auto val : v) {
		cout << " " << val ;
	    }
	    cout << endl ;
	}

	{
	    auto it = v.begin() ;
	    auto end = v.end() ;
	    make_heap(it,end) ;
	    cout << "heap raw\n" ;
	    for (auto val : v) {
		cout << " " << val ;
	    }
	    cout << endl ;
	}

	{
	    cout << "heap pre-order\n" ;
	    printnodes(v,0) ;
	    cout << endl ;
	}

	{
	    const int	sl = s.length() ;
	    cchar	*sp = s.c_str() ;
	    char	*bp ;
	    if ((bp = new(nothrow) char [sl+1]) != NULL) {
		const int	bl = sl ;
		strwcpy(bp,sp,sl) ;
		mkrevstr(bp,bl) ;
		cout << bp << endl ;
		delete [] bp ;
	    } else {
		rs = SR_NOMEM ;
	    } /* end if (m-a) */
	}


	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int heapleft(int n)
{
	return (2*n +1) ;
}

static int heapright(int n)
{
	return (2*n + 2) ;
}

static int printnodes(vector<int> &v,int n)
{
	if (n < v.size()) {
	    cout << " " << v[n] ;
	    printnodes(v,heapleft(n)) ;
	    printnodes(v,heapright(n)) ;
	} else {
	    cout << " ­" ;
	}
	return 0 ;
}

