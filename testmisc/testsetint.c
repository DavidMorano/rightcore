/* testsetint */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<stdio.h>
#include	<cinttypes>
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

#include	"setint.h"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" uint	elfhash(const void *,int) ;

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	setint		ints ;
	setint_cur	cur ;
	int		rs ;
	int		rs1 ;

	printf("main: ent\n") ;

	if ((rs = setint_start(&ints)) >= 0) {
	    if (rs >= 0) rs = setint_addval(&ints,2) ;
	    if (rs >= 0) rs = setint_addval(&ints,5) ;
	    if (rs >= 0) rs = setint_addval(&ints,17) ;
	    if ((rs = setint_curbegin(&ints,&cur)) >= 0) {
		int	v ;
		while ((rs1 = setint_enum(&ints,&cur,&v)) >= 0) {
		    printf("main: v=%d\n",v) ;
		} /* end while */
		if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
		rs1 = setint_curend(&ints,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (setint-cur) */
	    rs1 = setint_finish(&ints) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sethand) */

	printf("main: ret rs=%d\n",rs) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


