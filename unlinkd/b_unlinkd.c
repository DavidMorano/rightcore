/* b_unlinkd */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_NEWLOGID	1		/* use 'mklogid(3dam)' */


/* revision history:

	= 2005-04-20, David A­D­ Morano
	I changed the program so that the configuration file is consulted even
	if the program is not run in daemon-mode.  Previously, the
	configuration file was only consulted when run in daemon-mode.  The
	thinking was that running the program in regular (non-daemon) mode
	should be quick.  The problem is that the MS file had to be guessed
	without the aid of consulting the configuration file.  Although not a
	problem in most practice, it was not aesthetically appealing.  It meant
	that if the administrator changed the MS file in the configuration
	file, it also had to be changed by specifying it explicitly at
	invocation in non-daemon-mode of the program.  This is the source of
	some confusion (which the world really doesn't need).  So now the
	configuration is always consulted.  The single one-time invocation is
	still fast enough for the non-smoker aged under 40!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a MSU program
	out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child processes to exit explicitly.  Child processes are only
	created when run in "daemon" mode.

	Synopsis:

	$ unlinkd 


	Implementation note:

	It is difficult to close files when run as a SHELL builtin!  We want to
	close files when we run in the background, but when running as a SHELL
	builtin, we cannot close file descriptors untill after we fork (else we
	corrupt the enclosing SHELL).  However, we want to keep the files
	associated with persistent objects open across the fork.  This problem
	is under review.  Currently, there is not an adequate self-contained
	solution.


*******************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

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
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<msfile.h>
#include	<kinfo.h>
#include	<lfm.h>
#include	<getutmpent.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_unlinkd.h"
#include	"defs.h"
#include	"msflag.h"


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	INTPOLLMULT
#define	INTPOLLMULT	1000
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,cchar *,cchar *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getserial(const char *) ;
extern int	getutmpterm(char *,int,pid_t) ;
static int	prsetfname(cchar *,char *,cchar *,int,int,
			cchar *,cchar *,cchar *) ;
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
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_loga(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		lockinfo : 1 ;
	uint		fg:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, final ;
	PROGINFO	*pip ;
} ;

struct session {
	pid_t		sid ;
	char		termdev[MAXPATHLEN + 1] ;
} ;

struct config_flags {
	uint		p:1 ;
	uint		lockinfo:1 ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	struct config_flags	f ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	msupdate(PROGINFO *,LFM *) ;
static int	logstart(PROGINFO *) ;
static int	logmark(PROGINFO *,long) ;

static int	printlockcheck(PROGINFO *,LFM_CHECK *) ;

static int	loginfo(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

static int	config_start(struct config *,PROGINFO *,const char *) ;
static int	config_check(struct config *) ;
static int	config_read(struct config *) ;
static int	config_finish(struct config *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"sn",
	"af",
	"ef",
	"fg",
	"db",
	"msfile",
	"mspoll",
	"caf",
	"disable",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_fg,
	argopt_db,
	argopt_msfile,
	argopt_mspoll,
	argopt_caf,
	argopt_disable,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOMEM, EX_OSERR },
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_ALREADY, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"lockinfo",
	NULL
} ;

enum progopts {
	progopt_lockinfo,
	progopt_overlast
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
	"logsize",
	"msfile",
	"pidfile",
	"runtime",
	"intpoll",
	"markint",
	"lockint",
	"speedint",
	"logfile",
	NULL
} ;

enum params {
	param_cmd,
	param_logsize,
	param_msfile,
	param_pidfile,
	param_intrun,
	param_intpoll,
	param_markint,
	param_lockint,
	param_speedint,
	param_logfile,
	param_overlast
} ;


/* exported subroutines */


int b_unlinkd(int argc,cchar *argv[],void *contextp)
{
	int	rs ;
	int	rs1 ;
	int	ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_unlinkd) */


int p_unlinkd(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_unlinkd) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	struct session	sinfo ;
	struct ustat	sb, *sbp = (struct ustat *) &sb ;
	struct config	co ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

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
	int		cl, ml ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_child = FALSE ;
	int		f_caf = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*cfname = NULL ;
	const char	*msfname = NULL ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_unlinkd: starting DFD=%d\n",rs) ;
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
	pip->intrun = -1 ;
	pip->intmark = -1 ;
	pip->intlock = -1 ;
	pip->intspeed = -1 ;
	pip->intpoll = -1 ;
	pip->intdis = -1 ;

	if (rs >= 0) {
	    pip->lip = lip ;
	    rs = locinfo_start(lip,pip) ;
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	memset(&sinfo,0,sizeof(struct session)) ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

	            argval = (argp + 1) ;

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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
	                    pip->have.logfname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->have.logfname = TRUE ;
	                            pip->final.logfname = TRUE ;
	                            pip->lfname = avp ;
	                        }
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

/* argument-list file */
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

/* foregroup-mode */
	                case argopt_fg:
			    lip->have.fg = TRUE ;
			    lip->final.fg = TRUE ;
			    lip->f.fg = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
				    rs = optbool(avp,avl) ;
	                            lip->f.fg = (rs > 0) ;
	                        }
	                    }
			    break ;

/* MS poll interval */
	                case argopt_mspoll:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.intpoll = TRUE ;
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intpoll = v ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.intpoll = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->intpoll = v ;
	                        }
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_caf:
	                    f_caf = TRUE ;
	                    break ;

/* disable interval */
	                case argopt_disable:
	                    pip->f.disable = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.intdis = TRUE ;
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intdis = v ;
	                        }
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* daemon mode */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					const int	ch = MKCHAR(acp[0]) ;
	                		if (isdigitlatin(ch)) {
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

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_unlinkd: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

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
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
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
	}

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* process program options */

	if ((rs >= 0) && (pip->intpoll == 0) && (argvalue > 0)) {
	    pip->intpoll = argvalue ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->intrun >= 0)
	    pip->f.intrun = TRUE ;

	if (pip->intpoll >= 0)
	    pip->f.intpoll = TRUE ;

/* continue with prepatory initialization */

	pip->daytime = time(NULL) ;

	pip->pid = ugetpid() ;

	if ((rs >= 0) && 
	    ((pip->nodename == NULL) || (pip->nodename[0] == '\0'))) {
	    char	nodename[MAXHOSTNAMELEN + 1] ;
	    char	domainname[MAXNAMELEN + 1] ;
	    rs = getnodedomain(nodename,domainname) ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->nodename,nodename,-1) ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->domainname,domainname,-1) ;
	} /* end if */

	if (rs >= 0) {
	    char	username[USERNAMELEN + 1] ;
	    getusername(username,USERNAMELEN,-1) ;
	    proginfo_setentry(pip,&pip->username,username,-1) ;
	}

	pip->name = getourenv(env,VARNAME) ;

	if (rs < 0) goto badarg ;

/* find and open a configuration file (if there is one) */

	if (cfname == NULL) cfname = getourenv(pip->envv,VARCFNAME) ;
	if (cfname == NULL) cfname = CONFIGFNAME ;

	if ((rs1 = config_start(&co,pip,cfname)) >= 0) {
	    pip->config = &co ;
	    pip->have.config = TRUE ;
	    pip->open.config = TRUE ;
	}

/* hack */

	co.f.lockinfo = lip->f.lockinfo ;

/* find anything that we don't already have */

	if ((rs >= 0) && pip->f.daemon && (pip->pidfname[0] == '\0')) {
	    char	cname[MAXNAMELEN+1] ;
	    snsds(cname,MAXNAMELEN,pip->nodename,PIDFNAME) ;
	    mkpath3(tmpfname,pip->pr,RUNDNAME,cname) ;
	    rs = proginfo_setentry(pip,&pip->pidfname,tmpfname,-1) ;
	}

	if (pip->intrun < 1)
	    pip->intrun = INTRUN ;

	if (pip->intpoll < 0)
	    pip->intpoll = INTPOLL ;

	if (pip->intmark < 0)
	    pip->intmark = INTMARK ;

	if (pip->intlock < 0)
	    pip->intlock = TO_LOCK ;

	if (pip->intspeed < 0)
	    pip->intspeed = TO_SPEED ;

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLOGFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_unlinkd: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.logfname) ;
#endif

/* log ID */

	if (pip->f.daemon) {
	    char	logidbuf[LOGIDLEN+1] ;

	    mkpath3(tmpfname,pip->pr,VARDNAME,SERIALFNAME) ;

	    rs = getserial(tmpfname) ;

	    if (rs < 0) {

	        mkpath2(tmpfname,pip->tmpdname,SERIALFNAME) ;

	        rs = getserial(tmpfname) ;

	    }

	    v = (rs >= 0) ? rs : ((int) pip->pid) ;

#if	CF_NEWLOGID
	    mklogid(logidbuf,LOGIDLEN,pip->nodename,-1,v) ;
#else
	    snsd(logidbuf,LOGIDLEN,pip->nodename,v) ;
#endif

	    rs = proginfo_setentry(pip,&pip->logid,logidbuf,-1) ;

	} /* end if (daemon) */

/* logging is normally only for daemon mode */

	if ((rs >= 0) && (pip->f.daemon || pip->have.logfname)) {

/* log file */

	    if (pip->lfname[0] == '\0')
	        mkpath3(pip->lfname,pip->pr,LOGDNAME,LOGFNAME) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_unlinkd: logfname=%s logid=%s\n",
	            pip->lfname,pip->logid) ;
#endif

	    rs1 = logfile_open(&pip->lh,pip->lfname,0,0666,pip->logid) ;

	    if (rs1 >= 0) {
	        pip->open.logprog = TRUE ;
		logstart(pip) ;
	    } /* end if (logging startup) */

	} /* end if (daemon mode) */

/* can we open the MS file? */

	    if ((rs >= 0) && pip->f.daemon) {
	        LFM	pidlock ;
	        pid_t	pid = 0 ;
	        int	cs ;

		if (! lip->f.fg) {
	            if (pip->open.logprog) logfile_flush(&pip->lh) ;
		    if (pip->efp != NULL) shio_flush(pip->efp) ;
	            rs = uc_fork() ;
	            pid = rs ;
		    if ((rs >= 0) && (pid == 0)) f_child = TRUE ;
	        }

		if ((rs >= 0) && (pid == 0)) { /* child */

	            sinfo.sid = getsid(0) ;

	            rs1 = getutmpterm(sinfo.termdev,MAXPATHLEN,sinfo.sid) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_unlinkd: getutmpline() "
	                    "rs=%d termdev=%s\n",
	                    rs1,sinfo.termdev) ;
#endif

	            if (rs1 < 0)
	                mkpath1(sinfo.termdev,DEVTTY) ;

#ifdef	COMMENT
			if (pip->efp != NULL) {
			    pip->open.errfile = FALSE ;
	            	    shio_close(pip->efp) ;
	            	    memset(pip->efp,0,sizeof(SHIO)) ;
	                    pip->efp = NULL ;
			}
#endif


	            if (f_caf) {

			if ((pip->efp != NULL) && pip->open.errfile) {
			    pip->open.errfile = FALSE ;
	            	    shio_close(pip->efp) ;
	            	    memset(pip->efp,0,sizeof(SHIO)) ;
	                    pip->efp = NULL ;
			}

	                if (pip->open.logprog) {
	                    pip->open.logprog = FALSE ;
	                    logfile_close(&pip->lh) ;
			}

	                if (pip->open.config) {
	                    pip->open.config = FALSE ;
	                    config_finish(&co) ;
			}

#if	CF_DEBUGS || CF_DEBUG
#else
	                for (i = 0 ; i < NOFILE ; i += 1)
	                    u_close(i) ;
#endif

#if	CF_DEBUGS || CF_DEBUG
	                if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	                    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	                        cp = getourenv(envv,VARDEBUGFD2) ;
	                }
	                if (cp != NULL)
	                    debugopen(cp) ;
#endif /* CF_DEBUGS */

	                if (pip->have.config) {
	                    rs1 = config_start(&co,pip,cfname) ;
	                    pip->open.config = (rs1 >= 0) ;
	                }

	                if ((! pip->open.logprog) && 
	                    (pip->lfname[0] != '\0')) {

			    {
				const char	*lf = pip->lfname ;
				const char	*lid = pip->logid ;
	                        rs1 = logfile_open(&pip->lh,lf,0,0666,lid) ;
	                        pip->open.logprog = (rs1 >= 0) ;
			    }

	                }

	            } else {

	                if (sinfo.termdev[0] != '\0') {

	                    rs1 = shio_open(pip->efp,sinfo.termdev,"w",0666) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("b_unlinkd: shio_open() rs=%d "
	                            "termdev=%s\n",
	                            rs1,sinfo.termdev) ;
#endif

	                }

	                if ((sinfo.termdev[0] == '\0') || (rs1 < 0))
	                    pip->efp = NULL ;

	            } /* end if (close-all-files) */

		    if ((rs >= 0) && (! lip->f.fg)) {
			if (pip->pid != sinfo.sid) rs = u_setsid() ;
		    }

	            if (rs >= 0) {
	                const char	*ccp ;

	                pip->pid = ugetpid() ;

	                if (pip->debuglevel > 0) {

	                    shio_printf(pip->efp,"%s: pidlock=%s\n",
	                        pip->progname,
	                        pip->pidfname) ;

	                    shio_printf(pip->efp,"%s: intpoll=%u\n",
	                        pip->progname,
	                        pip->intpoll) ;

	                    shio_printf(pip->efp,"%s: intrun=%u\n",
	                        pip->progname,
	                        pip->intrun) ;

	                    shio_flush(pip->efp) ;

	                } /* end if (debugging information) */

			if (rs >= 0)
			    rs = loginfo(pip) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_unlinkd: pidname=%s\n",
				pip->pidfname) ;
#endif

	                ccp = lip->pidfname ;
	                if ((rs >= 0) && (ccp != NULL) && 
	                    (ccp[0] != '\0') && (ccp[0] != '-')) {

	                    LFM_CHECK	lc ;

	                    cl = sfdirname(pip->pidfname,-1,&cp) ;

	                    rs = mkpath1w(tmpfname,cp,cl) ;

	                    if ((rs >= 0) && (u_stat(tmpfname,&usb) < 0))
	                        mkdirs(tmpfname,TMPDMODE) ;

	                    pip->have.pidfname = TRUE ;
	                    rs = lfm_start(&pidlock,pip->pidfname,
	                        LFM_TRECORD, pip->intlock,&lc,
	                        pip->nodename,pip->username,pip->banner) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_unlinkd: lfm_start() rs=%d\n",
					rs) ;
#endif

	                    if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN))
	                        printlockcheck(pip,&lc) ;

	                } /* end if (PID lock) */

	                if (rs >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_unlinkd: daemon msupdate()\n") ;
#endif

	                    rs = msupdate(pip,&pidlock) ;

	                    if (rs == SR_AGAIN)
	                        rs = SR_OK ;

	                    if (pip->have.pidfname) {
	                        pip->have.pidname = FALSE ;
	                        lfm_finish(&pidlock) ;
			    }

	                } /* end if (got lock) */

	            } /* end if */

		} /* end if (child) */

	    } else
	        rs = msupdate(pip,NULL) ;

done:
	if ((rs >= 0) && (pip->debuglevel > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_unlinkd: daemon=%u child=%u\n",
	            pip->f.daemon,f_child) ;
#endif

	    if ((! pip->f.daemon) || f_child) {

	        shio_printf(pip->efp, "%s: MS updates=%u\n",
	            pip->progname,rs) ;

	    }
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_ALREADY:
	    case SR_AGAIN:
		ex = EX_MUTEX ;
	        if ((! pip->f.quiet) && (pip->efp != NULL)) {
	            shio_printf(pip->efp,
	                "%s: existing lock (%d)\n",
	                pip->progname,rs) ;
		}
	        if (pip->open.logprog) {
	            logfile_printf(&pip->lh,
	                "existing lock (%d)",rs) ;
		}
	        break ;
	    default:
		ex = mapex(mapexs,rs) ;
	        if ((! pip->f.quiet) && (pip->efp != NULL)) {
	            shio_printf(pip->efp,
	                "%s: could not perform update (%d)\n",
	                pip->progname,rs) ;
		}
	        if (pip->open.logprog) {
	            logfile_printf(&pip->lh,
	                "could not perform update (%d)",rs) ;
		}
	        break ;
	    } /* end switch */
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

	if ((pip->open.logprog && (! pip->f.daemon)) || f_child) {
	    logfile_printf(&pip->lh,"exiting ex=%u (%d)",
	        ex,rs) ;
	}

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    logfile_close(&pip->lh) ;
	}

	if (pip->open.config) {
	    pip->config = NULL ;
	    config_finish(&co) ;
	}

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    if (pip->f.daemon) {
	        shio_printf(pip->efp,"%s: (%s) exiting ex=%u (%d)\n",
	            pip->progname,((f_child) ? "child" : "parent"),ex,rs) ;
	    } else {
	        shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	            pip->progname,ex,rs) ;
	    }
	} /* end if */

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_unlinkd: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS || CF_DEBUG
	if (! f_child)
	    debugclose() ;
#endif

/* if child => exit, needed since return doesn't lead to exit! */

	if (f_child)
	    uc_exit(ex) ;

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

	fmt = "%s: USAGE> %s [-mspoll <int>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n", if (rs >= 0)
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

		while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

		        switch (oi) {

		case progopt_lockinfo:
			if (! lip->final.lockinfo) {
			    c += 1 ;
	                    lip->have.lockinfo = TRUE ;
	                    lip->f.lockinfo = TRUE ;
	                    if (vl > 0) {
				rs = optbool(vp,vl) ;
	                        lip->f.lockinfo = (rs > 0) ;
			    }
			}
	                break ;

	            } /* end switch */

			c += 1 ;
	            } else
	                rs = SR_INVALID ;

		    if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
} 
/* end subroutine (procopts) */


static int msupdate(pip,lp)
PROGINFO	*pip ;
LFM		*lp ;
{
	struct config	*cop = (struct config *) pip->config ;

	struct pollfd	fds[2] ;

	MSFILE		ms ;
	MSFILE_ENT	e, etmp ;

	KINFO		ki ;

	KINFO_DATA	d ;

	time_t	ti_start ;
	time_t	ti_log ;

	long	lw ;

	uint	ppm ;
	uint	pagesize = getpagesize() ;

	int	rs, rs1, i, c ;
	int	nfds, oflags ;
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_unlinkd/msupdate: runtime=%s\n",
	        timestr_elapsed((time_t) pip->intrun,timebuf)) ;
#endif

	nfds = 0 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = 0 ;
	fds[nfds].revents = 0 ;

	oflags = O_RDWR ;
	rs = msfile_open(&ms,pip->msfname,oflags,0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_unlinkd/msupdate: msfile_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	pip->daytime = time(NULL) ;

	ti_log = pip->daytime ;
	ti_start = pip->daytime ;

	rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&e) ;

	if (rs1 < 0) {

	    memset(&e,0,sizeof(MSFILE_ENT)) ;

	    strwcpy(e.nodename,
	        pip->nodename,MSFILEE_LNODENAME) ;

	}

/* get some updated information */

	if (pip->f.disable) {
	    e.flags |= MSFLAG_MDISABLED ;
	    if (pip->intdis > 0)
	        e.dtime = pip->daytime + pip->intdis ;
	}

/* do some load-ave updates */

	c = 0 ;
	if ((rs = kinfo_open(&ki,pip->daytime)) >= 0) {

	    while (rs >= 0) {

	        lw = pip->intrun - (pip->daytime - ti_start) ;
	        if (lw <= 0) break ;

	        if (pip->f.daemon) {

	            if (pip->have.pidfname && (lp != NULL)) {
	                LFM_CHECK	ci ;

	                rs = lfm_check(lp,&ci,pip->daytime) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_unlinkd/msupdate: "
				"lfm_check() rs=%d\n",
	                        rs) ;
	                    if (rs < 0) {
	                        debugprintf("b_unlinkd/msupdate: pid=%d\n",
				    ci.pid) ;
	                        debugprintf("b_unlinkd/msupdate: node=%s\n",
	                            ci.nodename) ;
	                        debugprintf("b_unlinkd/msupdate: user=%s\n",
	                            ci.username) ;
	                        debugprintf("b_unlinkd/msupdate: banner=%s\n",
	                            ci.banner) ;
	                    }
	                }
#endif /* CF_DEBUG */

	                if (rs == SR_LOCKLOST)
	                    printlockcheck(pip,&ci) ;

	            } /* end if (had a lock file) */

#ifdef	COMMENT
	            if ((rs < 0) && (rs != SR_AGAIN))
	                break ;

	            if (rs == SR_AGAIN) {
	                rs = SR_OK ;
	                break ;
	            }
#else
	            if (rs < 0)
	                break ;
#endif /* COMMENT */

	        } /* end if (daemon mode) */

/* continue with the update */

	        rs = kinfo_sysmisc(&ki,pip->daytime,&d) ;

	        e.boottime = (uint) d.boottime ;
	        e.ncpu = d.ncpu ;
	        e.nproc = d.nproc ;

	        e.la[0] = d.la_1min ;
	        e.la[1] = d.la_5min ;
	        e.la[2] = d.la_15min ;

/* calculate pages-per-megabyte */

	        ppm = (1024 * 1024) / pagesize ;

/* OK, now calculate the megabytes of each type of memory */

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	        rs1 = uc_sysconf(_SC_PHYS_PAGES,&lw) ;

	        e.pmtotal = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmtotal = (lw / ppm) ;

	        rs1 = uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

	        e.pmavail = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmavail = (lw / ppm) ;
#else
	        e.pmavail = 1 ;
#endif /* defined(SOLARIS) */

/* finish off with time stamps */

	        e.atime = (uint) 0 ;
	        e.utime = (uint) pip->daytime ;

/* write-back the update */

	        rs = msfile_update(&ms,pip->daytime,&e) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_unlinkd/msupdate: msfile_update() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	        c += 1 ;

	        if (! pip->f.daemon)
	            break ;

/* sleep for daemon mode */

	        for (i = 0 ; (i < pip->intpoll) ; i += 1) {

	            rs1 = u_poll(fds,nfds,INTPOLLMULT) ;

	        } /* end for */

	        pip->daytime = time(NULL) ;

/* maintenance */

	        if ((c & 15) == 3)
	            kinfo_check(&ki,pip->daytime) ;

	        if (pip->open.config && ((c & 3) == 4))
	            config_check(cop) ;

	        if ((c & 15) == 5)
	            msfile_check(&ms,pip->daytime) ;

	        if (pip->open.logprog && ((c & 7) == 1)) {

	            if ((pip->daytime - ti_log) >= pip->intmark) {

	                ti_log = pip->daytime ;
	                lw = labs(pip->intrun - (pip->daytime - ti_start)) ;

			logmark(pip,lw) ;

	            }
	        }

/* periodically close and reopen the log file (precaution?) */

	        if ((c & 31) == 1) {

	            if (pip->open.logprog) {

	                pip->open.logprog = FALSE ;
	                logfile_close(&pip->lh) ;

	            }

	            if ((! pip->open.logprog) && (pip->lfname[0] != '\0')) {
			const char	*lf = pip->lfname ;
			const char	*lid = pip->logid ;
	                rs1 = logfile_open(&pip->lh,lf,0,0666,lid) ;
	                pip->open.logprog = (rs1 >= 0) ;
	            }

	        } /* end if (logging) */

/* handle the special "exit" condition */

	        if (pip->cmd[0] != '\0') {

	            if (strcmp(pip->cmd,"exit") == 0)
	                break ;

	        }

/* get a flesh copy of the entry */

	        rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&etmp) ;

	        if (rs1 >= 0)
	            e = etmp ;

/* check disabled state only in daemon mode */

	        if ((e.dtime != 0) && (pip->daytime >= e.dtime)) {

	            e.dtime = 0 ;
	            e.flags &= (~ MSFLAG_MDISABLED) ;

	        }

	    } /* end while (end time not reached) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("b_unlinkd/msupdate: start=%s\n",
	            timestr_log(ti_start,timebuf)) ;
	        debugprintf("b_unlinkd/msupdate: now=%s\n",
	            timestr_log(pip->daytime,timebuf)) ;
	    }
#endif

	    kinfo_close(&ki) ;
	} /* end if (opened kernel channel) */

	msfile_close(&ms) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_unlinkd/msupdate: ret rs=%d c=%u\n",rs,c) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msupdate) */


static int logstart(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    cchar	*fmt ;
	    cchar	*a = getourenv(pip->envv,VARARCHITECTURE) ;
	    cchar	*s = getourenv(pip->envv,VARSYSNAME) ;
	    cchar	*r = getourenv(pip->envv,VARRELEASE) ;
	    cchar	*n = pip->name ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	if (pip->logsize > 0)
	    logfile_checksize(&pip->lh,pip->logsize) ;

	logfile_printf(&pip->lh,"%s %-14s %s",
	    timestr_logz(pip->daytime,timebuf),
	    pip->progname,pip->version) ;

	if ((s != NULL) && (r != NULL)) {

	    if (a != NULL) {
	        rs = logfile_printf(&pip->lh, "a=%s os=%s(%s) d=%s",
	            a,s,r,
	            pip->domainname) ;
	    } else
	        rs = logfile_printf(&pip->lh, "os=%s(%s) d=%s",
	            s,r,
	            pip->domainname) ;

	} /* end if (OS system information) */

	fmt = (n != NULL) ? "%s!%s (%s)" : "%s!%s" ;
	if (rs >= 0) {
	    rs = logfile_printf(&pip->lh,fmt,
	        pip->nodename,pip->username,n) ;
	}

	} /* end if (enabled) */

	return rs ;
}
/* end subroutine (logstart) */


static logmark(PROGINFO *pip,long lw)
{
	int		rs = SR_OK ;
	const char	*n = pip->name ;
	const char	*fmt ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (! pip->open.logprog)
	    goto ret0 ;

	rs = logfile_printf(&pip->lh,
	    "%s mark> %s",
	    timestr_logz(pip->daytime,timebuf),
	    pip->nodename) ;

	fmt = (n != NULL) ? "%s!%s (%s)" : "%s!%s" ;
	if (rs >= 0)
	    rs = logfile_printf(&pip->lh,fmt,
	        pip->nodename,pip->username,n) ;

	if (rs >= 0)
	    rs = logfile_printf(&pip->lh,
	        "pid=%u remaining=%s",
	        (uint) pip->pid,
	        timestr_elapsed(lw,timebuf)) ;

	if (rs >= 0)
	    rs = logfile_flush(&pip->lh) ;

ret0:
	return rs ;
}
/* end subroutine (logmark) */


/* print out lock-check information */
static int printlockcheck(pip,lcp)
PROGINFO	*pip ;
LFM_CHECK	*lcp ;
{
	int	rs = SR_OK ;

	const char	*np ;

	char	timebuf[TIMEBUFLEN + 1] ;


	switch (lcp->stat) {

	case SR_AGAIN:
	    np = "busy" ;
	    break ;

	case SR_LOCKLOST:
	    np = "lost" ;
	    break ;

	default:
	    np = "unknown" ;
	    break ;

	} /* end switch */

	if (pip->open.logprog) {

	    logfile_printf(&pip->lh,
	        "%s lock %s\n",
	        timestr_logz(pip->daytime,timebuf),
	        np) ;

	    logfile_printf(&pip->lh,
	        "other_pid=%d\n",
	        lcp->pid) ;

	    if (lcp->nodename != NULL)
	        logfile_printf(&pip->lh,
	            "other_node=%s\n",
	            lcp->nodename) ;

	    if (lcp->username != NULL)
	        logfile_printf(&pip->lh,
	            "other_user=%s\n",
	            lcp->username) ;

	    if (lcp->banner != NULL)
	        logfile_printf(&pip->lh,
	            "other_banner=%s\n",
	            lcp->banner) ;

	} /* end if (log-open) */

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {

	    shio_printf(pip->efp,
	        "%s: %s lock %s\n",
	        pip->progname,
	        timestr_logz(pip->daytime,timebuf),
	        np) ;

	    shio_printf(pip->efp,
	        "%s: other_pid=%d\n",
	        pip->progname,lcp->pid) ;

	    if (lcp->nodename != NULL)
	        shio_printf(pip->efp,
	            "%s: other_node=%s\n",
	            pip->progname,lcp->nodename) ;

	    if (lcp->username != NULL)
	        rs = shio_printf(pip->efp,
	            "%s: other_user=%s\n",
	            pip->progname,lcp->username) ;

	    if (lcp->banner != NULL)
	        shio_printf(pip->efp,
	            "%s: other_banner=%s\n",
	            pip->progname,lcp->banner) ;

	}

	return rs ;
}
/* end subroutine (printlockcheck) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{

	if (lip == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (locinfo_finish) */


/* configuration maintenance */
static int config_start(op,pip,cfname)
struct config	*op ;
PROGINFO	*pip ;
const char	*cfname ;
{
	VECSTR		sv ;
	int		rs = SR_OK ;
	char		tmpfname[MAXPATHLEN + 1] ;

	memset(op,0,sizeof(struct config)) ;

	op->pip = pip ;
	tmpfname[0] = '\0' ;
	if (strchr(cfname,'/') == NULL) {

	    if ((rs = vecstr_start(&sv)) >= 0) {

	        vecstr_envset(&sv,"p",pip->pr,-1) ;

	        vecstr_envset(&sv,"e","etc",-1) ;

	        vecstr_envset(&sv,"n",pip->searchname,-1) ;

	        rs = permsched(sched1,&sv,
	            tmpfname,MAXPATHLEN,cfname,R_OK) ;

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

	    op->f.p = TRUE ;
	    rs = config_read(op) ;

	    op->f.p = (rs >= 0) ;
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
struct config	*op ;
{
	PROGINFO	*pip = op->pip ;

	int	rs = SR_NOTOPEN ;


	if (op->f.p) {
	    if ((rs = paramfile_check(&op->p,pip->daytime)) > 0)
	        rs = config_read(op) ;
	}

	return rs ;
}
/* end subroutine (config_check) */


static int config_finish(op)
struct config	*op ;
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_NOTOPEN ;
	int		rs1 ;

	if (op->f.p) {

	    rs1 = expcook_finish(&op->cooks) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = paramfile_close(&op->p) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


static int config_read(op)
struct config	*op ;
{
	PROGINFO	*pip = op->pip ;
	PARAMFILE_CUR	cur ;
	int		rs = SR_NOTOPEN ;
	int		rs1 ;
	int		i ;
	int		ml, vl, el ;
	int		v ;
	cchar		*pr ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f.p) ;
#endif

	if (! op->f.p) goto ret0 ;

	pr = pip->pr ;
	    rs = SR_OK ;
	    for (i = 0 ; params[i] != NULL ; i += 1) {

	        rs1 = FALSE ;
	        switch (i) {

	        case param_msfile:
	            if (pip->f.msu_msfile)
	                rs1 = TRUE ;
	            break ;

	        case param_intrun:
	            if (pip->f.msu_intrun)
	                rs1 = TRUE ;
	            break ;

	        case param_intpoll:
	            if (pip->f.msu_intpoll)
	                rs1 = TRUE ;
	            break ;

	        } /* end switch */

	        if (rs1)
	            continue ;

	        if ((rs = paramfile_curbegin(&op->p,&cur)) >= 0) {

	        while (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: checking for param=%s\n",
	                    params[i]) ;
#endif

	            vl = paramfile_fetch(&op->p,params[i],&cur,
	                vbuf,VBUFLEN) ;

	            if (vl == SR_NOTFOUND) break ;

		    rs = vl ;
		    if (rs < 0) break ;

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

	                case param_logsize:
	                case param_intrun:
	                case param_intpoll:
	                case param_markint:
	                case param_lockint:
	                case param_speedint:
	                    rs1 = cfdecti(ebuf,el,&v) ;

	                    if ((rs1 >= 0) && (v >= 0)) {
	                        switch (i) {

	                        case param_logsize:
	                            pip->logsize = v ;
	                            break ;

	                        case param_intrun:
	                            if (! pip->final.intrun)
	                                pip->intrun = v ;
	                            break ;

	                        case param_intpoll:
	                            if (! pip->final.intpoll)
	                                pip->intpoll = v ;
	                            break ;

	                        case param_markint:
	                            if (! pip->final.markint)
	                                pip->intmark = v ;
	                            break ;

	                        case param_lockint:
	                            if (! pip->final.lockint)
	                                pip->intlock = v ;
	                            break ;

	                        case param_speedint:
	                            pip->intspeed = v ;
	                            break ;

	                        } /* end switch */
	                    } /* end if (valid number) */
	                    break ;

	                case param_msfile:
	                    if (! pip->final.msfile) {
	                        prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                            MSDNAME,MSFNAME,"") ;
	                        if (strcmp(pip->msfname,tmpfname) != 0) {
	                            pip->change.msfile = TRUE ;
	                            mkpath1(pip->msfname,tmpfname) ;
	                        }
	                    }
	                    break ;

	                case param_pidfile:
	                    if (! pip->final.pidname) {
	                        prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                            RUNDNAME,pip->nodename,PIDFNAME) ;
	                        if (strcmp(pip->pidfname,tmpfname) != 0) {
	                            pip->change.pidname = TRUE ;
	                            mkpath1(pip->pidfname,tmpfname) ;
	                        }
	                    }
	                    break ;

	                case param_logfile:
	                    if (! pip->final.logfname) {
	                        pip->have.logfname = TRUE ;
	                        prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                            LOGDNAME,pip->searchname,"") ;
	                        if (strcmp(pip->lfname,tmpfname) != 0) {
	                            pip->changed.logprog = TRUE ;
	                            rs = proginfo_setentry(pip,
					&pip->lfname,tmpfname,-1) ;
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


static int loginfo(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    LOCINFO	*lip = pip->lip ;
	    long	lw ;
	    char	digbuf[DIGBUFLEN + 1] ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	                    if (lip->pidfname != NULL)
	                        logfile_printf(&pip->lh,
	                            "pid=%s",lip->pidfname) ;

	                    logfile_printf(&pip->lh,
	                        "daemon pid=%u",((uint) pip->pid)) ;

	                    logfile_printf(&pip->lh,
	                        "intpoll=%s", strval(digbuf,pip->intpoll)) ;

	                    logfile_printf(&pip->lh,
	                        "intmark=%s", strval(digbuf,pip->intmark)) ;

	                    lw = pip->intrun ;
	                    if ((lw >= 0) && (lw < INT_MAX)) {
	                        timestr_elapsed(lw,timebuf) ;
	                    } else
	                        sncpy1(timebuf,TIMEBUFLEN,"max") ;

	                    logfile_printf(&pip->lh,
	                        "intrun=%s", timebuf) ;

	                    logfile_flush(&pip->lh) ;

	} /* end if */

	return rs ;
} 
/* end subroutine (loginfo) */


