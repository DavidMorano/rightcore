/* mainlistadd */
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
#include	<algorithm>
#include	<functional>
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

typedef forward_list<int>	ourlist ;

class listadder {
	ourlist		&lr ;
	ourlist		&l1 ;
	ourlist		&l2 ;
	forward_list<int>::iterator	i1 ;
	forward_list<int>::iterator	i2 ;
	forward_list<int>::iterator	ir ;
	    forward_list<int>::iterator	e1 ;
	    forward_list<int>::iterator	e2 ;
	    forward_list<int>::iterator	er ;
public:
	listadder(ourlist &alr,ourlist &al1, ourlist &al2) 
	: lr(alr), l1(al1), l2(al2) {
	    i1 = l1.begin() ;
	    i2 = l2.begin() ;
	    ir = lr.begin() ;
	    e1 = l1.end() ;
	    e2 = l2.end() ;
	    er = lr.end() ;
	} ;
	int addone(int c) {
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: ent c=%d\n",c) ;
#endif
	    if (c >= 0) {
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: in c=%d\n",c) ;
#endif
	        if ((i1 != e1) && (i2 != e2)) {
	            int	vr = c ;
		    c = 0 ;
		    if (i1 != e1) {
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: l1=%d\n",*i1) ;
#endif
			vr += *i1++ ;
		    }
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: 1 vr=%d\n",vr) ;
#endif
		    if (i2 != e2) {
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: l2=%d\n",*i2) ;
#endif
			vr += *i2++ ;
		    }
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: 2 vr=%d\n",vr) ;
#endif
	            if (vr >= 10) {
		        vr %= 10 ;
		        c = 1 ;
		    }
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: carrying c=%d\n",c) ;
#endif
		    if (lr.empty()) {
	    	        lr.push_front(vr) ;
			ir = lr.begin() ;
		    } else {
	    	        lr.insert_after(ir++,vr) ;
		    }
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: inserting done\n") ;
#endif
		    c = addone(c) ;
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: recurse done\n") ;
#endif
	        } else {
		    c = -1 ;
	        }
	    }
#if	CF_DEBUGS
	debugprintf("main/listadder_addone: ret c=%d\n",c) ;
#endif
	    return c ;
	} ;
} ;


/* forward references */

int printlist(forward_list<int> &,cchar *) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	forward_list<int>	l1 = { 2, 4, 6, 1, 0 } ;
	forward_list<int>	l2 = { 1, 5, 3, 2, 9 } ;
	forward_list<int>	lr ;
	int			rs = SR_OK ;
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
	    {
	    	        lr.push_front(17) ;
#if	CF_DEBUGS
	debugprintf("main: insert-print\n") ;
#endif
	        printlist(lr,"lr-test") ;
	 	lr.clear() ;
	    }
#if	CF_DEBUGS
	debugprintf("main: addone()\n") ;
#endif
	    a.addone(0) ;
#if	CF_DEBUGS
	debugprintf("main: output()\n") ;
#endif
	    printlist(lr,"lr") ;
	} /* end block */

#if	CF_DEBUGS
	debugclose() ;
#endif
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


int printlist(forward_list<int> &l,cchar *s)
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


