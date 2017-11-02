/* testsigpending */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_SIGPENDING	1		/* showpending */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<sigblock.h>
#include	<localmisc.h>

#define	VARDEBUGFNAME	"TESTSIGPENDING_DEBUGFILE"
#define	INT_WAIT	10

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*strsigabbr(uint) ;
extern cchar 	*getourenv(cchar **,cchar *) ;


/* forward references */

static int showpending() ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	{
	    sigset_t	nsm, osm ;
	    const int	sig = SIGINT ;
	    uc_sigsetempty(&nsm) ;
	    if ((rs = uc_sigsetadd(&nsm,sig)) >= 0) {
	        if ((rs = u_sigprocmask(SIG_BLOCK,&nsm,&osm)) >= 0) {
		    SIGBLOCK	b ;
		    if ((rs = sigblock_start(&b,NULL)) >= 0) {
			const time_t	st = time(NULL) ;
			time_t		dt ;

			dt = st ;
			while ((dt-st) < INT_WAIT) {

	                    msleep(100) ;

#if	CF_SIGPENDING
	                    showpending() ;
#endif

			    dt = time(NULL) ;
			} /* end while */
			sigblock_finish(&b) ;
		    } /* end if (sigblock) */
	            u_sigprocmask(SIG_SETMASK,&osm,NULL) ;
	        } /* end if (sigmask) */
	    } /* end if */
	} /* end block */

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int showpending()
{
	sigset_t	psm ;
	int		rs ;
	if ((rs = u_sigpending(&psm)) >= 0) {
	    int	i ;
	    for (i = 0 ; i < SIGRTMIN ; i += 1) {
	        if ((rs = uc_sigsetismem(&psm,i)) > 0) {
		    cchar	*cp = strsigabbr(i) ;
	            fprintf(stdout,"main/showpending: sig=%s(%d)\n",cp,i) ;
		    fflush(stdout) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
#if	CF_DEBUGS
	    debugprintf("main/showpending: for-out rs=%d nsigs=%d\n",rs,i) ;
#endif
	}
#if	CF_DEBUGS
	debugprintf("main/showpending: out rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (showpending) */


