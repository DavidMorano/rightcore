/* main (nsw) */
/* laguage=C90 */

/* 'netscape' watcher program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocation */
#define	CF_ARMED	1		/* program armed */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ nsw &

	Notes:

	The whole program is pretty much in this file ('main.c').  Happily that
	means that the program is fairly simple (as to all fit in this
	relatively small file).  Further, despite the seemingly proprietary
	nature of our task (ensuring that specified programs run at a certain
	priority) we manage to remain portable and generic in our use of system
	APIs.  We use the |getpriority(3c)| system call and its associated
	|setpriority(3c)| call to perform our function.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/resource.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<ids.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<field.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	vecstr_addcspath(VECSTR *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proglog_begin(PROGINFO *,USERINFO *) ;
extern int	proglog_end(PROGINFO *) ;
extern int	proglog_print(PROGINFO *,cchar *,int) ;
extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procnames(PROGINFO *,VECSTR *,const char *,int ll) ;
static int	procfindps(PROGINFO *) ;
static int	process(PROGINFO *,VECSTR *) ;
static int	procsearch(PROGINFO *,VECSTR *) ;
static int	procsearchline(PROGINFO *,VECSTR *,char *,int) ;
static int	procsearchsub(PROGINFO *,VECSTR *,const char *,int) ;
static int	prochandle(PROGINFO *,pid_t) ;
static int	proclogout(PROGINFO *,int) ;

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"log",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_log,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
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

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	SIGMAN		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	USERINFO	u ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl ;
	int		ai, ai_pos, ai_max, kwi ;
	int		rs, rs1 ;
	int		v ;
	int		scanned = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*cp ;


/* make sure that all of the first three FDs are used up */

#ifdef	COMMENT
	for (i = 0 ; i < 0 ; i += 1) {
	    int rs1 = u_fstat(i,&sb) ;
	    if (rs1 < 0) {
	        int	oflags = (i == 0) ? O_RDONLY : O_WRONLY ;
	        u_open(NULLDEV,oflags,0666) ;
	    }
	} /* end for */
#endif /* COMMENT */

	if_exit = 0 ;
	if_int = 0 ;

	rs = sigman_start(&sm,sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto badsigman ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

/* continue as normal */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

	pip->f.logprog = OPT_LOGPROG ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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
	        const int ach = MKCHAR(argp[1]) ;

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_logfile:
	                case argopt_log:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->lfname = avp ;
				}
	                    }
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

/* argument list file */
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

/* log file name */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->lfname = avp ;
				}
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->lfname = argp ;
				    }
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* run interval */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intrun = v ;
	                            }
	                        }
	                        break ;

/* options */
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

/* priority */
	                    case 'p':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(avp,avl,&v) ;
	                            pip->setprio = v ;
	                        }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* poll interval */
	                    case 't':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->intpoll = v ;
	                        }
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((pip->n == 0) && (argval != NULL)) {
	   rs = optvalue(argval,-1) ;
	   pip->n = rs ;
	}

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (pip->lfname == NULL) pip->lfname = getourenv(pip->envv,VARLFNAME) ;

/* some more initialization */

	if (pip->intpoll <= 0)
	    pip->intpoll = DEFINTPOLL ;

	if (pip->f.daemon && (pip->intrun <= 0))
	    pip->intrun = DEFINTRUN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: runint=%d\n",pip->intrun) ;
#endif

	pip->donetime = (pip->daytime + pip->intrun) ;

/* are we setup as we would like? */

	if (rs >= 0) {
	    uid_t	euid = pip->euid ;
	    if ((pip->euid != 0) && (pip->uid == 0)) {
	        u_seteuid(pip->uid) ;
	        euid = geteuid() ;
	    }
	    if ((euid != 0) && (pip->debuglevel > 0)) {
	        bprintf(pip->efp,"%s: euid=%d\n",pip->progname,euid) ;
	    }
	} /* end if */

	if (pip->setprio == 0) pip->setprio = SETPRIO ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: setprio=%d\n",pip->progname,pip->setprio) ;

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* user information */

	if (rs >= 0) {
	if ((rs = userinfo_start(&u,NULL)) >= 0) {
	    if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	        if ((rs = proglog_begin(pip,&u)) >= 0) {
	            if ((rs = procfindps(pip)) >= 0) {
	                if ((rs = ids_load(&pip->id)) >= 0) {
				ARGINFO	*aip = &ainfo ;
				BITS	*bop = &pargs ;
	                        cchar	*afn = afname ;
	                        if ((rs = procargs(pip,aip,bop,afn)) >= 0) {
	                            scanned = rs ;
	                            rs = proclogout(pip,scanned) ;
	                        }
	                        rs1 = ids_release(&pip->id) ;
	                        if (rs >= 0) rs = rs1 ;
	                } /* end if (ids) */
	            } /* end if (pcoclogps) */
	            rs1 = proglog_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (proglog) */
	        rs1 = procuserinfo_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procuserinfo) */
	    rs1 = userinfo_finish(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done rs=%d\n",rs) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            cchar	*fmt = "%s: invalid query (%d)\n" ;
	            bprintf(pip->efp,fmt,pip->progname,rs) ;
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
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

/* we are done */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

badsigman:
	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(int sn)
{
	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (sighand_int) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-t <poll>] [-af <afile>] [<name(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p <prio>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        const char	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procuserinfo_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	VECSTR		names ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = vecstr_start(&names,1,0)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; (rs >= 0) && (ai < aip->argc) ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = vecstr_adduniq(&names,cp,-1) ;
	                }
	            }

		    if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = BFILE_STDIN ;

	        if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procnames(pip,&names,cp,cl) ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
			fmt = "%s: inaccessible argument-list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {
	        const char	*cp = DEFPROGNAME ;
	        rs = vecstr_adduniq(&names,cp,-1) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        int	i ;
	        debugprintf("main: search-names¬\n") ;
	        for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: i=%u n=%s\n",i,cp) ;
	    }
#endif /* CF_DEBUG */

/* take some daemon actions */

	    if ((rs >= 0) && (pip->intrun > 0)) {
	        u_setsid() ;
	    } /* end if (daemon mode) */

	    if (rs >= 0) {
	        rs = process(pip,&names) ;
	        c = rs ;
	    }

	    rs1 = vecstr_finish(&names) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,VECSTR *nlp,cchar *lp,int ll)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		fl ;
	    const char	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = vecstr_adduniq(nlp,fp,fl) ;
	            if (rs < INT_MAX) c += 1 ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


static int procfindps(PROGINFO *pip)
{
	VECSTR		paths ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

	if ((rs = vecstr_start(&paths,5,0)) >= 0) {
	    if ((rs = vecstr_addcspath(&paths)) >= 0) {
		IDS	*idp = &pip->id ;
	        cchar	*pn = PROG_PS ;
	        char	pbuf[MAXPATHLEN+1] ;
	        if ((rs = getprogpath(idp,&paths,pbuf,pn,-1)) >= 0) {
	            const char	**vpp = &pip->pfname ;
	            pl = rs ;
	            rs = proginfo_setentry(pip,vpp,pbuf,pl) ;
		}
	    }
	    rs1 = vecstr_finish(&paths) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfindps: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (procfindps) */


static int process(PROGINFO *pip,VECSTR *nlp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		c_total = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: ent\n") ;
#endif

	while ((rs >= 0) && (pip->daytime <= pip->donetime)) {
	    char	timebuf[TIMEBUFLEN+1] ;

	    rs = procsearch(pip,nlp) ;
	    c = rs ;

	    if ((rs >= 0) && (c > 0)) {
	        c_total += c ;
	        if (pip->open.logprog) {
	            timestr_logz(pip->daytime,timebuf) ;
	            proglog_printf(pip,"%s scanned=%u",timebuf,c) ;
	        }
	    }

	    if (pip->f.daemon) {
	        sleep(pip->intpoll) ;
	        pip->daytime = time(NULL) ;
	    } else
	        break ;

	    if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	} /* end while (looping) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: ret rs=%d total=%u\n",rs,c_total) ;
#endif

	return (rs >= 0) ? c_total : rs ;
}
/* end subroutine (process) */


static int procsearch(PROGINFO *pip,VECSTR *nlp)
{
	const int	of = O_RDONLY ;
	const int	to = 10 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	int		i = 0 ;
	const char	*pfname = pip->pfname ;
	const char	*args[8] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procsearch: ent\n") ;
#endif

	args[i++] = PROG_PS ;
	args[i++] = "-A" ;
	args[i++] = "-o" ;
	args[i++] = "pid,comm" ;
	args[i] = NULL ;

	if ((rs = uc_openprog(pfname,of,args,NULL)) >= 0) {
	    FILEBUF	b ;
	    const int	fd = rs ;

	    if ((rs = filebuf_start(&b,fd,0L,2048,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		line = 0 ;
	        int		f_bol = TRUE ;
	        int		f_eol ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	            int	len = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procsearch: l=>%t<\n",
	                    lbuf,strlinelen(lbuf,len,40)) ;
#endif

	            f_eol = (lbuf[len-1] == '\n') ;
	            if (f_eol) lbuf[--len] = '\0' ;

	            if (f_bol && f_eol && (line > 0)) {
	                rs = procsearchline(pip,nlp,lbuf,len) ;
	                c += rs ;
	            }

	            if (f_eol) line += 1 ;

	            f_bol = f_eol ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = filebuf_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */

	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened directory) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procsearch: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsearch) */


static int procsearchline(PROGINFO *pip,VECSTR *nlp,char lbuf[],int len)
{
	int		rs = SR_OK ;
	int		cl ;
	int		v ;
	int		cmdl ;
	int		c = 0 ;
	const char	*cmdp ;
	const char	*cp ;

	if (len < 0) len = strlen(lbuf) ;

	if ((cl = nextfield(lbuf,len,&cp)) > 0) {
	    if ((rs = cfdeci(cp,cl,&v)) >= 0) {
	        pid_t		pid = (pid_t) v ;
	        const int	ccl = (len-((cp+cl+1)-lbuf)) ;
	        const char	*ccp = (cp+cl+1) ;
	        if ((cmdl = sfshrink(ccp,ccl,&cmdp)) > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procsearchline: pid=%u c=%t\n",
	                    pid,cmdp,strlinelen(cmdp,cmdl,40)) ;
#endif

	            if ((rs = procsearchsub(pip,nlp,cmdp,cmdl)) > 0) {
	                c += 1 ;
	                rs = prochandle(pip,pid) ;
	            }

	        } /* end if (cmd) */
	    } /* end if (valid PID) */
	} /* end if (PID field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsearchline) */


static int procsearchsub(PROGINFO *pip,vecstr *nlp,cchar *cmdp,int cmdl)
{
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;
	const char	*np ;

	if (pip == NULL) return SR_FAULT ;

	for (i = 0 ; vecstr_get(nlp,i,&np) >= 0 ; i += 1) {
	    if (np != NULL) {
	        f = (sfsub(cmdp,cmdl,np,NULL) > 0) ;
	        if (f) break ;
	    }
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procsearchsub) */


static int prochandle(PROGINFO *pip,pid_t pid)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		prio ;

	rs1 = uc_getpriority(PRIO_PROCESS,pid,&prio) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("handleproc: uc_getpriority() rs=%d pri=%d\n",
	        rs1,prio) ;
	}
#endif /* CF_DEBUG */

	if (pip->open.logprog) {
	    if (rs1 >= 0) {
	        proglog_printf(pip,"pid=%u pri=%d\n",pid,prio) ;
	    } else {
	        proglog_printf(pip,
	            "could not get priority pid=%u\n",pid) ;
	    }
	} /* end if */

	if ((rs1 >= 0) && (prio > pip->setprio)) {

#if	CF_ARMED
	    prio = pip->setprio ;
	    rs1 = uc_setpriority(PRIO_PROCESS,pid,prio) ;
#else /* CF_ARMED */
	    rs1 = SR_OK ;
#endif /* CF_ARMED */

	    if (pip->debuglevel > 0) {
	        const char	*pn = pip->progname ;
	        if (rs1 >= 0) {
	            bprintf(pip->efp,"%s: changed pid=%u\n",pn,pid) ;
	        } else {
	            bprintf(pip->efp,"%s: could not change pid=%u\n",pn,pid) ;
		}
	    } /* end if (debugging turned on) */

	    if (pip->open.logprog) {
	        if (rs1 >= 0) {
	            proglog_printf(pip,"changed pid=%u\n",pid) ;
	        } else {
	            proglog_printf(pip,"could not change pid=%u\n",pid) ;
		}
	    } /* end if (logging) */

	} /* end if (had a process and it was bad) */

	rs1 = (rs1 >= 0) ? 1 : 0 ;

	return (rs >= 0) ? rs1 : rs ;
}
/* end subroutine (prochandle) */


static int proclogout(PROGINFO *pip,int scanned)
{
	cchar	*fmt ;

	if (pip->debuglevel > 0) {
	    if (scanned >= 0) {
		fmt = "%s: total scanned=%u\n" ;
	        bprintf(pip->efp,fmt,pip->progname,scanned) ;
	    } else {
	        fmt = "%s: error (%d)\n" ;
	        bprintf(pip->efp,fmt,pip->progname,scanned) ;
	    }
	} /* end if */

	if (pip->open.logprog) {
	    char	timebuf[TIMEBUFLEN+1] ;
	    if (scanned >= 0) {
	        timestr_logz(pip->daytime,timebuf) ;
	        fmt = "%s total scanned=%u" ;
	        proglog_printf(pip,fmt,timebuf,scanned) ;
	    } else {
	        timestr_logz(pip->daytime,timebuf) ;
	        fmt = "%s error (%d)" ;
	        proglog_printf(pip,fmt,timebuf,scanned) ;
	    }
	} /* end if (logging) */

	return SR_OK ;
}
/* end subroutine (proclogout) */


