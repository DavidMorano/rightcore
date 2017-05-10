/* testmkuserpath */
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

#define	VARDEBUGFNAME	"TESTMKUSERPATH_DEBUGFILE"

extern int	mkuserpath(char *,const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	int		rs = SR_OK ;
	char		rbuf[MAXPATHLEN+1] ;

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
		rs = mkuserpath(rbuf,NULL,argv[ai],-1) ;
		printf("rs=%d p=%s\n",rs,rbuf) ;
		if (rs < 0) break ;
	    } /* end for */
	} /* end if (argv) */

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


