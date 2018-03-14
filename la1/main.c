/* main (liblkcmd) */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGN	0		/* special debugging */
#define	CF_UTIL		0		/* run the utility worker */


/* revision history:

	= 2001-11-01, David A­D­ Morano
	This subroutine was written for use as a front-end for Korn Shell (KSH)
	commands that are compiled as stand-alone programs.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end to make the various SHELL (KSH) built-in commands
	into stand-alone programs.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<signal.h>
#include	<ucontext.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<intceil.h>
#include	<sighand.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"kshlib.h"
#include	"maininfo.h"


/* local defines */

#if	defined(KSHBUILTIN) && (KSHBUILTIN > 0)
#define	CF_LOCKMEMALLOC		0	/* formerly =1 */
#else
#define	CF_LOCKMEMALLOC		0
#endif

#define	NDF		"main.deb"


/* typ-defs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	ucontext_rtn(ucontext_t *,long *) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	msleep(int) ;
extern int	haslc(cchar *,int) ;
extern int	hasuc(cchar *,int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;
extern cchar	*strsigabbr(int) ;


/* external variables */


/* local structures */

struct sigcode {
	int		code ;
	const char	*name ;
} ;


/* forward references */

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
		maininfohand_t	sh = main_sighand ;
	        if ((rs = maininfo_sigbegin(mip,sh,sigcatches)) >= 0) {
#if	CF_DEBUGN
	            nprintf(NDF,"main: sig-begin\n") ;
#endif
	            if ((rs = lib_initmemalloc(f_lockmemalloc)) >= 0) {
	                if ((rs = lib_mainbegin(envv,NULL)) >= 0) {
	                    if ((rs = maininfo_utilbegin(mip,f_util)) >= 0) {
	                        cchar	*srch ;

#if	CF_DEBUGN
	                        nprintf(NDF,"main: maininfo_srchname()\n") ;
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

#if	CF_DEBUGN
	                        nprintf(NDF,
				"main: maininfo_srchname-out rs=%d ex=%u\n",
				rs,ex) ;
#endif

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
	    } else {
	        ex = EX_OSERR ;
	    }
	} else {
	    ex = EX_OSERR ;
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"main: ret rs=%d ex=%u\n",rs,ex) ;
#endif

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
	cchar		*sn = "UNKNOWN" ;
	for (i = 0 ; scp[i].code != 0 ; i += 1) {
	    f = (scp[i].code == code) ;
	    if (f) break ;
	}
	if (f) sn = scp[i].name ;
	return sn ;
}
/* end subroutine (strsigcode) */


