/* maintimer (timer) */
/* lang=C++11 */

/* test the TIMER facility */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_DEBUGN	1		/* special debugging */
#define	CF_SIGHAND	1		/* install signal handlers */


/* revision history:

	= 2017-10-12, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We (try to) test the TIMER facility.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<ucontext.h>
#include	<dlfcn.h>
#include	<limits.h>
#include	<string.h>
#include	<stdio.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<string>
#include	<ostream>
#include	<iostream>
#include	<vsystem.h>
#include	<sighand.h>
#include	<sigevent.h>
#include	<timespec.h>
#include	<itimerspec.h>
#include	<timeout.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"maininfo.h"


/* local defines */

#define	VARDEBUGFNAME	"TIMER_DEBUGFILE"
#define	NDF		"timer.deb"


/* type-defs */

typedef int (*timeout_met)(void *,uint,int) ;


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	uc_safesleep(int) ;

extern "C" int	snwcpy(char *,int,const char *,int) ;
extern "C" int	sncpy2(char *,int,const char *,const char *) ;
extern "C" int	sncpy2w(char *,int,const char *,const char *,int) ;
extern "C" int	sncpylc(char *,int,const char *) ;
extern "C" int	sncpyuc(char *,int,const char *) ;
extern "C" int	sfbasename(cchar *,int,cchar **) ;
extern "C" int	ucontext_rtn(ucontext_t *,long *) ;
extern "C" int	bufprintf(char *,int,cchar *,...) ;
extern "C" int	msleep(int) ;
extern "C" int	haslc(cchar *,int) ;
extern "C" int	hasuc(cchar *,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern "C" int	nprintf(cchar *,cchar *,...) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;
extern "C" cchar	*getourenv(const char **,const char *) ;
extern "C" cchar	*strsigabbr(int) ;

extern "C" void		uctimeout_fini() ;


/* local structures */

struct sigcode {
	int		code ;
	const char	*name ;
} ;


/* forward references */

static int	maininfo_time(MAININFO *,time_t,int) ;

static void	main_sighand(int,siginfo_t *,void *) ;
static int	main_sigdump(siginfo_t *) ;

static cchar	*strsigcode(const struct sigcode *,int) ;


/* local variables */

static const int	sigcatches[] = {
	SIGILL, 
	SIGSEGV,
	SIGBUS,
	SIGQUIT,
	SIGABRT,
	SIGALRM,
	SIGTIMEOUT,
	0
} ;

static const struct sigcode	sigcode_ill[] = {
	{ ILL_ILLOPC, "ILLOPC" },
	{ ILL_ILLOPN, "ILLOPN" },
	{ ILL_ILLADR, "ILLADR" },
	{ ILL_ILLTRP, "ILLTRP" },
	{ ILL_PRVOPC, "PRBOPC" },
	{ ILL_PRVREG, "PRVREG" },
	{ ILL_COPROC, "COPROC" },
	{ ILL_BADSTK, "BADSTK" },
	{ 0, NULL }
} ;

static const struct sigcode	sigcode_segv[] = {
	{ SEGV_MAPERR, "MAPERR" },
	{ SEGV_ACCERR, "ACCERR" },
	{ 0, NULL }
} ;

static const struct sigcode	sigcode_bus[] = {
	{ BUS_ADRALN, "ADRALN" },
	{ BUS_ADRERR, "ADRERR" },
	{ BUS_OBJERR, "OBJERR" },
	{ 0, NULL }
} ;


/* exported subroutines */


/* ARHSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	time_t		dt = time(NULL) ;
	const int	tval = 20 ;
#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = 0 ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	{
	    MAININFO	mi, *mip = &mi ;
	    if ((rs = maininfo_start(mip,argc,argv)) >= 0) {
		maininfohand_t	sh = main_sighand ;
	        if ((rs = maininfo_sigbegin(mip,sh,sigcatches)) >= 0) {
	            {
	                rs = maininfo_time(mip,dt,tval) ;
	            }
	            rs1 = maininfo_sigend(mip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (maininfo-sig) */
	        rs1 = maininfo_finish(mip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (maininfo) */
	} /* end block */

#if	CF_DEBUGN
	nprintf(NDF,"main: done rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_wn: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"main: ret rs=%d\n",rs) ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int maininfo_time(MAININFO *mip,time_t dt,int tval)
{
	TIMESPEC	ts ;
	const time_t	wt = (dt+(tval/2)) ;
	int		rs ;
	int		rs1 ;
	if ((rs = timespec_load(&ts,wt,0)) >= 0) {
	    ITIMERSPEC	it ;
	    if ((rs = itimerspec_load(&it,&ts,NULL)) >= 0) {
	        SIGEVENT	se ;
	        const int	st = SIGEV_SIGNAL ;
	        const int	sig = SIGTIMEOUT ;
	        const int	val = 0 ;
	        if ((rs = sigevent_load(&se,st,sig,val)) >= 0) {
	            const int	cid = CLOCK_REALTIME ;
	            timer_t	tid ;
	            if ((rs = uc_timercreate(cid,&se,&tid)) >= 0) {
			const int	tf = TIMER_ABSTIME ;

#if	CF_DEBUGN
	                nprintf(NDF,"main: uc_timer() rs=%d\n",rs) ;
#endif

			if ((rs = uc_timerset(tid,tf,&it,NULL)) >= 0) {
			    sigset_t	ss ;
			    siginfo_t	si ;
			    uc_sigsetempty(&ss) ;
			    uc_sigsetadd(&ss,sig) ;
	                    rs = uc_sigwaitinfo(&ss,&si) ;
	                    printf("wake from wait-info %d\n",rs) ;
			}

	                printf("done\n") ;
	                rs1 = uc_timerdestroy(tid) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (uc_timer) */
	        } /* end if (sigevent_load) */
	    } /* end if (itimerspec_load) */
	} /* end if (timespec_load) */
	return rs ;
}
/* end subroutine (maininfo_time) */


/* ARGSUSED */
static void main_sighand(int sn,siginfo_t *sip,void *vcp)
{
#if	CF_DEBUGN
	nprintf(NDF,"main_sighand: sn=%d(%s)\n",sn,strsigabbr(sn)) ;
#endif

	if (vcp != NULL) {
	    Dl_info	dl ;
	    long	ra ;
	    ucontext_t	*ucp = (ucontext_t *) vcp ;
	    void	*rtn ;
	    const int	wlen = LINEBUFLEN ;
	    int		wl ;
	    cchar	*fmt ;
	    char	wbuf[LINEBUFLEN+1] ;
	    ucontext_rtn(ucp,&ra) ;
	    if (ra != 0) {
	        rtn = (void *) ra ;
	        dladdr(rtn,&dl) ;
	        fmt = "rtn=%08lX fn=%s sym=%s\n" ;
	        wl = bufprintf(wbuf,wlen,fmt,ra,dl.dli_fname,dl.dli_sname) ;
	        write(2,wbuf,wl) ;
	    }
	}

	if (sip != NULL) {
	    main_sigdump(sip) ;
	}
	u_exit(EX_TERM) ;
}
/* end subroutine (main_sighand) */


static int main_sigdump(siginfo_t *sip)
{
	const int	wlen = LINEBUFLEN ;
	const int	si_signo = sip->si_signo ;
	const int	si_code = sip->si_code ;
	int		wl ;
	const char	*sn = strsigabbr(sip->si_signo) ;
	const char	*as = "*na*" ;
	const char	*scs = NULL ;
	const char	*fmt ;
	char		wbuf[LINEBUFLEN+1] ;
	char		abuf[16+1] ;
#if	CF_DEBUGN
	nprintf(NDF,"main_sighand: signo=%d\n",si_signo) ;
#endif
	switch (si_signo) {
	case SIGILL:
	    scs = strsigcode(sigcode_ill,si_code) ;
	    break ;
	case SIGSEGV:
	    scs = strsigcode(sigcode_segv,si_code) ;
	    bufprintf(abuf,16,"%p",sip->si_addr) ;
	    as = abuf ;
	    break ;
	case SIGBUS:
	    scs = strsigcode(sigcode_bus,si_code) ;
	    bufprintf(abuf,16,"%p",sip->si_addr) ;
	    as = abuf ;
	    break ;
	case SIGQUIT:
	    scs = "¤na¤" ;
	    break ;
	default:
	    scs = "¤default¤" ;
	    break ;
	} /* end switch */
	fmt = "SIG=%s code=%d(%s) addr=%s\n" ;
#if	CF_DEBUGN
	nprintf(NDF,"main_sighand: bufprintf() sn=%s\n",sn) ;
#endif
	wl = bufprintf(wbuf,wlen,fmt,sn,si_code,scs,as) ;
#if	CF_DEBUGN
	nprintf(NDF,"main_sighand: bufprintf() rs=%d\n",wl) ;
#endif
	write(2,wbuf,wl) ;
	return 0 ;
}
/* end subroutine (main_sigdump) */


static const char *strsigcode(const struct sigcode *scp,int code)
{
	int		i ;
	int		f = FALSE ;
	const char	*sn = "UNKNOWN" ;
	for (i = 0 ; scp[i].code != 0 ; i += 1) {
	    f = (scp[i].code == code) ;
	    if (f) break ;
	}
	if (f) sn = scp[i].name ;
	return sn ;
}
/* end subroutine (strsigcode) */


