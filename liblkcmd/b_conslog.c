/* b_conslog */

/* utility to log message to the system logger facility */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ conslog <conslog> [-c[=<b>]] [-n <name>[:<version>]] 
		[-s <logsize>] [-if <infile>] [-V]


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
#include	<sys/syslog.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<char.h>
#include	<conslog.h>
#include	<expcook.h>
#include	<linefold.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_conslog.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGCOLS
#define	LOGCOLS		(80-16)		/* log-columns */
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	TAGBUFLEN
#define	TAGBUFLEN	60
#endif

#ifndef	OUTBUFLEN
#define	OUTBUFLEN	100
#endif

#define	TO_READ		4		/* read time-out */
#define	INTFLUSH	4		/* flush interval (4 seconds) */

#define	EOFSTR		"*EOF*"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snscs(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	nleadcasestr(cchar *,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	matpcasestr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	getlogfac(cchar *,int) ;
extern int	getlogpri(cchar *,int) ;
extern int	hasalluc(cchar *,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strncasestr(cchar *,int,cchar *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		audit:1 ;
	uint		name:1 ;
	uint		create:1 ;
	uint		facspec:1 ;
	uint		prispec:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	vecstr		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	cchar		*fac ;
	cchar		*pri ;
	cchar		*alloc ;	/* memory-allocations */
	uid_t		uid, euid ;
	int		to_read ;
	int		logpri ;
	char		timebuf[TIMEBUFLEN + 1] ;
} ;

enum las {
	la_fac,
	la_pri,
	la_overlast
} ;

struct sepstr {
	cchar		*sp ;
	int		sl ;
} ;

struct sepstrs {
	struct sepstr	s[la_overlast] ;
} ;


/* forward references */

static int	mainsub(int,cchar *[],cchar *[],void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procisexit(PROGINFO *) ;
static int	proclog(PROGINFO *,cchar *) ;
static int	process(PROGINFO *,CONSLOG *,cchar *) ;
static int	processer(PROGINFO *,CONSLOG *,EXPCOOK *,cchar *) ;
static int	procline(PROGINFO *,CONSLOG *,cchar *,int) ;
static int	procsubs(PROGINFO *,EXPCOOK *,char *,int,cchar *,int) ;
static int	loadcooks(PROGINFO *,EXPCOOK *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_logparams(LOCINFO *,cchar *,cchar *) ;
static int	locinfo_logvals(LOCINFO *) ;

static int	sepstrs_load(struct sepstrs *,cchar *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"if",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_if,
	argopt_overlast
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

static const char	*akonames[] = {
	"fac",
	"pri",
	"ignore",
	"audit",
	"create",
	NULL
} ;

enum akonames {
	akoname_fac,
	akoname_pri,
	akoname_ignore,
	akoname_audit,
	akoname_create,
	akoname_overlast
} ;


/* exported subroutines */


int b_conslog(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_conslog) */


int p_conslog(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_conslog) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		to_read = TO_READ ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ifname = NULL ;
	cchar		*facspec = NULL ;
	cchar		*prispec = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_conslog: starting DFD=%d\n",rs) ;
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

/* local information */

	pip->lip = &li ;
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

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
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

/* input file */
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

/* create-conslog */
	                    case 'c':
	                        lip->have.create = TRUE ;
	                        lip->final.create = TRUE ;
	                        lip->f.create = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.create = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* facility-specification */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.facspec = TRUE ;
	                                lip->final.facspec = TRUE ;
	                                facspec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

/* priority-specification */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.prispec = TRUE ;
	                                lip->final.prispec = TRUE ;
	                                prispec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* read time-out */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                to_read = v ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_conslog: debuglevel=%u\n",pip->debuglevel) ;
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

/* program search name */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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
	} /* end if (help) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

#ifdef	COMMENT
	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}
#endif /* COMMENT */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if ((! pip->final.ignore) && hasalluc(pip->progname,-1))
	    pip->f.ignore = TRUE ;

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;
	if (ifname == NULL) ifname = getourenv(pip->envv,VARIFNAME) ;

	if (to_read < 1) to_read = 1 ;
	lip->to_read = to_read ;

	if (rs < 0) goto badarg ;

/* give the answers */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        if (cp != '\0') {
	            pan += 1 ;
	            facspec = cp ;
	            break ;
	        }
	    }
	} /* end for (looping through positional arguments) */

	if ((facspec == NULL) || (facspec[0] == '\0')) {
	    facspec = getourenv(pip->envv,VARFAC) ;
	}

	if ((prispec == NULL) || (prispec[0] == '\0')) {
	    prispec = getourenv(pip->envv,VARPRI) ;
	}

	if (rs >= 0) {
	    if ((rs = locinfo_logparams(lip,facspec,prispec)) >= 0) {
	        if ((rs = locinfo_logvals(lip)) >= 0) {
		    if (argval != NULL) {
	                rs = optvalue(argval,-1) ;
	                lip->logpri = rs ;
	            }
		}
	    }
	}

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    {
	                rs = proclog(pip,ifname) ;
		    }
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
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
	        shio_printf(pip->efp,
	            "%s: could not perform function (%d)\n",
	            pip->progname,rs) ;
	    }
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_conslog: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
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
	    debugprintf("b_conslog: final mallout=%u\n",mo-mo_start) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
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
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [[<fac>:]]<tag>[:<ver>] [-f <fac>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p [<fac>:]<pri>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-if <infile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-names */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

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

	                case akoname_fac:
	                    if (! lip->final.facspec) {
	                        lip->have.facspec = TRUE ;
	                        lip->final.facspec = TRUE ;
	                        if (vl > 0) {
				    cchar	**vpp = &lip->fac ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_pri:
	                    if (! lip->final.prispec) {
	                        lip->have.prispec = TRUE ;
	                        lip->final.prispec = TRUE ;
	                        if (vl > 0) {
				    cchar	**vpp = &lip->pri ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_ignore:
	                    if (! pip->final.ignore) {
	                        pip->have.ignore = TRUE ;
	                        pip->final.ignore = TRUE ;
	                        pip->f.ignore = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.ignore = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_audit:
	                    if (! lip->final.audit) {
	                        lip->have.audit = TRUE ;
	                        lip->final.audit = TRUE ;
	                        lip->f.audit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.audit = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_create:
	                    if (! lip->final.create) {
	                        lip->have.create= TRUE ;
	                        lip->final.create = TRUE ;
	                        lip->f.create = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.create = (rs > 0) ;
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


static int procisexit(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return lib_sigterm() ;
}
/* end subroutine (procisexit) */


static int proclog(PROGINFO *pip,cchar *ifname)
{
	LOCINFO		*lip = pip->lip ;
	CONSLOG		ls, *lsp = &ls ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		logfac ;
	int		bytes = 0 ;
	cchar		*fac ;

	fac = lip->fac ;

	if ((fac != NULL) && (fac[0] != '\0')) {
	    rs = getlogfac(fac,-1) ;
	    logfac = rs ;
	} else {
	    logfac = LOG_USER ;
	}

	if (rs >= 0) {
	    if ((rs = conslog_open(lsp,logfac)) >= 0) {

	        rs = process(pip,lsp,ifname) ;
	        bytes = rs ;

	        rs1 = conslog_close(lsp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (conslog) */
	} /* end if (ok) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: bytes=%u\n",pip->progname,bytes) ;
	}

	return (rs >= 0) ? bytes : rs ;
}
/* end subroutine (proclog) */


static int process(PROGINFO *pip,CONSLOG *lsp,cchar *ifn)
{
	EXPCOOK		ck ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_conslog/process: to=%d\n",to) ;
#endif

	if ((rs = expcook_start(&ck)) >= 0) {
	    if ((rs = loadcooks(pip,&ck)) >= 0) {
	        rs = processer(pip,lsp,&ck,ifn) ;
	        tlen = rs ;
	    } /* end if (load-cooks) */
	    rs1 = expcook_finish(&ck) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (expcook) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_conslog/process: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (process) */


static int processer(PROGINFO *pip,CONSLOG *lsp,EXPCOOK *ckp,cchar ifn[])
{
	LOCINFO		*lip = pip->lip ;
	time_t		ti_lastcheck = pip->daytime ;
	const int	to = lip->to_read ;
	const int	intf = INTFLUSH ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		eofstrlen ;
	int		ll ;
	int		tlen = 0 ;
	int		f_stdin = FALSE ;
	int		f_fifo = FALSE ;
	cchar		*eofstr = EOFSTR ;
	cchar		*obp ;
	char		os[10] = { 0 } ;

	eofstrlen = strlen(eofstr) ;

	if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-')) {
	    f_stdin = TRUE ;
	    ifn = STDINFNAME ;
	}

	if (! f_stdin) {
	    struct ustat	sb ;
	    int	rs1 = u_stat(ifn,&sb) ;
	    if (rs1 >= 0) f_fifo = S_ISFIFO(sb.st_mode) ;
	}
	if (rs >= 0) {
	    int	i = 0 ;
	    os[i++] = 'r' ;
	    if (f_fifo) os[i++] = 'n' ;
	    os[i] = '\0' ;
	}

	if (rs >= 0) {
	    SHIO	infile, *ifp = &infile ;
	    if ((rs = shio_open(ifp,ifn,os,0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        const int	olen = OUTBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;
	        char		obuf[OUTBUFLEN + 1] ;

	        if (f_fifo) {
	            rs = shio_control(ifp,SHIO_CNONBLOCK,0) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_conslog/process: "
	                    "shio_control() rs=%d\n",rs) ;
#endif
	        }

	        while (rs >= 0) {
	            ll = shio_readlinetimed(ifp,lbuf,llen,to) ;
	            if (ll == 0) break ; /* EOF */
	            rs = ll ;

	            if ((rs >= 0) && (ll > 0)) {

	                if (strncmp(lbuf,eofstr,eofstrlen) == 0)
	                    break ;

	                obp = obuf ;
	                if ((rs = procsubs(pip,ckp,obuf,olen,lbuf,ll)) >= 0) {
	                    int	obl = rs ;

	                    if (obl == 0) {
	                        obp = lbuf ;
	                        obl = ll ;
	                    }

	                    if (obl > 0) {
	                        rs = procline(pip,lsp,obp,obl) ;
	                        tlen += rs ;
	                    }

	                } /* end if (procsubs) */

	            } /* end if (non-zero data) */

	            pip->daytime = time(NULL) ;
	            if (rs >= 0) {
	                if ((pip->daytime - ti_lastcheck) >= intf) {
	                    ti_lastcheck = pip->daytime ;
	                    rs = conslog_check(lsp,pip->daytime) ;
	                }
	            }

	            if (rs == SR_TIMEDOUT) rs = SR_OK ;
	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs >= 0) rs = procisexit(pip) ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (open-input) */
	} /* end if (ok) */

	if (tlen < 0) tlen = INT_MAX ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (processer) */


static int procline(PROGINFO *pip,CONSLOG *lsp,cchar obuf[],int olen)
{
	LOCINFO		*lip = pip->lip ;
	const int	cols = LOGCOLS ;
	const int	indent = 2 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ol = olen ;
	int		logpri ;
	int		tlen = 0 ;
	cchar		*tp ;
	cchar		*op = obuf ;

	if (ol < 0) ol = strlen(op) ;

	while (ol && CHAR_ISWHITE(*op)) {
	    op += 1 ;
	    ol -= 1 ;
	}

	if ((*op == '<') && ((tp = strnchr(op,ol,'>')) != NULL)) {
	    cchar	*cp = (op+1) ;
	    const int	cl = (tp-(op+1)) ;
	    ol -= ((op+ol)-(tp+1)) ;
	    op = (tp+1) ;
	    rs = getlogpri(cp,cl) ;
	    logpri = rs ;
	} else {
	    logpri = lip->logpri ;
	}

	if (rs >= 0) {
	    LINEFOLD	lf ;
	    if ((rs = linefold_start(&lf,cols,indent,op,ol)) >= 0) {
	        int	i ;
	        int	ll ;
	        cchar	*lp ;
	        for (i = 0 ; (ll = linefold_get(&lf,i,&lp)) >= 0 ; i += 1) {
	            tlen += ll ;
	            rs = conslog_write(lsp,logpri,lp,ll) ;
	            if (rs < 0) break ;
	        } /* end for */
	        rs1 = linefold_finish(&lf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (linefold) */
	} /* end if (ok) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procline) */


static int procsubs(PROGINFO *pip,EXPCOOK *ckp,char obuf[],int olen,
			cchar lbuf[],int llen)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		obl = 0 ;

	if (strnchr(lbuf,llen,'%') != NULL) {
	    time_t	dt = pip->daytime ;

	    if (lip->timebuf[0] == '\0') {
	        pip->daytime = dt ;
	        timestr_logz(dt,lip->timebuf) ;
	        rs = expcook_add(ckp,"T",lip->timebuf,-1) ;
	    }

	    if (rs >= 0) {
	        rs1 = expcook_exp(ckp,0,obuf,olen,lbuf,llen) ;
	        obl = rs1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("b_conslog/procsubs: ec_ex() rs=%d\n",rs1) ;
	            debugprintf("b_conslog/procsubs: obuf=>%t<\n",
	                obuf,strlinelen(obuf,obl,40)) ;
	        }
#endif

	        if (rs1 < 0) {
	            obl = sncpy1(obuf,olen,"*truncated*") ;
	        }
	    } /* end if */

	} /* end if (ok) */

	return (rs >= 0) ? obl : rs ;
}
/* end subroutine (procsubs) */


static int loadcooks(PROGINFO *pip,EXPCOOK *ckp)
{
	int		rs = SR_OK ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"N",pip->nodename,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"D",pip->domainname,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"U",pip->username,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"NAME",pip->name,-1) ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		rs1 ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    char	hbuf[MAXHOSTNAMELEN + 1] ;
	    if ((rs1 = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        rs = expcook_add(ckp,"H",hbuf,rs1) ;
	    }
	}

	return rs ;
}
/* end subroutine (loadcooks) */


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
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

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


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to_read = TO_READ ;
	lip->logpri = -1 ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->alloc != NULL) {
	    rs1 = uc_free(lip->alloc) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->alloc = NULL ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


static int locinfo_logparams(LOCINFO *lip,cchar *facspec,cchar *prispec)
{
	struct sepstrs	seps ;
	int		rs = SR_OK ;

	memset(&seps,0,sizeof(struct sepstrs)) ;

	if ((facspec != NULL) && (facspec[0] != '\0')) {
	    sepstrs_load(&seps,facspec,-1) ;
	} /* end if (facspec) */

	if ((prispec != NULL) && (prispec[0] != '\0')) {
	    cchar	*sp = prispec ;
	    cchar	*tp ;
	    if ((tp = strpbrk(sp,":.")) != NULL) {
	        seps.s[la_fac].sp = sp ;
	        seps.s[la_fac].sl = (tp-sp) ;
	        seps.s[la_pri].sp = (tp+1) ;
	        seps.s[la_pri].sl = -1 ;
	    } else {
	        seps.s[la_pri].sp = prispec ;
	        seps.s[la_pri].sl = -1 ;
	    }
	} /* end if (prispec) */

	{
	    int		size = 0 ;
	    int		i ;
	    char	*p ;

	    for (i = 0 ; i < la_overlast ; i += 1) {
	        cchar	*sp = seps.s[i].sp ;
	        int	sl = seps.s[i].sl ;
	        size += 1 ;
	        if (sp != NULL) size += ((sl >= 0) ? sl : strlen(sp)) ;
	    } /* end for */

	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        char	*bp = p ;
	        lip->alloc = p ;
	        for (i = 0 ; i < la_overlast ; i += 1) {
	            cchar	*sp = seps.s[i].sp ;
	            int		sl = seps.s[i].sl ;
	            if (sp != NULL) {
	                switch (i) {
	                case la_fac:
	                    lip->fac = bp ;
	                    break ;
	                case la_pri:
	                    lip->pri = bp ;
	                    break ;
	                } /* end switch */
	                bp = strwcpy(bp,sp,sl) + 1 ;
	            } /* end if (non-NULL) */
	        } /* end for */
	    } /* end if (memory-allocation) */
	} /* end block */

	return rs ;
}
/* end subroutine (locinfo_logparams) */


static int locinfo_logvals(LOCINFO *lip)
{
	int		rs = SR_OK ;
	cchar		*pri = lip->pri ;

	if ((pri != NULL) && (pri[0] != '\0')) {
	    rs = getlogpri(pri,-1) ;
	    lip->logpri = rs ;
	} else {
	    lip->logpri = LOG_INFO ;
	}

	return rs ;
}
/* end subroutine (locinfo_logvals) */


static int sepstrs_load(struct sepstrs *ssp,cchar sp[],int sl)
{
	int		n = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	memset(ssp,0,sizeof(struct sepstr)) ;

	if (sl > 0) {
	    cchar	*tp ;

	    n += 1 ;
	    if ((tp = strnpbrk(sp,sl,":.")) != NULL) {
	        n += 1 ;
	        ssp->s[la_fac].sp = sp ;
	        ssp->s[la_fac].sl = (tp - sp) ;
	        ssp->s[1].sp = (tp+1) ;
	        ssp->s[1].sl = ((sp+sl)-(tp+1)) ;
	    } else {
	        ssp->s[la_fac].sp = sp ;
	        ssp->s[la_fac].sl = sl ;
	    } /* end if */

	} /* end if (non-zero) */

	return n ;
}
/* end subroutine (sepstrs_load) */


