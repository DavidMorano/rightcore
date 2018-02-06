/* testsetstr */

/* we test the SETSTR (a set of strings) object */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocations */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ucmallreg.h>
#include	<localmisc.h>
#include	"setstr.h"


#define	VARDEBUGFNAME	"TESTSETSTR_DEBUGFILE"
#define	VARDEBUGFD1	"TESTSETSTR_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"


/* external subroutines */

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;

/* exported subroutines */

int main(int argc,cchar **argv,cchar **envv)
{
	SETSTR		ss ;
	FILE		*ofp = stdout ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	{
	    cchar		*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if (argc > 1) {
	    if ((rs = setstr_start(&ss)) >= 0) {
	        int	i ;
#if	CF_DEBUGS
	debugprintf("testsetstr: insert1\n") ;
#endif
		if (rs >= 0) {
		    for (i = 1 ; i < argc ; i += 1) {
		        rs = setstr_add(&ss,argv[i],-1) ;
		        if (rs < 0) break ;
		    }
		}
#if	CF_DEBUGS
	debugprintf("testsetstr: insert2\n") ;
#endif
		if (rs >= 0) {
		    for (i = 1 ; i < argc ; i += 1) {
		        rs = setstr_add(&ss,argv[i],-1) ;
		        if (rs < 0) break ;
		    }
		}
#if	CF_DEBUGS
	debugprintf("testsetstr: retrieve\n") ;
#endif
		if (rs >= 0) {
		    SETSTR_CUR	c ;
		    if ((rs = setstr_curbegin(&ss,&c)) >= 0) {
			cchar	*cp ;
			while ((rs1 = setstr_enum(&ss,&c,&cp)) >= 0) {
	
#if	CF_DEBUGS
			debugprintf("testsetstr: s=%s\n",cp) ;
#endif
	    		    fprintf(ofp,"main: s=%s\n",cp) ;

			    if (rs < 0) break ;
			} /* end while */
			if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
			rs1 = setstr_curend(&ss,&c) ;
			if (rs >= 0) rs = rs1 ;
		    }
		}
		if (rs >= 0) {
			cchar	*cp = "one" ;
			if ((rs = setstr_already(&ss,cp,-1)) > 0) {
	    		    fprintf(ofp,"main: already s=%s\n",cp) ;
			}
		}
	        rs1 = setstr_finish(&ss) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (setstr) */
	} /* end if (arguments) */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


