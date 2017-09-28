/* mainheap */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We look at a heap and find the nth (by index) smallest element in a
	(modifiable) sequence of numbers.

	Algoirthms:

	A	max-heap smart
	B	max-heap dumb
	C	nth_element()


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
#include	<forward_list>
#include	<vector>
#include	<string>
#include	<queue>
#include	<deque>
#include	<list>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* typedefs */

typedef vector<int>::iterator	vit ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	mkrevstr(char *,int) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */

static int ntha(const vector<int> &,const int n) ;
static int nthb(const vector<int> &,const int n) ;

static int heapleft(int) ;
static int heapright(int) ;
static int printnodes(vector<int> &,int) ;

static int printv(vector<int> &,int) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	vector<int>	v ;
	string		s = "hello world!" ;
	const int	n = 4 ; /* nth (smallest) element */
	int		rs = SR_OK ;
	int		ex = 0 ;

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
	    cout << "heap pre-order\n" ;
	    printnodes(v,0) ;
	    cout << endl ;
	}

/* nth smallest */

	{
	    vector<int>		v ;
	    int			nth ;
	    cout << "n=" << n << endl ;
	    for (auto ch : s) {
	        ch &= 0xff ;
	        v.push_back(ch) ;
	    }
	    nth = ntha(v,n) ;
	    cout << "A nth=" << nth << endl ;
	    nth = nthb(v,n) ;
	    cout << "B nth=" << nth << endl ;
	    {
	        typedef less<int>	icmp_t ;
	        const int		vl = v.size() ;
	        if (n < vl) {
	            icmp_t	icmp ;
	            vit		end = v.end() ;
	            vit		it = v.begin() ;
	            /* nth_element<vit>(it,it+n,end) ; */
		    nth_element<vit,icmp_t>(it,it+n,end,icmp) ;
	            nth = v[n] ;
	            cout << "C nth=" << nth << endl ;
	            printv(v,vl) ;
	        }
	    }
	}

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


/* smarter */
static int ntha(const vector<int> &s,const int n)
{
	priority_queue<int>		maxheap ;
	const int	sl = s.size() ;
	int		ch ;
	int		maxch ;
	int		nth = 0 ;
	int		i ;
	for (i = 0 ; i < sl ; i += 1) {
	    ch = s[i] ;
	    if (i > n) {
	        maxch = maxheap.top() ;
	        if (ch <= maxch) {
	            maxheap.push(ch) ;
	            maxheap.pop() ;
	        }
	    } else {
	        maxheap.push(ch) ;
	    }
	} /* end for */
	if (i > 0) nth = maxheap.top() ;
	return nth ;
}
/* end subroutine (ntha) */


/* dumber */
static int nthb(const vector<int> &s,const int n)
{
	priority_queue<int>		maxheap ;
	const int	sl = s.size() ;
	int		ch ;
	int		nth = 0 ;
	int		i ;
	for (i = 0 ; i < sl ; i += 1) {
	    ch = s[i] ;
	    maxheap.push(ch) ;
	    if (i > n) {
	        maxheap.pop() ;
	    }
	} /* end for */
	if (i > 0) nth = maxheap.top() ;
	return nth ;
}
/* end subroutine (nthb) */


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
	const int	vl = (int) v.size() ;
	if (n < vl) {
	    cout << " " << v[n] ;
	    printnodes(v,heapleft(n)) ;
	    printnodes(v,heapright(n)) ;
	} else {
	    cout << " ­" ;
	}
	return 0 ;
}
/* end subroutine (printnodes) */


static int printv(vector<int> &v,int n)
{
	int		i = 0 ;
	for (auto e : v) {
	    if (i++ < n) {
	        cout << " " << e ;
	    }
	}
	cout << endl ;
	return 0 ;
}
/* end subroutine (printv) */


