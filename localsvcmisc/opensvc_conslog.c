/* opensvc_conslog */

/* LOCAL facility open-service (conslog) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano

	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather
	I should have started this code by using the corresponding UUX
	dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an user open-service module.  This little diddy supplies
	a fast little "issue" message for programs that perform logins
	onto the host.

	Synopsis:

	int opensvc_conslog(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/log.h>		/* for LOG_MAXPS */
#include	<sys/strlog.h>		/* interface definitions */
#include	<sys/syslog.h>		/* for all other 'LOG_xxx' */
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_conslog.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGDEV
#define	LOGDEV		"/dev/conslog"
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	NDEBFNAME	"opensvc_conslog.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getlogfac(const char *,int) ;
extern int	getnodename(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	localgetorgloc(const char *,char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;


/* local structures */


/* forward references */

static int worker(int,int,int) ;
static int conswrite(int,int,int,const char *,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_overlast
} ;


/* exported subroutines */


int opensvc_conslog(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	BITS		pargs ;
	KEYOPT		akopts ;
	const int	am = (of & O_ACCMODE) ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		pipes[2] ;
	int		fac = LOG_LOCAL7 ;
	int		pri = LOG_INFO ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*sn = "conslog" ;
	const char	*facspec = NULL ;
	const char	*prispec = NULL ;
	const char	*cp ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* program-root */
	                    case 'R':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
			    } else
	                        rs = SR_INVALID ;
	                        break ;

	                    case 'o':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
			    } else
	                        rs = SR_INVALID ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0) goto badarg ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_conslog: ai_pos=%u ai_max=%u\n",
		ai_pos,ai_max) ;
#endif

/* check arguments */

	if ((am == O_RDONLY) || (am == O_RDWR)) {
	    rs = SR_BADF ;
	    goto badarg ;
	}

/* initialization */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
		cp = argv[ai] ;
	        switch (pan) {
		case 0:
		    facspec = cp ;
	            break ;
		case 1:
		    prispec = cp ;
	            break ;
		} /* end switch */
		pan += 1 ;
		if (pan >= 2) break ;
	    }
	} /* end for */

/* optional facility and priority */

	if ((rs >= 0) && ((facspec != NULL) && (facspec[0] != '\0'))) {
	    rs = getlogfac(facspec,-1) ;
	    fac = rs ;
	} /* end if */

	if ((rs >= 0) && ((prispec != NULL) && (prispec[0] != '\0'))) {
	    rs = cfdeci(prispec,-1,&pri) ;
	}

	pri &= LOG_PRIMASK ;		/* truncate any garbage */

/* write it out */

	if (rs >= 0) {
	if ((rs = u_pipe(pipes)) >= 0) {
	    const int	rfd = pipes[0] ; /* read end */
	    fd = pipes[1] ; /* write end */

	    if ((rs = uc_fork()) == 0) worker(rfd,fac,pri) ;

	    u_close(rfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if (u_pipe) */
	} /* end if (ok) */

badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_conslog: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_conslog) */


/* local subroutines */


static int worker(int rfd,int fac,int pri)
{
	FILEBUF		b ;
	const mode_t	om = 0666 ;
	const int	of = O_WRONLY ;
	int		rs ;
	int		i ;
	int		ex = EX_OK ;

	for (i = 0 ; i < NOFILE ; i += 1) {
	    if (i != rfd) u_close(i) ;
	}

	if ((rs = u_open(LOGDEV,of,om)) >= 0) {
	    const int	fd = rs ;

	    if ((rs = filebuf_start(&b,rfd,0L,0,0)) >= 0) {
	   	const int	llen = LINEBUFLEN ;
		char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
		    int	ll = rs ;

		    if (lbuf[ll-1] == '\n') ll -= 1 ;
		    if (ll > LOG_MAXPS) ll = LOG_MAXPS ;

		    rs = conswrite(fd,fac,pri,lbuf,ll) ;
		    if (rs < 0) break ;

		} /* end while (reading lines) */

		filebuf_finish(&b) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} /* end if (open) */

	if (rs < 0) ex = EX_DATAERR ;

	return u_exit(ex) ;
}
/* end subroutine (worker) */


static int conswrite(fd,logfac,logpri,bp,bl)
int		fd ;
int		logfac ;
int		logpri ;
const char	bp[] ;
int		bl ;
{
	struct strbuf	cmsg, dmsg ;
	struct log_ctl	lc ;
	int		rs ;

	memset(&lc,0,sizeof(struct log_ctl)) ;
	lc.flags = SL_CONSOLE ;
	lc.level = 0 ;
	lc.pri = (logfac | logpri) ;

	/* set up the strbufs */
	cmsg.maxlen = sizeof(struct log_ctl) ;
	cmsg.len = sizeof(struct log_ctl) ;
	cmsg.buf = (caddr_t) &lc ;

	dmsg.maxlen = (bl+1) ;
	dmsg.len = bl ;
	dmsg.buf = (char *) bp ;

	/* output the message to the local logger */
	rs = u_putmsg(fd,&cmsg,&dmsg,0) ;

	return rs ;
}
/* end subroutine (conswrite) */


