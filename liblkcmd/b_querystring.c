/* b_querystring */

/* SHELL built-in to return load averages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This parses web a query-string into key-value pairs and prints them out
	to standard-output.

	Synopsis:

	$ querystring [-qs <querystring>]


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
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<userinfo.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_querystring.h"
#include	"defs.h"
#include	"proglog.h"


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* local defines */

#define	CVTBUFLEN	100

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfhexi(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	mksublogid(char *,int,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	ishexlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	strlinelen(const char *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		dummy:1 ;
	uint		init:1 ;
	uint		nocache:1 ;
	uint		nprocs:1 ;
	uint		ncpus:1 ;
	uint		btime:1 ;
	uint		rnum:1 ;
	uint		mem:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	PROGINFO	*pip ;
	const char	*utfname ;
	time_t		btime ;		/* machine boot-time */
	uint		nprocs ;
	uint		ncpus ;
	uint		rnum ;
	uint		pmt ;		/* physical-memory-total */
	uint		pma ;		/* physical-memory-avail */
	uint		pmu ;		/* physical-memory-usage */
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *,cchar *) ;
static int	procinput(PROGINFO *,void *,cchar *) ;
static int	procname(PROGINFO *,void *, const char *) ;
static int	procnamer(PROGINFO *,void *,cchar *,int) ;
static int	procfixval(PROGINFO *,char *,int,cchar *,int) ;
static int	proclogline(PROGINFO *,cchar *) ;
static int	proclog(PROGINFO *,cchar *,int,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

static char	*strwebhex(char *,cchar *,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"qs",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_qs,
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

static const char *akonames[] = {
	"dummy",
	NULL
} ;

enum akonames {
	akoname_dummy,
	akoname_overlast
} ;


/* exported subroutines */


int b_querystring(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_querystring) */


int p_querystring(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_querystring) */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO	li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
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
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*qs = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_querystring: starting DFD=%d\n",rs) ;
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
	pip->daytime = time(NULL) ;
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

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

/* keyword match or only key letters? */

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

/* output file name */
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

/* input file name */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ifname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log file */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->lfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* query-string */
	                case argopt_qs:
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            qs = argp ;
				} else
	                            rs = SR_INVALID ;
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

/* quiet mode */
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
	    debugprintf("b_querystring: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

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

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (qs == NULL) qs = getourenv(envv,VARQS) ;
	if (qs == NULL) qs = getourenv(envv,VARQUERYSTRING) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* OK, we finally do our thing */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            if ((rs = proglog_begin(pip,&u)) >= 0) {
			{
			    ARGINFO	*aip = &ainfo ;
			    BITS	*bop = &pargs ;
			    cchar	*afn = afname ;
			    cchar	*ofn = ofname ;
			    cchar	*ifn = ifname ;
			    rs = procargs(pip,aip,bop,ofn,afn,ifn,qs) ;
			}
		        rs1 = proglog_end(pip) ;
		        if (rs >= 0) rs = rs1 ;
		    } /* end if (proglogfile) */
	            rs1 = procuserinfo_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

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
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    rs1 = shio_close(pip->efp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

badpargs:
	rs1 = locinfo_finish(lip) ;
	if (rs >= 0) rs = rs1 ;

badlocstart:
	rs1 = proginfo_finish(pip) ;
	if (rs >= 0) rs = rs1 ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_querystring: final mallout=%u\n",(mo-mo_start)) ;
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
/* end subroutine (b_querystring) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-qs <querystring>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
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

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_dummy:
	                    if (! lip->final.dummy) {
	                        lip->have.dummy = TRUE ;
	                        lip->final.dummy = TRUE ;
	                        lip->f.dummy = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
				    lip->f.dummy = (rs > 0) ;
				}
	                    }
	                    break ;
	                default:
	                    rs = SR_INVALID ;
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

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

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


static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
	    if (rs & KSHLIB_RMKSH) {
		if ((rs = lib_serial()) >= 0) {
		    const int	s = rs ;
		    const int	plen = LOGIDLEN ;
		    const int	pv = pip->pid ;
		    const char	*nn = pip->nodename ;
		    char	pbuf[LOGIDLEN+1] ;
		    if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
			const int	slen = LOGIDLEN ;
			char		sbuf[LOGIDLEN+1] ;
			if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
			    const char	**vpp = &pip->logid ;
			    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
			}
		    }
		} /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int procargs(pip,aip,bop,ofn,afn,ifn,qs)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
const char	*ofn ;
const char	*afn ;
const char	*ifn ;
const char	*qs ;
{
	SHIO		ofile, *ofp = &ofile ;
	const int	to_open = -1 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_opene(ofp,ofn,"wct",0666,to_open)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;

	    if (rs >= 0) {
		int	ai ;
		int	f ;
		cchar	**argv = aip->argv ;
		for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    	    f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	    	    if (f) {
	                cp = argv[ai] ;
			if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procname(pip,ofp,cp) ;
			}
		    }

		    if (rs >= 0) rs = lib_sigterm() ;
		    if (rs >= 0) rs = lib_sigintr() ;
	    	    if (rs < 0) break ;
		} /* end for */
	    } /* end if (ok) */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            SHIO	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	            if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

			    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
				lbuf[(cp-lbuf)+cl] = '\0' ;
				if (cp[0] != '#') {
	                            pan += 1 ;
	                            rs = procname(pip,ofp,cp) ;
	                            wlen += rs ;
				}
			    }

		    	    if (rs >= 0) rs = lib_sigterm() ;
		    	    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
		        if (rs >= 0) rs = rs1 ;
	            } else {
			fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
			fmt = "%s: afile=%s\n" ;
	                shio_printf(pip->efp,fmt,pn,afn) ;
	            } /* end if */

	        } /* end if (procesing file argument file list) */

		if ((rs >= 0) && (ifn != NULL) && (ifn[0] != '\0')) {
		    pan += 1 ;
		    rs = procinput(pip,ofp,ifn) ;
		    wlen += rs ;
		}

	        if ((rs >= 0) && (pan == 0)) {
		    if (qs != NULL) {
	                pan += 1 ;
	                rs = procname(pip,ofp,qs) ;
	                wlen += rs ;
		    }
	        } /* end if (option-argument) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    fmt = "%s: ofile=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procinput(PROGINFO *pip,void *ofp,cchar *ifn)
{
	SHIO		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (strcmp(ifn,"-") == 0) ifn = STDINFNAME ;

	if ((rs = shio_open(ifp,ifn,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		cl ;
	    const char	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

			    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
				lbuf[(cp-lbuf)+cl] = '\0' ;
				if (cp[0] != '#') {
	                            rs = procname(pip,ofp,cp) ;
	                            wlen += rs ;
				}
			    }

		    	    if (rs >= 0) rs = lib_sigterm() ;
		    	    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(ifp) ;
		        if (rs >= 0) rs = rs1 ;
	            } else {
	                    shio_printf(pip->efp,
	                        "%s: inaccessible input (%d)\n",
	                        pip->progname,rs) ;
	                    shio_printf(pip->efp,"%s: ifile=%s\n",
	                        pip->progname,ifn) ;
	            } /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procinput) */


/* process a specification name */
static int procname(PROGINFO *pip,void *ofp,cchar *qs)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if ((qs != NULL) && (qs[0] != '\0')) {
	    int		sl = strlen(qs) ;
	    int		cl ;
	    const char	*sp = qs ;
	    const char	*tp ;
	    const char	*cp ;

	    if (pip->verboselevel >= 2)
		proclogline(pip,qs) ;

	    if (pip->debuglevel > 0) {
		shio_printf(pip->efp,"%s: £ »%t«\n",
		    pip->progname,qs,strlinelen(qs,-1,60)) ;
	    }

	    while ((tp = strnchr(sp,sl,'&')) != NULL) {
		cp = sp ;
		cl = (tp-sp) ;
	        if (cl > 0) {
		    rs = procnamer(pip,ofp,cp,cl) ;
		    wlen += rs ;
		}
	        sl -= ((tp-1)-sp) ;
	        sp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (sl > 0)) {
		    rs = procnamer(pip,ofp,sp,sl) ;
		    wlen += rs ;
	    }

	} /* end if (non-null) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_querystring/procname: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


static int procnamer(PROGINFO *pip,void *ofp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		kl ;
	int		vl = 0 ;
	int		wlen = 0 ;
	const char	*tp ;
	const char	*kp ;
	const char	*vp = NULL ;
	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_querystring/procnamer: ent\n") ;
	    debugprintf("b_querystring/procnamer: sl=%d s=>%t<\n",
		sl,sp,sl) ;
	}
#endif
	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	kp = sp ;
	kl = sl ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    kl = (tp - sp) ;
	    vp = (tp + 1) ;
	    vl = ((sp + sl) - vp) ;
	}
	if (kl > 0) {
	    const int	vlen = MAXNAMELEN ;
	    char	vbuf[MAXNAMELEN+1] ;
	    while ((vl > 0) && CHAR_ISWHITE(*vp)) {
		vp += 1 ;
		vl -= 1 ;
	    }
	    if ((vl > 0) && ((strnpbrk(vp,vl,"% +\t")) != NULL)) {
		rs = procfixval(pip,vbuf,vlen,vp,vl) ;
		vl = rs ;
		vp = vbuf ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_querystring/procnamer: vl=%d f=>%t<\n",vl,vp,vl) ;
#endif
	    } /* end if (value) */
	    if (rs >= 0) {
		rs = shio_printf(ofp,"%t\t%t\n",kp,kl,vp,vl) ;
		wlen += rs ;
		if (pip->open.logprog && pip->f.logprog) {
		    proclog(pip,kp,kl,vp,vl) ;
		}
	    } /* end if (ok) */
	} /* end if (key) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnamer) */


static int procfixval(PROGINFO *pip,char *rbuf,int rlen,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	if (vl > 0) {
	    cchar	*tp ;
	    char	*rp = rbuf ;
	    if (vl > rlen) vl = rlen ;
	    while ((tp = strnpbrk(vp,vl,"%+\t")) != NULL) {
	 	const int	sch = MKCHAR(*tp) ;
		if ((tp-vp) > 0) {
		    rp = strwcpy(rp,vp,(tp-vp)) ;
		}
		switch (sch) {
		case '+':
		    if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
		    break ;
		case '\t':
		    if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
		    break ;
		case '%':
		    {
			const int	tl = (vl-(tp-vp)) ;
		        rp = strwebhex(rp,tp,tl) ;
		        tp += MIN(2,tl) ;
		    }
		    break ;
		} /* end switch */
		vl -= ((tp+1)-vp) ;
		vp = (tp+1) ;
	    } /* end while */
	    if ((rs >= 0) && (vl > 0)) {
		while ((vl > 0) && CHAR_ISWHITE(vp[vl-1])) vl -= 1 ;
		rp = strwcpy(rp,vp,vl) ;
	    }
	    i = (rp-rbuf) ;
	} /* end if (positive) */
	rbuf[i] = '\0' ;
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (procfixval) */


static int proclogline(PROGINFO *pip,cchar *qs)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    LOGFILE	*lfp = &pip->lh ;
	    cchar	*fmt = "£ »%s«" ;
	    logfile_printf(lfp,fmt,qs) ;
	}
	return rs ;
}
/* end subroutine (proclogline) */


static int proclog(pip,kp,kl,vp,vl)
PROGINFO	*pip ;
const char	*kp ;
const char	*vp ;
int		kl ;
int		vl ;
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    LOGFILE	*lfp = &pip->lh ;
	    cchar	*fmt = "· %t=%t" ;
	    if ((vl > 0) && (strnpbrk(vp,vl," \t+-\"\'") != NULL)) {
	        fmt = "· %t=¦%t¦" ;
	    }
	    logfile_printf(lfp,fmt,kp,kl,vp,vl) ;
	}
	return rs ;
}
/* end subroutine (proclog) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	if (lip == NULL) return SR_FAULT ;

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


static char *strwebhex(char *rp,cchar *tp,int tl)
{
	if ((tl >= 3) && (*tp == '%')) {
	    const int	ch1 = MKCHAR(tp[1]) ;
	    const int	ch2 = MKCHAR(tp[2]) ;
	    if (ishexlatin(ch1) && ishexlatin(ch2)) {
		int	v ;
		if (cfhexi((tp+1),2,&v) >= 0) {
		    *rp++ = v ;
		}
	    }
	}
	return rp ;
}
/* end subroutine (strwebhex) */


