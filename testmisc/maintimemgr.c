/* maintimeout (timeout) */
/* lang=C++11 */

/* test the TIMEOUT facility */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_DEBUGN	1		/* special debugging */
#define	CF_SIGHAND	1		/* install signal handlers */
#define	CF_CALLFINI	0		/* call |uctimeout_fini()| */


/* revision history:

	= 2017-10-12, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We (try to) test the TIMEOUT facility.


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
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"timeout.h"
#include	"maininfo.h"


/* local defines */

#define	VARDEBUGFNAME	"TIMEOUT_DEBUGFILE"
#define	NDF		"uctimeout.deb"


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
extern "C" cchar	*strsigabbr(int) ;

#if	CF_CALLFINI
extern "C" void		uctimeout_fini() ;
#endif


/* local structures */

struct sigcode {
	int		code ;
	const char	*name ;
} ;


/* forward references */

static void	main_sighand(int,siginfo_t *,void *) ;
static int	main_sigdump(siginfo_t *) ;

static cchar	*strsigcode(const struct sigcode *,int) ;

static int	ourwake(void *,uint,int) ;


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
	TIMEOUT		to ;
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
	            const time_t	wake = (dt+(tval/2)) ;
	            if ((rs = timeout_load(&to,wake,NULL,ourwake,42,1)) >= 0) {
	                const int	cmd = timeoutcmd_set ;
	                if ((rs = uc_timeout(cmd,&to)) >= 0) {
	                    const int	id = rs ;

#if	CF_DEBUGN
	                    nprintf(NDF,"main: uc_timeout() rs=%d\n",rs) ;
#endif

	                    printf("id=%d\n",id) ;
	                    uc_safesleep(tval/2) ;

#if	CF_DEBUGN
	                    nprintf(NDF,"main: back rs=%d\n",rs) ;
#endif

	                    printf("done\n") ;
	                } /* end if (uc_timeout) */
	            } /* end if (timeout_load) */
	            rs1 = maininfo_sigend(&mi) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (maininfo-sig) */
	        rs1 = maininfo_finish(mip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (maininfo) */
	} /* end block */

#if	CF_DEBUGN
	nprintf(NDF,"main: done rs=%d\n",rs) ;
#endif

#if	CF_CALLFINI
	uctimeout_fini() ;
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
	nprintf(NDF,"main_sighand: sn=%s\n",sn) ;
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


static int ourwake(void *objp,uint tag,int arg)
{
	printf("int objp=%p tag=%u arg=%d\n",objp,tag,arg) ;
	return 0 ;
}
/* end subroutine (ourwake) */


