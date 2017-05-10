/* teststring */
/* lang=C++98 */

#define	CF_DEBUGS	1
#include	<envstandards.h>

#include	<string>

#include	<stdio.h>
#include	<vsystem.h>


#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#define	VARDEBUGFNAME	"TESTSTRING_DEBUGFILE"

using namespace		std ;

#if	CF_DEBUGS
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	string		s ;

	int	rs ;

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

		s += np ;

		if (rs < 0) break ;
	    } /* end for */
	} /* end if (argv) */
	debugprintf("main: out rs=%d\n",rs) ;

	printf("ngdir=%s\n",s.c_str()) ;

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

