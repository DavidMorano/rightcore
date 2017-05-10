/* testpcsngdname */
/* lang=C89 */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	BBNEWSDNAME
#define	BBNEWSDNAME	"spool/boards"
#endif

#define	VARDEBUGFNAME	"TESTPCSNGDNAME_DEBUGFILE"

extern int	pcsngdname(const char *,char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	const int	rlen = MAXPATHLEN ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*pr = PCS ;
	const char	*newsdname = BBNEWSDNAME ;

	char	rbuf[MAXPATHLEN+1] = { 0 } ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if (argv != NULL) {
	    int	ai ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
		const char	*np = argv[ai] ;

	        rs = pcsngdname(pr,rbuf,newsdname,np) ;

#if	CF_DEBUGS
	        debugprintf("main: pcsngdname() rs=%d\n",rs) ;
#endif

		if (rs < 0) break ;
	    } /* end for */
	} /* end if (argv) */
#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

	printf("rs=%d ngdname=%s\n",rs,rbuf) ;

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

