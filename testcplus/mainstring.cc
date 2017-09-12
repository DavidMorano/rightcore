/* mainstring */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
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
extern "C" int	mkrevstr(char *,int) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	string		s = "hello world!" ;
	string		w ;
	int		i ;
	int		rs = SR_OK ;

/* one */

	for (i = 0 ; i < s.length() ; i += 1) {
	    w += s[i] ;
	}
	cout << s << endl ;
	cout << w << endl ;

/* two */

	{
	    cchar	*ns = "hello world more!" ;
	    for (i = 0 ; ns[i] ; i += 1) {
	  	int 	ch = ns[i] ;
	        s.replace(i,1,1,ch) ;
	    }
	    cout << s << endl ;
	}

/* three */

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


