/* testcomparse */
/* lang=C89 */

#define	CF_DEBUGS	1

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	"comparse.h"

#define	VARDEBUGFNAME	"TESTCOMPARSE_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

int main(int argc,const char **argv,const char **envv)
{
	COMPARSE	coms ;
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	const char	*sp = "this is ( a comment )" ;
	char		hbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if ((rs = comparse_start(&coms,sp,-1)) >= 0) {
	    const char	*vp, *cp ;
	    int		vl, cl ;

	    rs = comparse_getval(&coms,&vp) ;
	    vl = rs ;
	    printf("getval() rs=%d\n",rs) ;
	    printf("getval() v=>%s<\n",vp) ;

	    rs = comparse_getcom(&coms,&cp) ;
	    cl = rs ;
	    printf("getcom() rs=%d\n",rs) ;
	    printf("getcom() c=>%s<\n",cp) ;

	    comparse_finish(&coms) ;
	} /* end if (comparse) */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

