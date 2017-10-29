/* testgetuserhome */
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

#define	VARDEBUGFNAME	"TESTGETUSERHOME_DEBUGFILE"

extern int	getuserhome(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

int main(int argc,const char **argv,const char **envv)
{
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	char		hbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

	if ((argv != NULL) && (argc > 1)) {
	    int	ai ;
	    for (ai = 1 ; argv[ai] != NULL ; ai += 1) {
	        const char	*un = argv[ai] ;
	        rs = getuserhome(hbuf,hlen,un) ;
	        printf("un=%s rs=%d hd=%s\n",un,rs,hbuf) ;
	    } /* end for */
	} /* end if */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


