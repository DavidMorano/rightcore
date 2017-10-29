/* testutmpacc */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<utmpacc.h>
#include	<localmisc.h>

#ifndef	UEBUFLEN
#define	UEBUFLEN	UTMPACCENT_BUFLEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTUTMPACC_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;


int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	{
	    time_t	bt ;
	    if ((rs = utmpacc_boottime(&bt)) >= 0) {
	        char	timebuf[TIMEBUFLEN+1] ;
	        timestr_logz(bt,timebuf) ;
	        printf("bt=%s\n",timebuf) ;
	    }
	}

#if	CF_DEBUGS
	        debugprintf("main: utmpacc_boottime() rs=%d\n",rs) ;
#endif

	{
	    if ((rs = utmpacc_runlevel()) >= 0) {
	        printf("runlevel=%c\n",rs) ;
	    }
	}

#if	CF_DEBUGS
	        debugprintf("main: utmpacc_runlevel() rs=%d\n",rs) ;
#endif

	{
	    if ((rs = utmpacc_users(0)) >= 0) {
	        printf("nusers=%d\n",rs) ;
	    }
	}

#if	CF_DEBUGS
	        debugprintf("main: utmpacc_users() rs=%d\n",rs) ;
#endif

	{
	    UTMPACCENT	ue ;
	    const int	uelen = UEBUFLEN ;
	    char	uebuf[UEBUFLEN+1] ;
	    if ((rs = utmpacc_entsid(&ue,uebuf,uelen,-1)) >= 0) {
	        printf("sid=%d\n",ue.sid) ;
	        printf("name=%s\n",ue.user) ;
	        printf("line=%s\n",ue.line) ;
	        printf("host=%s\n",ue.host) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("main: utmpacc_ent() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    struct utmpacc_stats	s ;
	    if ((rs = utmpacc_stats(&s)) >= 0) {
	        printf("stats max=%u\n",s.max) ;
	        printf("stats ttl=%u\n",s.ttl) ;
	        printf("stats nent=%u\n",s.nent) ;
	        printf("stats acc=%u\n",s.acc) ;
	        printf("stats phit=%u\n",s.phit) ;
	        printf("stats pmis=%u\n",s.pmis) ;
	        printf("stats nhit=%u\n",s.nhit) ;
	        printf("stats nmis=%u\n",s.nmis) ;
	    }
#if	CF_DEBUGS
	    debugprintf("main: utmpacc_stats() rs=%d\n",rs) ;
#endif
	} /* end if */

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


