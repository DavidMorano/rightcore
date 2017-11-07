/* maincallobj (calobj) */
/* lang=C++11 */

/* this is a test of initiating and receiving time-out call-back calls */


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

	We (try to) test the idea of receiving time-out call-backs.


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
#include	<stdexcept>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<thread>
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
#include	"ccmutex.hh"


/* local defines */

#define	VARDEBUGFNAME	"CALLOBJ_DEBUGFILE"
#define	NDF		"callobj.deb"


/* type-defs */

typedef int (*timeout_met)(void *,uint,int) ;


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	uc_timeout(int,TIMEOUT *) ;
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

static int	ourobj_hand(void *,uint,int) ;

struct sigcode {
	int		code ;
	const char	*name ;
} ;

class ourobj {
	TIMEOUT		to ;
	ccmutex		m ;
	int		c = 0 ;
	bool		f_running = false ;
public:
	ourobj() {
	    timeout_load(&to,0L,NULL,ourobj_hand,0U,0) ;
#if	CF_DEBUGS
	    debugprintf("main/ourobj::ctor: ent\n") ;
#endif
	} ;
	~ourobj() {
#if	CF_DEBUGS
	    debugprintf("main/ourobj::dtor: ent\n") ;
#endif
	    if (f_running) {
	        const int	cmd = timeoutcmd_cancel ;
		f_running = false ;
		uc_timeout(cmd,&to) ;
	    }
#if	CF_DEBUGS
	    debugprintf("main/ourobj::dtor: ret\n") ;
#endif
	} ;
	int set(int valint,uint tag,int arg) {
	    int		rs = SR_OK ;
#if	CF_DEBUGS
	    debugprintf("main/ourobj::set: val=%d\n",valint) ;
#endif
	    if (valint >= 0) {
	        const time_t	dt = time(NULL) ;
		time_t		valabs ;
	        if (f_running) {
	            const int	cmd = timeoutcmd_cancel ;
		    f_running = false ;
		    rs = uc_timeout(cmd,&to) ;
	        }
		if (rs >= 0) {
		    timeout_met	met = ourobj_hand ;
		    valabs = (dt+valint) ;
		    if ((rs = timeout_load(&to,valabs,this,met,tag,arg)) >= 0) {
			const int	cmd = timeoutcmd_set ;
			if ((rs = uc_timeout(cmd,&to)) >= 0) {
			    f_running = true ;
			}
#if	CF_DEBUGS
	    	    debugprintf("main/ourobj::set: uc_timeout() rs=%d\n",rs) ;
#endif
		    }
		}
	    } else {
		rs = SR_INVALID ;
	    }
	    return rs ;
	} ;
	int cancel() {
	    int		rs = SR_OK ;
	    if (f_running) {
	        const int	cmd = timeoutcmd_cancel ;
		f_running = false ;
		rs = uc_timeout(cmd,&to) ;
	    }
	    return rs ;
	} ;
	int timeout(uint tag,int arg) {
	    guardmutex		lck(m) ;
	    int			rs ;
#if	CF_DEBUGS
	    debugprintf("main/ourobj::timeout: ent\n") ;
#endif
	    cout << "timeout tag=" << tag << " arg=" << arg << endl ;
	    f_running = false ;
	    rs = set(4,0x5a,++c) ;
#if	CF_DEBUGS
	    debugprintf("main/ourobj::timeout: ret rs=%d\n",rs) ;
#endif
	    return rs ;
	} ;
} ; /* end structure (ourobj) */

/* in theory should be marked extern-C to get correct call convention */
static int ourobj_hand(void *objp,uint tag,int arg)
{
	ourobj		*cop = (ourobj *) objp ;
	return cop->timeout(tag,arg) ;
}
/* end subroutine (ourobj_hand) */


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
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
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
	ourobj		os ;
	int		rs ;
	if ((rs = os.set(4,0xC4,0)) >= 0) {
	    sleep(tval) ;
	}
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


