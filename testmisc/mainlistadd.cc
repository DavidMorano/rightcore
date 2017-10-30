/* mainlistadd */
/* lang=C++11 */

/* test of list addition operations */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is really a sort of game.  We add numbers in two lists and put the
	results into a third list.


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
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"LISTADD_DEBUGFILE"


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

class listadder {
	vector<int>	&lr ;
	ourlist		&l1 ;
	ourlist		&l2 ;
	forward_list<int>::iterator	i1 ;
	forward_list<int>::iterator	i2 ;
	vector<int>::iterator	ir ;
	forward_list<int>::iterator	e1 ;
	forward_list<int>::iterator	e2 ;
	vector<int>::iterator	er ;
public:
	listadder(vector<int> &alr,ourlist &al1,ourlist &al2)
	    : lr(alr), l1(al1), l2(al2) {
	    i1 = l1.begin() ;
	    i2 = l2.begin() ;
	    ir = lr.begin() ;
	    e1 = l1.end() ;
	    e2 = l2.end() ;
	    er = lr.end() ;
	} ;
	int addto(int v) {
	    lr.push_back(v) ;
	    return 0 ;
	} ;
	int addone(int c) {
	    if (c >= 0) {
		if ((i1 != e1) && (i2 != e2)) {
	            int	vr = c ;
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: in c=%d\n",c) ;
#endif
	            c = 0 ;
	            if (i1 != e1) {
#if	CF_DEBUGS
	                debugprintf("main/listadder_addone: l1=%d\n",*i1) ;
#endif
	                vr += *i1 ; i1++ ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: 1 vr=%d\n",vr) ;
#endif
	            if (i2 != e2) {
#if	CF_DEBUGS
	                debugprintf("main/listadder_addone: l2=%d\n",*i2) ;
#endif
	                vr += *i2 ; i2++ ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: 2 vr=%d\n",vr) ;
#endif
	            if (vr >= 10) {
	                vr %= 10 ;
	                c = 1 ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: i vr=%d c=%d\n",vr,c) ;
#endif

		    addto(vr) ;
		    
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: inserting done\n") ;
#endif
	            c = addone(c) ;
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: recurse done\n") ;
#endif
		}
	    }
#if	CF_DEBUGS
	    debugprintf("main/listadder_addone: ret c=%d\n",c) ;
#endif
	    return c ;
	} ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	forward_list<int>	l1 = { 2, 4, 8, 1, 0 } ;
	forward_list<int>	l2 = { 1, 5, 3, 2, 9 } ;
	vector<int>		lr ;
	int			rs = SR_OK ;
	int			ex = 0 ;
	cchar			*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	{
	    listadder		a(lr,l1,l2) ;
#if	CF_DEBUGS
	    debugprintf("main: insert\n") ;
#endif
	   
	    printlist(l1,"l1>") ;
	    printlist(l2,"l2>") ;

#if	CF_DEBUGS
	    debugprintf("main: addone()\n") ;
#endif
	    a.addone(0) ;
#if	CF_DEBUGS
	    debugprintf("main: output()\n") ;
#endif
	    printvec(lr,"lr") ;
	} /* end block (central) */

#if	CF_DEBUGS
	debugprintf("main: ret\n") ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int printlist(forward_list<int> &l,cchar *s)
{
	int	c = 0 ;
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


