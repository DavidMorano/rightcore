/* testprgetprogpath */


#define	CF_DEBUGS	1		/* compile-time debugging */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdio.h>

#include	"testprgetprogpath_config.h"


#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


extern const char	*getourenv(const char **,const char *) ;


int main(int argc, const char **argv,const char **envv)
{
	int	rs = 0 ;

	const char	*pr = "/usr/add-on/local" ;
	const char	*pn = "imail" ;
	char		progfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = prgetprogpath(pr,progfname,pn) ;

	printf("rs=%d progfname=%s\n",rs,progfname) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */



