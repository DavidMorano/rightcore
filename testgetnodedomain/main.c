/* main */


#define	CF_DEBUGS	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<stdio.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


extern int	getnodedomain(char *,char *) ;


int main(int argc,cchar *argv[],cchar *envv[])
{
	int	rs ;

	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	rs = getnodedomain(nodename,domainname) ;

	fprintf(stdout,"main: getnodedomain() rs=%d n=%s d=%s\n",
		rs,nodename,domainname) ;

	fflush(stdout) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


