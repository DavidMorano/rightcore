/* b_mailforward */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a MSU program
	out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally !  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child processes to exit explicitly.  Child processes are only
	created when run in "daemon" mode.

	Synopsis:

	$ mailforward <addr(s)>


	Implementation note:

	It is difficult to close files when run as a SHELL builtin!  We want to
	close files when we run in the background, but when running as a SHELL
	builtin, we cannot close file descriptors untill after we fork (else we
	corrupt the enclosing SHELL).  However, we want to keep the files
	associated with persistent objects open across the fork.  This problem
	is under review.  Currently, there is not an adequate self-contained
	solution.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecint.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<prsetfname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_mailforward.h"
#include	"defs.h"


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif	/* VBUFLEN */

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif /* EBUFLEN */

#define	CONFIG		struct config

#define	PROGSTATE	struct progstate
#define	PROGSTATE_FL	struct progstate_flags

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	NDF		"/tmp/mailforward.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_loga(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct progstate_flags {
	uint		nums:1 ;
	uint		emas:1 ;
} ;

struct progstate {
	PROGINFO	*pip ;
	PROGSTATE_FL	open ;
	VECSTR		emas ;
	VECINT		nums ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	progstate_start(PROGSTATE *,PROGINFO *) ;
static int	progstate_nums(PROGSTATE *,const char *,int) ;
static int	progstate_ema(PROGSTATE *,const char *,int) ;
static int	progstate_finish(PROGSTATE *) ;

static int	procmailbox(PROGINFO *,PROGSTATE *) ;

static int	config_start(CONFIG *,PROGINFO *,const char *) ;
static int	config_check(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_finish(CONFIG *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"LOGFILE",
	"md",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_logfile,
	argopt_md,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"cmd",
	"maildir",
	"loglen",
	"pollint",
	"intlock",
	"logfile",
	NULL
} ;

enum params {
	param_cmd,
	param_maildir,
	param_loglen,
	param_pollint,
	param_intlock,
	param_logfile,
	param_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const MAPEX	mapexs[] = {
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
	{ 0, 0 }
} ;


/* exported subroutines */


int b_mailforward(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_mailforward) */


int p_mailforward(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mailforward) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	PROGSTATE	ps, *psp = &ps ;
	struct ustat	sb ;
	struct ustat	*sbp = (struct ustat *) &sb ;
	CONFIG	co ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;
	USERINFO	u ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		n, i, j ;
	int		size ;
	int		sl, cl, ml ;
	int		opts ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_remove = FALSE ;
	int		f_all = FALSE ;
	int		f_list = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cfname = NULL ;
	const char	*maildname = NULL ;
	const char	*mailuser = NULL ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARNDF)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_mailforward: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->loglen = -1 ;
	pip->intrun = -1 ;
	pip->intmark = -1 ;
	pip->intlock = -1 ;
	pip->intspeed = -1 ;
	pip->intpoll = -1 ;
	pip->disint = -1 ;

	pip->f.quiet = FALSE ;
	pip->f.daemon = FALSE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&kopts) ;
	pip->open.kopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = progstate_start(&ps,pip) ;
	}

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
		const int	ach = MKCHAR(argp[1]) ;

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

/* do we have a keyword match or should we assume only key letters ? */

	            if ((kwi = matstr2(argopts,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_logfile:
	                    pip->have.log = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
				    pip->final.logprog = TRUE ;
	                            strwcpy(pip->lfname,avp,
	                                MIN(avl,MAXNAMELEN)) ;
				}
	                    }
	                    break ;

/* mail directory */
	                case argopt_md:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            maildname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            maildname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
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

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file-name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
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

	                    case 'C':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    pip->have.cfname = TRUE ;
				    pip->final.cfname = TRUE ;
	                            cfname = argp ;
				}
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        f_all = TRUE ;
	                        break ;

/* daemon mode */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					const int	ch = MKCHAR(avp[0]) ;
	                		if (isdigitlatin(ach)) {
					    pip->final.intrun = TRUE ;
	                                    rs = cfdecti(avp,avl,&v) ;
	                                    pip->intrun = v ;
	                                } else if (tolower(*avp) == 'i') {
	                                    pip->intrun = INT_MAX ;
	                                } else
	                                    rs = SR_INVALID ;
	                            }
	                        }
	                        break ;

	                    case 'l':
	                        f_list = TRUE ;
	                        break ;

/* numbers */
	                        case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = progstate_nums(&ps,argp,argl) ;
				} else
	                            rs = SR_INVALID ;
	                            break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* options */
	                        case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
				} else
	                            rs = SR_INVALID ;
	                            break ;

	                    case 'r':
	                        f_remove = TRUE ;
	                        break ;

/* alternate user */
	                        case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                mailuser = argp ;
				} else
	                            rs = SR_INVALID ;
	                            break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: finished parsing arguments\n") ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp, "%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp, "%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	} /* end if */

	if (f_help || f_usage || f_version)
	    goto retearly ;


	ex = EX_OK ;

/* cotinue intialization */

	pip->daytime = time(NULL) ;

	rs = userinfo_start(&u,NULL) ;
	if (rs < 0) {
		ex = EX_NOUSER ;
		shio_printf(pip->efp,
			"%s: could not get user information (%d)\n",
			pip->progname,rs) ;
		goto baduserinfo ;
	}

	pip->pid = u.pid ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;

/* argument defaults */

	if ((rs >= 0) && (pip->intpoll == 0) && (argvalue > 0)) {
	    pip->intpoll = argvalue ;
	}

/* set finals */

	if (maildname != NULL)
		pip->final.mailforward_maildir = TRUE ;

	if (cfname != NULL)
		pip->final.cfname = TRUE ;

	if (pip->intpoll >= 0)
	    pip->final.mailforward_pollint = TRUE ;

/* find and open a configuration file (if there is one) */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (cfname == NULL) cfname = getourenv(pip->envv,VARCFNAME) ;
	if (cfname == NULL) cfname = CONFIGFNAME ;

	if ((rs1 = config_start(&co,pip,cfname)) >= 0) {
	    pip->config = &co ;
	    pip->have.config = TRUE ;
	    pip->open.config = TRUE ;
	}

/* find anything that we don't already have */

	if (pip->intrun < 1)
	    pip->intrun = RUNINT ;

	if (pip->intpoll < 0)
	    pip->intpoll = POLLINT ;

	if (pip->intmark < 0)
	    pip->intmark = MARKINT ;

	if (pip->intlock < 0)
	    pip->intlock = TO_MAILLOCK ;

	if (pip->intspeed < 0)
	    pip->intspeed = TO_SPEED ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailforward: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.log) ;
#endif

/* log ID */

	v = (int) pip->pid ;
	mklogid(pip->logid,LOGIDLEN,pip->nodename,5,v) ;

/* log file */

	if (pip->have.logfname) {

	    if (pip->lfname[0] == '\0')
	        mkpath3(pip->lfname,pip->pr,LOGDNAME,LOGFNAME) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_mailforward: logfname=%s logid=%s\n",
	            pip->lfname,pip->logid) ;
#endif

	    rs1 = logfile_open(&pip->lh,pip->lfname,0,0666,pip->logid) ;

	    if (rs1 >= 0) {
	        pip->open.logprog = TRUE ;

	        if (pip->loglen > 0)
	            logfile_checksize(&pip->lh,pip->loglen) ;

	        logfile_userinfo(&pip->lh,&u,
	            pip->daytime,pip->progname,pip->version) ;

	    }

	} /* end if (have.logprog) */

/* figure out a mail spool directory */

	if (pip->maildname == NULL) {

		pip->have.mailforward_maildir = TRUE ;
		proginfo_setentry(pip,&pip->maildname,MAILDNAME,-1) ;

	}

/* get the mailuser */

	if (mailuser == NULL)
		mailuser = u.username ;

/* perform the function */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
		cp = argv[ai] ;
		pan += 1 ;
		rs = progstate_ema(&ps,cp,-1) ;
	    }

	    if (rs < 0) break ;
	} /* end for (looping through positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	            	    pan += 1 ;
	            	    rs = progstate_ema(&ps,cp,-1) ;
			}
		    }

		    if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: inaccessible argument-list (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: \trs=%d afile=%s\n",
	                pip->progname,rs,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */


/* HERE */

	rs = procmailbox(pip,psp) ;



/* DONE */

/* we are done */

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    logfile_close(&pip->lh) ;
	}

	if (pip->open.config) {
	    pip->config = NULL ;
	    config_finish(&co) ;
	    pip->config = NULL ;
	}

	userinfo_finish(&u) ;

baduserinfo:
/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
		}
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	   shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	            pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	progstate_finish(&ps) ;

	if (pip->open.kopts) {
	    pip->open.kopts = FALSE ;
	    keyopt_finish(&kopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_mailforward: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-speed[=name]] [-msfile file]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the mailbox */
static int procmailbox(PROGSTATE *psp,PROGINFO *pip)
{
	int		rs = SR_OK ;



	return rs ;
}
/* end subroutine (procmailbox) */


/* program state */
static int progstate_start(PROGSTATE *psp,PROGINFO *pip)
{
	int		rs ;
	int		opts ;

	psp->pip = pip ;
	opts = 0 ;
	rs = vecint_start(&psp->nums,10,opts) ;
	psp->open.nums = (rs >= 0) ;
	if (rs < 0)
		goto bad0 ;

	rs = vecstr_start(&psp->emas,10,opts) ;
	psp->open.emas = (rs >= 0) ;
	if (rs < 0)
		goto bad1 ;

ret0:
	return rs ;

bad1:
	vecint_finish(&psp->nums) ;

bad0:
	goto ret0 ;
}
/* end subroutine (progstate_start) */


static int progstate_finish(PROGSTATE *psp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (psp->open.emas) {
	    psp->open.emas = FALSE ;
	    rs1 = vecstr_finish(&psp->emas) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (psp->open.nums) {
	    psp->open.nums = FALSE ;
	    rs1 = vecint_finish(&psp->nums) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (progstate_finish) */


/* get numbers */
static int progstate_nums(psp,ap,al)
PROGSTATE	*psp ;
const char	*ap ;
int		al ;
{
	int		rs = SR_OK ;
	int		cl ;
	int		v ;
	int		c = 0 ;
	const char	*tp ;
	const char	*cp ;

	if (al < 0)
		al = strlen(ap) ;

	while ((rs >= 0) && al && ((tp = strnpbrk(ap,al," \t,")) != NULL)) {

		cp = (char *) ap ;
		cl = (tp - ap) ;
		if (cl > 0) {

		    c += 1 ;
		    rs = cfdecui(cp,cl,&v) ;

		    if (rs >= 0)
			rs = vecint_add(&psp->nums,v) ;

		}

		al = ((ap + al) - (tp + 1)) ;
		ap = (tp + 1) ;

	} /* end while */

	if ((rs >= 0) && (al > 0)) {
	    c += 1 ;
	    if ((rs = cfdecui(ap,al,&v)) >= 0) {
		rs = vecint_add(&psp->nums,v) ;
	    }
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progstate_nums) */


/* store EMAs */
static int progstate_ema(PROGSTATE *psp,cchar *ap,int al)
{
	int		rs ;

	if (al < 0) al = strlen(ap) ;

	rs = vecstr_add(&psp->emas,ap,al) ;

	return rs ;
}
/* end subroutine (progstate_ema) */


/* configuration maintenance */
static int config_start(CONFIG *op,PROGINFO *pip,cchar *cfname)
{
	VECSTR		sv ;
	int		rs = SR_OK ;
	char		tmpfname[MAXPATHLEN + 1] ;

	memset(op,0,sizeof(CONFIG)) ;

	op->pip = pip ;
	tmpfname[0] = '\0' ;
	if (strchr(cfname,'/') == NULL) {

	    if ((rs = vecstr_start(&sv,6,0)) >= 0) {

	        vecstr_envset(&sv,"p",pip->pr,-1) ;

	        vecstr_envset(&sv,"e","etc",-1) ;

	        vecstr_envset(&sv,"n",pip->searchname,-1) ;

	        rs = permsched(sched1,&sv,
	            tmpfname,MAXPATHLEN, cfname,R_OK) ;

	        if (rs == 0)
	            rs = mkpath1(tmpfname,cfname) ;

	        vecstr_finish(&sv) ;
	    } /* end if (finding file) */

	} else
	    mkpath1(tmpfname,cfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: rs=%d cfname=%s\n",rs,tmpfname) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {

	    shio_printf(pip->efp,"%s: conf=%s\n",
	        pip->progname,tmpfname) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: 2\n") ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_start: cfname=%s\n",tmpfname) ;
#endif

	    rs = paramfile_open(&op->p,(const char **) pip->envv,tmpfname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_start: paramfile_open() rs=%d\n",rs) ;
#endif

	}

	if (rs >= 0) {

	    rs = expcook_start(&op->cooks) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_start: expcook_start() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad1 ;

	    expcook_add(&op->cooks,"P",pip->progname ,-1) ;

	    expcook_add(&op->cooks,"S",pip->searchname,-1) ;

	    expcook_add(&op->cooks,"N",pip->nodename,-1) ;

	    expcook_add(&op->cooks,"D",pip->domainname,-1) ;

	    snsds(tmpfname,MAXPATHLEN,pip->nodename,pip->domainname) ;

	    expcook_add(&op->cooks,"H",tmpfname,-1) ;

	    expcook_add(&op->cooks,"R",pip->pr,-1) ;

	    expcook_add(&op->cooks,"U",pip->username,-1) ;

	    op->f_p = TRUE ;
	    rs = config_read(op) ;

	    op->f_p = (rs >= 0) ;
	    if (rs < 0)
	        goto bad2 ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d \n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	expcook_finish(&op->cooks) ;

bad1:
	paramfile_close(&op->p) ;

bad0:
	goto ret0 ;
}
/* end subroutine (config_start) */


static int config_check(op)
CONFIG	*op ;
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_NOTOPEN ;

	if (op->f_p) {
	    if ((rs = paramfile_check(&op->p,pip->daytime)) > 0)
	        rs = config_read(op) ;
	}

	return rs ;
}
/* end subroutine (config_check) */


static int config_finish(op)
CONFIG	*op ;
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_NOTOPEN ;

	if (op->f_p) {
	    expcook_finish(&op->cooks) ;
	    rs = paramfile_close(&op->p) ;
	}

	return rs ;
}
/* end subroutine (config_finish) */


static int config_read(op)
CONFIG	*op ;
{
	PROGINFO	*pip = op->pip ;
	struc locinfo	*lip ;
	PARAMFILE_CUR	cur ;
	int		rs = SR_NOTOPEN ;
	int		rs1 ;
	int		i ;
	int		ml, vl, el ;
	int		v ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f_p) ;
#endif

	lip = pip->lip ;
	if (! op->f_p)
	    goto ret0 ;

	    rs = SR_OK ;
	    for (i = 0 ; params[i] != NULL ; i += 1) {
		cchar	*pr = pip->pr ;

	        rs1 = FALSE ;
	        switch (i) {

	        case param_pollint:
	            if (pip->f.pollint)
	                rs1 = TRUE ;
	            break ;

	        } /* end switch */

	        if (rs1)
	            continue ;

	        if ((rs = paramfile_curbegin(&op->p,&cur)) >= 0) {
		    const int	vlen = VBUFLEN ;

	        while (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: checking for param=%s\n",
	                    params[i]) ;
#endif

	            vl = paramfile_fetch(&op->p,params[i],&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: vbuf=>%t<\n",vbuf,vl) ;
#endif

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {

	                el = expcook_exp(&op->cooks,0,
				ebuf,EBUFLEN,vbuf,vl) ;

	                if (el >= 0)
	                    ebuf[el] = '\0' ;

	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	            if (el > 0) {
			char	tmpfname[MAXPATHLEN + 1] ;

	                switch (i) {

	                case param_loglen:
	                case param_pollint:
	                case param_intlock:
	                    rs1 = cfdecti(ebuf,el,&v) ;

	                    if ((rs1 >= 0) && (v >= 0)) {

	                        switch (i) {

	                        case param_loglen:
	                            pip->loglen = v ;
	                            break ;

	                        case param_pollint:
				    if (! pip->final.pollint)
	                                pip->intpoll = v ;
	                            break ;

	                        case param_intlock:
				    if (! pip->final.intlock)
	                                pip->intlock = v ;
	                            break ;

	                        } /* end switch */

	                    } /* end if (valid number) */
	                    break ;

	                case param_maildir:
			    if (! pip->final.mailforward_maildir) {
				pip->have.mailforward_maildir = TRUE ;
	                        prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                            NULL,MAILDNAME,"") ;
				if ((pip->maildname == NULL) ||
				    (strcmp(pip->maildname,tmpfname) != 0)) {
				    pip->change.mailforward_maildir = TRUE ;
				    proginfo_setentry(pip,&pip->maildname,
					tmpfname,-1) ;
				}
			    }
	                    break ;

	                case param_logfile:
			    if (! pip->final.logfname) {
				pip->have.logfname = TRUE ;
	                        setfname(pip,tmpfname,ebuf,el,TRUE,
	                            LOGDNAME,pip->searchname,"") ;
				if (strcmp(pip->lfname,tmpfname) != 0) {
				    pip->change.logfname = TRUE ;
				    mkpath1(pip->lfname,tmpfname) ;
				}
			    }
	                    break ;

	                case param_cmd:
	                    ml = MIN(LOGIDLEN,el) ;
	                    if (ml && (pip->cmd[0] == '\0'))
	                        strwcpy(pip->cmd,ebuf,ml) ;
	                    break ;

	                } /* end switch */

	            } /* end if (got one) */

	        } /* end while (fetching) */

	        paramfile_curend(&op->p,&cur) ;
	    } /* end if (parameters) */

	    if (rs < 0) break ;
	} /* end for */

ret0:
	return rs ;
}
/* end subroutine (config_read) */


