/* b_msinfo */

/* query the machine status database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FLOAT	0		/* use floating point */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written. 

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a MSINFO
	program out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child procnodees to exit explicitly.  Child procnodees are only
	created when run in "daemon" mode.

	Implemtation note: We do not print out the data on a node whence we
	first get it.  First we collect all of the data on all nodes, and then
	we print out all nodes that we have data for.  We do this because,
	there is optimized locking within the MSFILE object and the look can be
	held from one enumeration of a node to another (that is the
	optimization) and if we printed stuff out between getting a node's
	information, we cannot guarantee that we will not be blocked on output
	while we might be holding the MSFILE lock.

	Synopsis:

	$ msinfo [-msfile <file>]


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
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<estrings.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<msfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_msinfo.h"
#include	"defs.h"


/* local defines */

#ifndef	MSFLAG_MDISABLED
#define	MSFLAG_MDISABLED	0x0001
#endif

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 20)
#endif

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	msfile_best(MSFILE *,time_t,uint,MSFILE_ENT *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		nh:1 ;
	uint		all:1 ;
	uint		empty:1 ;
	uint		enabled:1 ;
	uint		age:1 ;
	uint		speedname:1 ;
	uint		speedint:1 ;
	uint		zerospeed:1 ;
	uint		zeroentry:1 ;
	uint		geekout:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	cchar		*speedname ;
	vecstr		stores ;
	int		speedint ;
	int		ndisplay ;
} ;

struct lav {
	uint		i, f ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,VECSTR *,cchar *) ;
static int	procloads(PROGINFO *,vecstr *,cchar *,int) ;
static int	procload(PROGINFO *,vecstr *,cchar *,int) ;

static int	procout(PROGINFO *,VECSTR *,int,int (*cmpfunc)(),
			cchar *,cchar *) ;
static int	procmsfile(PROGINFO *,VECSTR *,VECOBJ *,void *,
			cchar *,int,int) ;
static int	procnode(PROGINFO *,vecobj *,SHIO *,MSFILE *,int,cchar *) ;
static int	printout(PROGINFO *,SHIO *,VECOBJ *) ;
static int	printnode(PROGINFO *,void *,MSFILE_ENT *) ;
static int	printnodeage(PROGINFO *,void *,MSFILE_ENT *) ;

static int	calcmu(uint,uint) ;

#if	(CF_FLOAT == 0)
static int	mklav(struct lav *,uint) ;
#endif

static int	cmp_utime(), cmpr_utime() ;
static int	cmp_stime(), cmpr_stime() ;
static int	cmp_dtime(), cmpr_dtime() ;
static int	cmp_la1m(), cmpr_la1m() ;
static int	cmp_speed(), cmpr_speed() ;
static int	cmp_pmt(), cmpr_pmt() ;
static int	cmp_pma(), cmpr_pma() ;
static int	cmp_ncpu(), cmpr_ncpu() ;

#if	CF_DEBUGS || CF_DEBUG
static int	debugmse(MSFILE_ENT *) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"db",
	"msfile",
	"mspoll",
	"speedname",
	"zerospeed",
	"zeroentry",
	"nh",
	"empty",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"age",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_db,
	argopt_msfile,
	argopt_mspoll,
	argopt_speedname,
	argopt_zerospeed,
	argopt_zeroentry,
	argopt_nh,
	argopt_empty,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_age,
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

static const char	*progopts[] = {
	"quiet",
	"speedname",
	"speedint",
	"age",
	"all",
	"geekout",
	NULL
} ;

enum progopts {
	progopt_quiet,
	progopt_speedname,
	progopt_speedint,
	progopt_age,
	progopt_all,
	progopt_geekout,
	progopt_overlast
} ;

static const char	*sortkeys[] = {
	"none",
	"utime",
	"stime",
	"dtime",
	"la1min",
	"la5min",
	"la15min",
	"speed",
	"pmt",
	"pma",
	"cpu",
	"ncpu",
	NULL
} ;

enum sortkeys {
	sortkey_none,
	sortkey_utime,
	sortkey_stime,
	sortkey_dtime,
	sortkey_la1m,
	sortkey_la5m,
	sortkey_la15m,
	sortkey_speed,
	sortkey_pmt,
	sortkey_pma,
	sortkey_cpu,
	sortkey_ncpu,
	sortkey_overlast
} ;

static const char	*prognames[] = {
	"msinfo",
	"msage",
	NULL
} ;

enum prognames {
	progname_msinfo,
	progname_msage,
	progname_overlast
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


int b_msinfo(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_msinfo) */

int p_msinfo(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_msinfo) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
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
	int		i ;
	int		v ;
	int		ski = -1 ;
	int		(*cmpfunc)() ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_reverse = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	char		msfnamebuf[MAXPATHLEN + 1] ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*msfname = NULL ;
	cchar		*sortspec = NULL ;
	cchar		*algorithm = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_msinfo: starting DFD=%d\n",rs) ;
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
	pip->intpoll = TO_POLL ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	lip->f.nh = FALSE ;

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

/* output file */
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

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cp = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* MS file name */
	                case argopt_db:
	                case argopt_msfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            msfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                msfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* MS poll interval */
	                case argopt_mspoll:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intpoll = v ;
	                        }
	                    } else {
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
	                    }
	                    break ;

	                case argopt_speedname:
	                    lip->have.speedname = TRUE ;
	                    lip->final.speedname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->speedname = avp ;
	                    }
	                    break ;

	                case argopt_zerospeed:
	                    lip->f.zerospeed = TRUE ;
	                    break ;

	                case argopt_zeroentry:
	                    lip->f.zeroentry = TRUE ;
	                    break ;

	                case argopt_nh:
	                    lip->f.nh = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.nh = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_empty:
	                    lip->f.empty = TRUE ;
	                    break ;

/* age */
	                case argopt_age:
	                    lip->f.age = TRUE ;
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

/* algorithm */
	                    case 'a':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                algorithm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* only enabled nodes */
	                    case 'e':
	                        lip->f.enabled = TRUE ;
	                        break ;

	                    case 'h':
	                        lip->f.nh = FALSE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.nh = (rs == 0) ;
	                            }
	                        }
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

/* node only */
	                    case 'n':
	                        pip->f.nodeonly = TRUE ;
	                        break ;

/* reverse search sense */
	                    case 'r':
	                        f_reverse = TRUE ;
	                        break ;

/* sort specification */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sortspec = argp ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_msinfo: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* continue */

	if ((i = matstr(prognames,pip->progname,-1)) >= 0) {
	    switch (i) {
	    case progname_msage:
	        lip->f.age = TRUE ;
	        break ;
	    } /* end switch */
	}

	if (pip->intpoll < 2)
	    pip->intpoll = 2 ;

	if ((rs >= 0) && (lip->ndisplay <= 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    lip->ndisplay = rs ;
	}

/* sort key */

	ski = sortkey_none ;
	if ((rs >= 0) && (sortspec != NULL)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_msinfo: sortkeyspec=%s\n",sortspec) ;
#endif

	    i = matostr(sortkeys,2,sortspec,-1) ;

	    if ((pip->debuglevel > 0) && (i < 0)) {
	        shio_printf(pip->efp,"%s: invalid sort key\n",
	            pip->progname) ;
	    }

	    if (i >= 0) ski = i ;

	} /* end if (sortspec) */

	switch (ski) {
	case sortkey_none:
	    break ;
	default:
	case sortkey_utime:
	    ski = sortkey_utime ;
	    cmpfunc = (f_reverse) ? cmpr_utime : cmp_utime ;
	    break ;
	case sortkey_stime:
	    ski = sortkey_stime ;
	    cmpfunc = (f_reverse) ? cmpr_stime : cmp_stime ;
	    break ;
	case sortkey_dtime:
	    ski = sortkey_dtime ;
	    cmpfunc = (f_reverse) ? cmpr_dtime : cmp_dtime ;
	    break ;
	case sortkey_la1m:
	    ski = sortkey_la1m ;
	    cmpfunc = (f_reverse) ? cmpr_la1m : cmp_la1m ;
	    break ;
	case sortkey_la5m:
	    ski = sortkey_la1m ;
	    cmpfunc = (f_reverse) ? cmpr_la1m : cmp_la1m ;
	    break ;
	case sortkey_la15m:
	    ski = sortkey_la1m ;
	    cmpfunc = (f_reverse) ? cmpr_la1m : cmp_la1m ;
	    break ;
	case sortkey_speed:
	    ski = sortkey_speed ;
	    cmpfunc = (f_reverse) ? cmpr_speed : cmp_speed ;
	    break ;
	case sortkey_pmt:
	    ski = sortkey_pmt ;
	    cmpfunc = (f_reverse) ? cmpr_pmt : cmp_pmt ;
	    break ;
	case sortkey_pma:
	    ski = sortkey_pma ;
	    cmpfunc = (f_reverse) ? cmpr_pma : cmp_pma ;
	    break ;
	case sortkey_cpu:
	case sortkey_ncpu:
	    ski = sortkey_ncpu ;
	    cmpfunc = (f_reverse) ? cmpr_ncpu : cmp_ncpu ;
	    break ;
	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msinfo: sortkey=%s(%u)\n",sortkeys[ski],ski) ;
#endif

/* initialization */

	if (rs >= 0) {
	    rs = proginfo_nodename(pip) ;	/* ensure we have anodename */
	}

/* try to find a MS file */

	if (msfname == NULL) {
	    if ((cp = getourenv(envv,VARMSFNAME)) != NULL) {
	        msfname = cp ;
	    }
	}

	if ((rs >= 0) && (msfname == NULL)) {
	    msfname = msfnamebuf ;
	    rs = mkpath3(msfnamebuf,pip->pr,"var",MSFNAME) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msinfo: MS stuff, zerospeed=%u\n",
	        lip->f.zerospeed) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: msfname=%s\n",
	        pip->progname,msfname) ;
	    if (algorithm != NULL) {
	        shio_printf(pip->efp,"%s: algorithm=%s\n",
	            pip->progname,algorithm) ;
	    }
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* collect the nodes (if we have any) */

	if (rs >= 0) {
	    vecstr	nodes ;
	    if ((rs = vecstr_start(&nodes,10,0)) >= 0) {

	        if (! lip->f.all) {
	            rs = procargs(pip,&ainfo,&pargs,&nodes,afname) ;
	        } /* end if (not doing all nodes in DB) */

	        if (rs >= 0) {
	            rs = procout(pip,&nodes,ski,cmpfunc,msfname,ofname) ;
	        }

	        rs1 = vecstr_finish(&nodes) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        ex = EX_OSERR ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        cchar *pn = pip->progname ;
	        cchar *fmt = "%s: could not perform function (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	    switch (rs) {
	    case SR_NOMEM:
	        ex = EX_OSERR ;
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
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

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
	    debugprintf("b_msinfo: final mallout=%u\n",(mo-mo_start)) ;
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
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-nh] [-db <dbfile>] [<node(s)> ...]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {
	                int	v ;

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_quiet:
	                    if (! pip->final.quiet) {
	                        pip->have.quiet = TRUE ;
	                        pip->final.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_speedname:
	                    if (! lip->final.speedname) {
	                        lip->have.speedname = TRUE ;
	                        lip->final.speedname = TRUE ;
	                        if (vl > 0) {
	                            cchar	**vpp = &lip->speedname ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case progopt_speedint:
	                    if (! lip->final.speedint) {
	                        lip->have.speedint = TRUE ;
	                        lip->final.speedint = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->speedint = v ;
	                        }
	                    }
	                    break ;
	                case progopt_age:
	                    if (! lip->final.age) {
	                        lip->have.age = TRUE ;
	                        lip->final.age = TRUE ;
	                        lip->f.age = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.age = (rs > 0) ;
	                        }
	                    } /* end if */
	                    break ;
	                case progopt_all:
	                    if (! lip->final.all) {
	                        lip->have.all = TRUE ;
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.all = (rs > 0) ;
	                        }
	                    } /* end if */
	                    break ;
	                case progopt_geekout:
	                    if (! lip->final.geekout) {
	                        lip->have.geekout = TRUE ;
	                        lip->final.geekout = TRUE ;
	                        lip->f.geekout = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.geekout = (rs > 0) ;
	                        }
	                    } /* end if */
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,VECSTR *nnp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		c = 0 ;
	int		cl ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procload(pip,nnp,cp,-1) ;
			c += rs ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procloads(pip,nnp,cp,cl) ;
	                        c += rs ;
	                    }
	                }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procloads(PROGINFO *pip,vecstr *nnp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procload(pip,nnp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloads) */


static int procload(PROGINFO *pip,vecstr *nnp,cchar *sp,int sl)
{
	int		rs ;
	rs = vecstr_adduniq(nnp,sp,sl) ;
	if ((rs >= 0) && (rs < INT_MAX)) rs = 1 ;
	return rs ;
}
/* end subroutine (procload) */


static int procout(pip,nnp,ski,cmpfunc,msfn,ofn)
PROGINFO	*pip ;
vecstr		*nnp ;
int		ski ;
int		(*cmpfunc)() ;
cchar		*msfn ;
cchar		*ofn ;
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;

	if ((ofn == NULL) || (ofn[0] == '\0'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    VECOBJ	entries ;
	    const int	esize = sizeof(MSFILE_ENT) ;

	    if ((rs = vecobj_start(&entries,esize,DEFNODES,0)) >= 0) {
	        int	msflags = 0 ;
	        int	oflags = 0 ;

	        if (lip->f.enabled)
	            msflags |= MSFILE_FLA ;

	        if (lip->f.empty)
	            msflags |= MSFILE_FUSERS ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msinfo: msfile_open() fn=%s\n",msfn) ;
#endif

	        oflags = O_RDONLY ;
	        if (lip->f.zerospeed || lip->f.zeroentry)
	            oflags = O_RDWR ;

	        rs = procmsfile(pip,nnp,&entries,ofp,msfn,oflags,msflags) ;

/* optionally sort them according to the spec (if given) */

	        if ((rs >= 0) && (ski >= 0) && (ski != sortkey_none))
	            vecobj_sort(&entries,cmpfunc) ;

/* print everything out */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msinfo: printing out\n") ;
#endif

	        if (rs >= 0) {
	            rs = printout(pip,ofp,&entries) ;
	        }

	        rs1 = vecobj_finish(&entries) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (entries) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return rs ;
}
/* end subroutine (procout) */


static int procmsfile(pip,nnp,elp,ofp,msfname,oflags,msflags)
PROGINFO	*pip ;
VECSTR		*nnp ;
VECOBJ		*elp ;
void		*ofp ;
cchar		msfname[] ;
int		oflags ;
{
	LOCINFO		*lip = pip->lip ;
	MSFILE		ms ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = msfile_open(&ms,msfname,oflags,0666)) >= 0) {
	    MSFILE_ENT	e, *ep ;
	    int		i ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_msinfo: specified nodes\n") ;
#endif

/* loop through any specified nodes */

	    if (! lip->f.all) {
	        cchar	*cp ;

	        for (i = 0 ; vecstr_get(nnp,i,&cp) >= 0 ; i += 1) {

	            c += 1 ;
	            rs = procnode(pip,elp,ofp,&ms,msflags,cp) ;

	            if (rs < 0) break ;
	        } /* end for (looping through positional arguments) */

	    } /* end if (processing node entries) */

	    if ((rs >= 0) && (c == 0)) {
	        MSFILE_CUR	cur ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msinfo: all nodes\n") ;
#endif

	        if ((rs = msfile_curbegin(&ms,&cur)) >= 0) {

	            i = 0 ;
	            while ((rs >= 0) && (i < 4)) {

	                rs1 = msfile_enum(&ms,&cur,&e) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_msinfo: msfile_enum() rs=%d\n",rs) ;
	                    if (rs1 >= 0)
	                        debugmse(&e) ;
	                }
#endif

	                if (rs1 == SR_NOTFOUND) break ;
	                rs = rs1 ;
	                if (rs < 0)
	                    break ;

	                c += 1 ;
	                if (e.flags & MSFLAG_MDISABLED)
	                    continue ;

	                if (e.nodename[0] == '\0')
	                    continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_msinfo: nodename=%s\n",
	                        e.nodename) ;
#endif

	                rs = vecobj_add(elp,&e) ;
	                i += 1 ;
	                if (i >= MAXNODES) break ;

	                if (rs < 0) break ;
	            } /* end while */

	            rs1 = msfile_curend(&ms,&cur) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (cursor) */

	    } /* end if (all nodes) */

/* zero out nodes if requested */

	    if ((rs >= 0) && (lip->f.zerospeed || lip->f.zeroentry)) {
	        MSFILE_ENT	e, ez ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msinfo: zeroing speed\n") ;
#endif

	        if (lip->f.zeroentry) {
	            memset(&ez,0,sizeof(MSFILE_ENT)) ;
		}

	        for (i = 0 ; vecobj_get(elp,i,&ep) >= 0 ; i += 1) {
	            if (ep == NULL) continue ;

	            e = *ep ;
	            e.speed = 0 ;
	            if (lip->f.zerospeed) {
	                rs = msfile_update(&ms,pip->daytime,&e) ;

	            } else if (lip->f.zeroentry) {
	                rs = msfile_write(&ms,pip->daytime,
	                    e.nodename,-1,&ez) ;
		    }

	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (zeroing speed element) */

	    rs1 = msfile_close(&ms) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msfile) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsfile) */


static int procnode(pip,elp,ofp,msp,msflags,name)
PROGINFO	*pip ;
vecobj		*elp ;
SHIO		*ofp ;
MSFILE		*msp ;
int		msflags ;
cchar		name[] ;
{
	LOCINFO		*lip = pip->lip ;
	MSFILE_ENT	e ;
	int		rs ;

	if (strcmp(name,"-") == 0) {
	    rs = msfile_match(msp,pip->daytime, pip->nodename,-1,&e) ;
	} else if (strcmp(name,"+") == 0) {
	    rs = msfile_best(msp,pip->daytime, msflags,&e) ;
	} else {
	    rs = msfile_match(msp,pip->daytime, name,-1,&e) ;
	}

	if (rs >= 0) {

	    rs = vecobj_add(elp,&e) ;

	} else if ((rs != SR_NOANODE) && 
	    (! pip->f.nodeonly)) {

	    if (lip->f.age) {
	        shio_printf(ofp,"\n") ;
	    } else {
	        shio_printf(ofp,"%-14s *NA*\n",name) ;
	    }

	    if (strcmp(name,"+") == 0)
	        rs = SR_OK ;

	} /* end if */

	return rs ;
}
/* end subroutine (procnode) */


static int printout(pip,ofp,elp)
PROGINFO	*pip ;
SHIO		*ofp ;
VECOBJ		*elp ;
{
	LOCINFO		*lip = pip->lip ;
	MSFILE_ENT	*ep ;
	int		rs = SR_OK ;
	int		i, j ;
	int		wlen = 0 ;

	if ((! lip->f.age) && (! lip->f.geekout) && (! pip->f.nodeonly)) {
	    if (! lip->f.nh)
	        shio_printf(ofp,
	            "NODE           SPEED NCPU"
	            "  1m   5m  15m  NPROC   PMT    PMA MU  UPDATED\n") ;
	}

	if (lip->f.age)
	    pip->daytime = time(NULL) ;

	j = 0 ;
	for (i = 0 ; vecobj_get(elp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {

	        if (lip->f.age) {
	            rs = printnodeage(pip,ofp,ep) ;
	        } else {
	            rs = printnode(pip,ofp,ep) ;
	        }

	        if ((lip->ndisplay > 0) && (++j >= lip->ndisplay)) break ;

	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printout) */


static int printnodeage(pip,ofp,ep)
PROGINFO	*pip ;
void		*ofp ;
MSFILE_ENT	*ep ;
{
	ulong		age ;
	int		rs ;
	int		wl = 0 ;

	if (ep == NULL) return SR_FAULT ;

	age = pip->daytime - ep->utime ;
	rs = shio_printf(ofp, "%lu\n",age) ;
	wl += rs ;

	return (rs >= 0) ? wl : rs ;
}
/* end subroutine (printnodeage) */


static int printnode(pip,ofp,ep)
PROGINFO	*pip ;
void		*ofp ;
MSFILE_ENT	*ep ;
{
	LOCINFO		*lip = pip->lip ;
	time_t		t ;
	int		rs = SR_OK ;
	int		mu ;
	int		wl = 0 ;
	cchar		*fmt ;
	char		nodebuf[NODENAMELEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (ep == NULL) return SR_FAULT ;

	if (lip->f.geekout > 0) {

	    wl += shio_printf(ofp,"%s\n",ep->nodename) ;

	    wl += shio_printf(ofp,"  la=%08x:%08x:%08x\n",
	        ep->la[0],ep->la[1],ep->la[2]) ;

	    wl += shio_printf(ofp,"  ncpu=%u speed=%u nproc=%u\n",
	        ep->ncpu,ep->speed,ep->nproc) ;

	    wl += shio_printf(ofp,"  pmtotal=%u pmavail=%u\n",
	        ep->pmtotal,ep->pmavail) ;

	    t = (time_t) ep->atime ;
	    wl += shio_printf(ofp,"  access=%s\n",
	        timestr_logz(t,timebuf)) ;

	    t = (time_t) ep->utime ;
	    wl += shio_printf(ofp,"  update=%s\n",
	        timestr_logz(t,timebuf)) ;

	} else if (pip->f.nodeonly) {

	    wl += shio_printf(ofp,"%s\n",ep->nodename) ;

	} else {

#if	CF_FLOAT
	    double	f1, f5, f15 ;
#endif

	    strwcpy(nodebuf,ep->nodename,MIN(NODENAMELEN,14)) ;

	    t = (time_t) ep->utime ;
	    mu = calcmu(ep->pmtotal,ep->pmavail) ;

#if	CF_FLOAT
	    f1 = ((double) ep->la[0]) / FSCALE ;
	    f5 = ((double) ep->la[1]) / FSCALE ;
	    f15 = ((double) ep->la[2]) / FSCALE ;

	    MAXOUT(f1) ;

	    MAXOUT(f5) ;

	    MAXOUT(f15) ;

	    fmt = "%-14s %5u %4u %4.1f %4.1f %4.1f %4u %6u %6u %2u%% %s\n" ;
	    wl += shio_printf(ofp,fmt,
	        nodebuf,
	        MIN(ep->speed,99999),
	        MIN(ep->ncpu,9999),
	        f1,f5,f15,
	        MIN(ep->nproc,9999),
	        MIN(ep->pmtotal,999999),
	        MIN(ep->pmavail,999999),
	        mu,
	        timestr_log(t,timebuf)) ;

#else /* CF_FLOAT */
	    {
	        struct lav	la1, la5, la15 ;

	        mklav(&la1,ep->la[0]) ;

	        mklav(&la5,ep->la[1]) ;

	        mklav(&la15,ep->la[2]) ;

		fmt = "%-14s %5u %4u %2u.%1u %2u.%1u %2u.%1u %4u "
			"%6u %6u %2u%% %s\n" ;
	        wl += shio_printf(ofp,fmt,
	            nodebuf,
	            MIN(ep->speed,99999),
	            MIN(ep->ncpu,9999),
	            la1.i,la1.f,
	            la5.i,la5.f,
	            la15.i,la15.f,
	            MIN(ep->nproc,9999),
	            MIN(ep->pmtotal,999999),
	            MIN(ep->pmavail,999999),
	            mu,
	            timestr_log(t,timebuf)) ;

	    } /* end block */
#endif /* CF_FLOAT */

	} /* end if (processing mode) */

	return (rs >= 0) ? wl : rs ;
}
/* end subroutine (printnode) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
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


/* calculate memory usage */
int calcmu(uint t,uint a)
{
	int		mu = 0 ;
	if (t > 0) {
	    uint	n100 = ((t - a) * 100) ;
	    mu = (n100 / t) ;
	}
	return mu ;
}
/* end subroutine (calcmu) */


#if	(CF_FLOAT == 0)

static int mklav(struct lav *lp,uint v)
{
	uint		vint, vfract, vround ;
	uint		mask ;

	mask = (FSCALE - 1) ;
	vfract = v & mask ;
	vint = v / FSCALE ;

	vfract *= 1000 ;
	vfract /= FSCALE ;

	vround = vfract % 100 ;
	if (vround >= 45) {
	    vfract += 100 ;
	    if (vfract >= 1000) {
	        vfract = 0 ;
	        vint += 1 ;
	    }
	}

	vfract /= 100 ;
	if (vint > 99) {
	    vint = 99 ;
	    vfract = 9 ;
	}

	lp->i = vint ;
	lp->f = vfract ;
	return 0 ;
}
/* end subroutine (mklav) */

#endif /* (CF_FLOAT == 0) */


static int cmp_utime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e2p->utime - e1p->utime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_utime) */


static int cmpr_utime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e1p->utime - e2p->utime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_utime) */


static int cmp_stime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e2p->stime - e1p->stime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_stime) */


static int cmpr_stime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e1p->stime - e2p->stime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_stime) */


static int cmp_dtime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e2p->dtime - e1p->dtime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_dtime) */


static int cmpr_dtime(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e1p->dtime - e2p->dtime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_dtime) */


static int cmp_la1m(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e2p->la[0] - e1p->la[0]) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_la1m) */


static int cmpr_la1m(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e1p->la[0] - e2p->la[0]) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;

}
/* end subroutine (cmpr_la1m) */


static int cmp_speed(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e2p->speed - e1p->speed) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_speed) */


static int cmpr_speed(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
		    rc = (e1p->speed - e2p->speed) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_speed) */


static int cmp_pmt(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e2p->pmtotal - e1p->pmtotal) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_pmt) */


static int cmpr_pmt(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e1p->pmtotal - e2p->pmtotal) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_pmt) */


static int cmp_pma(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e2p->pmavail - e1p->pmavail) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_pma) */


static int cmpr_pma(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e1p->pmavail - e2p->pmavail) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_pma) */


static int cmp_ncpu(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MSFILE_ENT	*e1p = *e1pp ;
	            MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e2p->ncpu - e1p->ncpu) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmp_ncpu) */


static int cmpr_ncpu(void **e1pp,void **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
		    MSFILE_ENT	*e1p = *e1pp ;
		    MSFILE_ENT	*e2p = *e2pp ;
	            rc = (e1p->ncpu - e2p->ncpu) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpr_ncpu) */


#if	CF_DEBUGS || CF_DEBUG
static int debugmse(MSFILE_ENT *ep)
{
	debugprintf("b_msinfo/debugmse: node=%s ncpu=%d nproc=%d\n",
	    ep->nodename,ep->ncpu,ep->nproc) ;
	return SR_OK ;
}
/* end subroutine (debugmse) */
#endif /* CF_DEBUGS */


