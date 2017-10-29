/* testinit (C89) */

#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<getbufsize.h>
#include	<hostent.h>
#include	<localmisc.h>

#ifndef	VARDEBUGFNAME
#define	VARDEBUGFNAME	"TESTGETHOST_DEBUGFILE"
#endif

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

int main(int argc,const char **argv,const char **envv)
{
	int		rs = SR_OK ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if (argc > 0) {
	    HOSTENT	he ;
	    const int	helen = getbufsize(getbufsize_he) ;
	    char	*hebuf ;
	    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    	int	ai ;
	        for (ai = 1 ; ai < argc ; ai += 1) {
	            cchar	*name = argv[ai] ;
	            if (name[0] != '\0') {
	                rs = uc_gethostbyname(name,&he,hebuf,helen) ;
		        printf("rs=%d \n",rs) ;
	            }
	        } /* end for */
		uc_free(hebuf) ;
	    } /* end if (memory-allocation) */
	} /* end if (positive) */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

