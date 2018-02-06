/* main (liblkcmd) */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_UTIL		0		/* run the utility worker */
#define	CF_SIGHAND	1		/* install signal handlers */
#define	CF_SIGALTSTACK	0		/* do *not* define */


/* revision history:

	= 1998-07-28, David A­D­ Morano
	This subroutine was written for use as a front-end for Korn Shell (KSH)
	commands that are compiled as stand-alone programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end to make the various SHELL (KSH) built-in commands
	into stand-alone programs.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<ucontext.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<intceil.h>
#include	<vecstr.h>
#include	<sighand.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"kshlib.h"
#include	"maininfo.h"


/* local defines */

#if	defined(KSHBUILTIN) && (KSHBUILTIN > 0)
#define	CF_LOCKMEMALLOC		1
#else
#define	CF_LOCKMEMALLOC		0
#endif

#define	NDF		"main.deb"


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	ucontext_rtn(ucontext_t *,long *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	msleep(int) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;
extern cchar	*strsigabbr(int) ;


/* external variables */


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

static int	maininfo_sigbegin(MAININFO *) ;
static int	maininfo_sigend(MAININFO *) ;

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


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	const int	f_lockmemalloc = CF_LOCKMEMALLOC ;
	const int	f_util = CF_UTIL ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;

#if	CF_DEBUGN
	nprintf(NDF,"main: ent\n") ;
#endif

	if (argv != NULL) {
	    MAININFO	mi, *mip = &mi ;
	    if ((rs = maininfo_start(mip,argc,argv)) >= 0) {
	        if ((rs = maininfo_sigbegin(mip)) >= 0) {
#if	CF_DEBUGN
	            nprintf(NDF,"main: sig-begin\n") ;
#endif
	            if ((rs = lib_initmemalloc(f_lockmemalloc)) >= 0) {
	                if ((rs = lib_mainbegin(envv,NULL)) >= 0) {
	                    if ((rs = maininfo_utilbegin(mip,f_util)) >= 0) {
	                        const char	*srch ;

#if	CF_DEBUGN
	                        nprintf(NDF,"main: progname=%s\n",pn) ;
#endif

	                        if ((rs = maininfo_srchname(mip,&srch)) >= 0) {
#if	CF_DEBUGN
	                            nprintf(NDF,"main: srch=%s\n",srch) ;
#endif
	                            ex = lib_callcmd(srch,argc,argv,envv,NULL) ;
#if	CF_DEBUGN
	                            nprintf(NDF,"main: lib_callcmd() ex=%u\n",
	                                ex) ;
#endif
	                        } /* end if */

	                        rs1 = maininfo_utilend(mip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (maininfo-util) */
	                    rs1 = lib_mainend() ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (lib-main) */
	            } /* end if (lib_initmemalloc) */
#if	CF_DEBUGN
	            nprintf(NDF,"main: sig-end\n") ;
#endif
	            rs1 = maininfo_sigend(&mi) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (maininfo-sig) */
	        rs1 = maininfo_finish(mip) ;
	        if (rs >= 0) rs = rs1 ;
	    } else
	        ex = EX_OSERR ;
	} else
	    ex = EX_OSERR ;

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_SIGALTSTACK
static int maininfo_sigbegin(MAININFO *mip)
{
	size_t		ms ;
	const int	ps = getpagesize() ;
	const int	ss = (2*SIGSTKSZ) ;
	int		rs ;
	int		mp = (PROT_READ|PROT_WRITE) ;
	int		mf = (MAP_PRIVATE|MAP_NORESERVE|MAP_ANON) ;
	int		fd = -1 ;
	void		*md ;
	ms = iceil(ss,ps) ;
	if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	    mip->mdata = md ;
	    mip->msize = ms ;
	    mip->astack.ss_size = ms ;
	    mip->astack.ss_sp = md ;
	    mip->astack.ss_flags = 0 ;
	    if ((rs = u_sigaltstack(&mip->astack,NULL)) >= 0) {
	        void	(*sh)(int,siginfo_t *,void *) = main_sighand ;
	        rs = sighand_start(&mip->sh,NULL,NULL,sigcatches,sh) ;
	        if (rs < 0) {
	            mip->astack.ss_flags = SS_DISABLE ;
	            u_sigaltstack(&mip->astack,NULL) ;
	        }
	    } /* end if (u_sigaltstack) */
	    if (rs < 0) {
	        u_munmap(mip->mdata,mip->msize) ;
	        mip->mdata = NULL ;
	    }
	} /* end if (mmap) */
#if	CF_DEBUGN
	nprintf(NDF,"maininfo_sigbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (maininfo_sigbegin) */
#else /* CF_SIGALTSTACK */
static int maininfo_sigbegin(MAININFO *mip)
{
	int		rs = SR_OK ;
	void		(*sh)(int,siginfo_t *,void *) = main_sighand ;
#if	CF_SIGHAND
	rs = sighand_start(&mip->sh,NULL,NULL,sigcatches,sh) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"maininfo_sigbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (maininfo_sigbegin) */
#endif /* CF_SIGALTSTACK */


static int maininfo_sigend(MAININFO *mip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SIGHAND
	rs1 = sighand_finish(&mip->sh) ;
	if (rs >= 0) rs = rs1 ;
#endif

#if	CF_SIGALTSTACK
	mip->astack.ss_flags = SS_DISABLE ;
	rs1 = u_sigaltstack(&mip->astack,NULL) ;
	if (rs >= 0) rs = rs1 ;

	if (mip->mdata != NULL) {
	    rs1 = u_munmap(mip->mdata,mip->msize) ;
	    if (rs >= 0) rs = rs1 ;
	    mip->mdata = NULL ;
	    mip->msize = 0 ;
	}
#endif /* CF_SIGALTSTACK */

	return rs ;
}
/* end subroutine (maininfo_sigend) */


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


