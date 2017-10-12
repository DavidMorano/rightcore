/* testugetpid */
/* lang=C99 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_DEBUGN	0		/* special debugging */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-10-06, David A­D­ Morano
	Updated and enhanced.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<ugetpid.h>
#include	<localmisc.h>


#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTUGETPID_DEBUGFILE"

#define	NDF		"testugetpid.deb"

extern int	bufprintf(char *,int,cchar *,...) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar 	*getourenv(cchar **,cchar *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */

/* exported subroutines */

/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*ofp = stdout ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting fd=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"main: inside\n") ;
#endif

	rs = ugetpid() ;
	fprintf(ofp,"p ugetpid() rs=%d\n",rs) ;

	fflush(ofp) ;

#if	CF_DEBUGS
	debugprintf("main: fork()\n") ;
#endif

	if ((rs = uc_fork()) == 0) { /* child */

	    rs = ugetpid() ;
	    fprintf(ofp,"c ugetpid() rs=%d\n",rs) ;

	    u_exit(0) ;
	} else if (rs > 0) { /* parent */
	    int	cs ;
	    u_wait(&cs) ;
	    fprintf(ofp,"cs=%u\n",cs) ;
	} /* end if */

	if (rs < 0)
	fprintf(ofp,"failure (%d)\n",rs) ;

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"main: leaving\n") ;
#endif

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


