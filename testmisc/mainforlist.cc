/* mainforlist */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */


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
#include	<forward_list>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"FORLIST_DEBUGFILE"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	mkrevstr(char *,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */

static int printlist(forward_list<int> &,cchar *) ;
static int printvec(vector<int> &,cchar *) ;

typedef forward_list<int>	ourlist ;


/* forward references */

static int forlist_add(ourlist &,ourlist::iterator &,int) ;


/* local variables */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	int			rs = SR_OK ;
	cchar			*cp ;
#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */
	{
	    forward_list<int>	srclist = { 2, 4, 8, 1, 0 	} ;
	    forward_list<int>	lr ;
	    ourlist::iterator	it ;
	    printlist(srclist,"src") ;
	    it = lr.begin() ;
	    for (auto v : srclist) {
		rs = forlist_add(lr,it,v) ;
	    }
	    printlist(lr,"res") ;
	} /* end block */

#if	CF_DEBUGS
	debugprintf("main: ret\n") ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


/* extreme danger: only increment iterator after the insert call */
static int forlist_add(ourlist &ll,ourlist::iterator &it,int v) { 
	int		rs = SR_OK ;
	if (ll.empty()) {
	    ll.push_front(v) ;
	    it = ll.begin() ;
	} else {
	    ll.insert_after(it,v) ; /* danger: only increment iterator after */
	    it++ ;
	}
	return rs ;
}
/* end subroutine (forlist_add) */


static int printlist(forward_list<int> &l,cchar *s)
{
	int	c = 0 ;
	cout << s ;
	for (auto v : l) {
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
}
/* end subroutine (printlist) */


static int printvec(vector<int> &l,cchar *s)
{
	int	c = 0 ;
	for (auto v : l) {
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
}
/* end subroutine (printvec) */


