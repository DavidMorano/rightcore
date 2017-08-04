/* minvec */
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
#include	<vecint.h>
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

static const int	vals[] = { 3, 7, 19, 12, 43 } ;


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	vecint		vi ;
	int		rs ;

	if ((rs = vecint_start(&vi,5,0)) >= 0) {
	    const int	n = nelem(vals) ;
	    if ((rs = vecint_addlist(&vi,vals,n)) >= 0) {
		if ((rs = vecint_assign(&vi,8,71)) >= 0) {
	            int		i ;
	            int		v ;
	            for (i = 0 ; vecint_getval(&vi,i,&v) >= 0 ; i += 1) {
		        cout << " " << v ;
	            }
	            cout << endl ;
	        }
	    }
	    vecint_finish(&vi) ;
	} /* end if (vecint) */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


