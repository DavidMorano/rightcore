/* main (numwords) */
/* lang=C++11 */

/* convert a Number to Words */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-08-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes numbers (positive integers) as arguments and
        creates a string containing the necessary words to describe each given
        number.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
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
#include	<cfdec.h>
#include	<ctwords.hh>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"NUMWORDS_DEBUGFILE" 


/* name-spaces */

using namespace std ;


/* external subroutines */

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


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */
	if (argc > 1) {
	    int	ai ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
		int	v ;
		cchar	*a = argv[ai] ;
		if ((rs = cfdeci(a,-1,&v)) >= 0) {
		    string	s ;
		    rs = ctwords(&s,v) ;
		    cout << s << endl ;
		}
		if (rs < 0) break ;
	    } /* end for */
	}
#if	CF_DEBUGS
	debugclose() ;
#endif
	return 0 ;
}
/* end subroutine (main) */


