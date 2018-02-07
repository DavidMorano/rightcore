/* testobuf */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2012 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Test of the OBUF object.  It hardly needs any testing, really.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<exitcodes.h>
#include	"obuf.hh"

#ifndef	VARDEBUGFNAME
#define	VARDEBUGFNAME	"TESTOBUF_DEBUGFILE"
#endif

/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugprinthex(const char *,int,const char *,int) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **arvv,cchar **envv)
{
	obuf		o1 ;
	int		c ;
	int		rs = SR_OK ;
	int		ex = EX_OK ;

#if	CF_DEBUGS 
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

	o1.add('a') ;
	printf("o1.count=%u\n",o1.count()) ;
	{
		obuf o2("Hello world!") ;
		printf("o2.count=%u\n",o2.count()) ;
	}

	if (rs < 0) ex = EX_DATAERR ;
	return ex ;
}
/* end subroutine (main) */


