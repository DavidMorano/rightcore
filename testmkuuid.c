/* testmkuuid */
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
#include	<mkuuid.h>
#include	<snmkuuid.h>
#include	<localmisc.h>

#define	VARDEBUGFNAME	"TESTMKUUID_DEBUGFILE"

extern int	mkuuid(MKUUID *,int) ;
extern int	snmkuuid(char *,int,MKUUID *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(cchar **,cchar *) ;


int main(int argc,const char **argv,const char **envv)
{
	MKUUID		uuid ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if ((rs = mkuuid(&uuid,0)) >= 0) {
	    const int	rlen = MAXPATHLEN ;
	    char	rbuf[MAXPATHLEN+1] ;
#if	CF_DEBUGS
	    debugprinthex("main: time=",80,&uuid.time,8) ;
	    debugprinthex("main: clk=",80,&uuid.clk,8) ;
	    debugprinthex("main: node=",80,&uuid.node,8) ;
#endif
	    if ((rs = snmkuuid(rbuf,rlen,&uuid)) >= 0) {
		printf("rs=%d uuid=%s\n",rs,rbuf) ;
	    }
	} /* end if (argv) */

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


