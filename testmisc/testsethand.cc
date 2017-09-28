/* testsethand */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013 -07-11, David A­D­ Morano
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

#include	"sethand.h"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" uint	elfhash(const void *,int) ;

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */

class ourobj : public sethandent {
    int		v = 0 ;
    uint        gethash() const {
	return elfhash(kp,kl) ;
    } ;
    int		getval() const {
	return v ;
    } ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	sethand		s(5) ;
	int		rs ;
	int		rs1 ;

	printf("main: ent\n") ;

	if ((rs = s.begin()) >= 0) {
	    ourobj	*ep ;
	    sethand_it	c ;
	    {

	    } /* end block */
	    if ((rs = s.itbegin(&c)) >= 0) {
		while ((rs1 = s.itenum(&d,&ep)) >= 0) {


		} /* end while */
		if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	        rs1 = s.end() ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sethand-it) */
	} /* end if (sethand) */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


