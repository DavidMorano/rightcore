/* main */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGN	1		/* special debugging */
#define	CF_SIGDUMPER	0		/* test |sigdumper()| */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test some aspects of signal operation on this OS.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<ucontext.h>
#include	<dlfcn.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<ascii.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"sighand.h"


/* local defines */

#define	SIGDUMPFILE	"sigdumper"

#define	INT_SLEEP	20

#define	NDF		"main.deb"


/* external variables */

extern int	ucontext_rtn(ucontext_t *,long *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	msleep(int) ;

#if	CF_SIGDUMPER
extern int	sigdumper(const char *,int,const char *) ;
#endif /* CF_SIGDUMPER */

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;
extern const char	*strsigabbr(int) ;


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif

struct sigcode {
	int		code ;
	const char	*name ;
} ;


/* forward references */

static void	main_sigint(int,siginfo_t *,void *) ;
static void	main_sighand(int,siginfo_t *,void *) ;
static int	main_sigdump(siginfo_t *) ;

static cchar	*strsigcode(const struct sigcode *,int) ;


/* local variables */

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ SR_DOM, EX_NOPROG },
	{ 0, 0 }
} ;

static const int	sigcatches[] = {
	SIGILL, 
	SIGSEGV,
	SIGBUS,
	SIGQUIT,
	SIGABRT,
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

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	0
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_OK ;

/* test SIGDUMPER */

#if	CF_SIGDUMPER
	{
	const pid_t	pid = getpid() ;
	sigdumper(SIGDUMPFILE,pid,"s0") ;
	sleep(10) ;
	sigdumper(SIGDUMPFILE,pid,"s1") ;
	} /* end block */
#endif /* CF_SIGDUMPER */

/* test taking a signal and printing out information */

	if (rs >= 0) {
	    SIGHAND	sh ;
	    void	(*sf)(int,siginfo_t *,void *) = main_sighand ;
	    if ((rs = sighand_start(&sh,NULL,NULL,sigcatches,sf)) >= 0) {
	        SIGHAND	si ;
	        void	(*sf)(int,siginfo_t *,void *) = main_sigint ;
	        if ((rs = sighand_start(&si,NULL,NULL,sigints,sf)) >= 0) {
		    const time_t	ti_start = time(NULL) ;
		    time_t		ti_now ;

		    ti_now = ti_start ;
		    while ((ti_now-ti_start) < INT_SLEEP) {
		        sleep(1) ;
		        fprintf(stdout,"loop\n") ;
		        ti_now = time(NULL) ;
		    } /* end while */

	            rs1 = sighand_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (sighand) */
	        rs1 = sighand_finish(&sh) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sighand) */
	} /* end block */

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


/* ARGSUSED */
static void main_sigint(int sn,siginfo_t *sip,void *vcp)
{
	const int	wlen = LINEBUFLEN ;
	int		wl ;
	cchar		*fmt = "sig=%u\n" ;
	char		wbuf[LINEBUFLEN+1] ;
#if	CF_DEBUGN
	nprintf(NDF,"main_sigint: sn=%d(%s)\n",sn,strsigabbr(sn)) ;
#endif
	wl = bufprintf(wbuf,wlen,fmt,sn) ;
	write(2,wbuf,wl) ;

	if (vcp != NULL) {
	    Dl_info	dl ;
	    long	ra ;
	    ucontext_t	*ucp = (ucontext_t *) vcp ;
	    void	*rtn ;
	    ucontext_rtn(ucp,&ra) ;
	    rtn = (void *) ra ;
	    dladdr(rtn,&dl) ;
	    fmt = "rtn=%08lX fn=%s sym=%s\n" ;
	    wl = bufprintf(wbuf,wlen,fmt,ra,dl.dli_fname,dl.dli_sname) ;
	    write(2,wbuf,wl) ;
	}
}
/* end subroutine (main_sigint) */


/* ARGSUSED */
static void main_sighand(int sn,siginfo_t *sip,void *vcp)
{
	const int	wlen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		wl ;
	cchar		*fmt = "sig=%u\n" ;
	char		wbuf[LINEBUFLEN+1] ;
#if	CF_DEBUGN
	nprintf(NDF,"main_sighand: sn=%d(%s)\n",sn,strsigabbr(sn)) ;
#endif
	wl = bufprintf(wbuf,wlen,fmt,sn) ;
	write(2,wbuf,wl) ;

	if (vcp != NULL) {
	    SBUF	b ;
	    ucontext_t	*ucp = (ucontext_t *) vcp ;
	    mcontext_t	*mcp ;
	    greg_t	*rp ;
	    mcp = &ucp->uc_mcontext ;
	    rp = mcp->gregs ;
	    if ((rs = sbuf_start(&b,wbuf,wlen)) >= 0) {
	        int	i ;
	        for (i = 0 ; i < _NGREG ; i += 1) {
		    sbuf_printf(&b,"%02u",i) ;
		    sbuf_char(&b,' ') ;
	            sbuf_hexul(&b,rp[i]) ;
		    sbuf_char(&b,'\n') ;
	        }
	        wl = sbuf_finish(&b) ;
		write(2,wbuf,wl) ;
	    } /* end if (sbuf) */
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


