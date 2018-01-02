/* b_webcounter */

/* this is a generic "main" module for the WEBCOUNTER program */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_DEBUG	0		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2000-03-02, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

	= 2017-10-02, David A­D­ Morano
	I update this to use the QUERYSTRING object.  Frist, using that object
	puts that "query string" parsing code all in one place, and Secondly,
	it handles the weirdo cases of query-string which we previously did
	not handle fully everywhere welse.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for small programs.


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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<expcook.h>
#include	<paramfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<mapstrint.h>
#include	<prsetfname.h>
#include	<querystring.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_webcounter.h"
#include	"defs.h"
#include	"proglog.h"
#include	"filecounts.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config
#define	CONFIG_MAGIC	0x23FFEEDD

#define	COUNTINFO	struct countinfo

#define	PO_OPTION	"option"


/* external subroutines */

extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		print:1 ;
	uint		add:1 ;
	uint		inc:1 ;
	uint		list:1 ;
	uint		hdr:1 ;
	uint		basedname:1 ;
	uint		dbname:1 ;
} ;

struct locinfo {
	VECSTR		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	PROGINFO	*pip ;
	vecstr		sufmaps ;
	vecstr		sufsubs ;
	const char	*basedname ;
	const char	*dbname ;
	void		*ofp ;
} ;

struct config {
	uint		magic ;
	PROGINFO	*pip ;
	PARAMOPT	*app ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;

struct countinfo {
	cchar		*dbp ;
	cchar		*cnp ;
	int		dbl ;
	int		cnl ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procname(PROGINFO *,MAPSTRINT *,cchar *,int) ;
static int	procdebugdb(PROGINFO *,cchar *) ;
static int	proclist(PROGINFO *) ;
static int	procreg(PROGINFO *,MAPSTRINT *) ;
static int	procreger(PROGINFO *,cchar *,int,int,MAPSTRINT *) ;
static int	procqs(PROGINFO *,cchar *) ;
static int	procqsget(PROGINFO *,QUERYSTRING *,COUNTINFO *) ;
static int	procqser(PROGINFO *pip,cchar *,cchar *,int,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *,const char *) ;
static int	procourconf_end(PROGINFO *) ;

static int	procout_begin(PROGINFO *,void *,const char *) ;
static int	procout_end(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_basedir(LOCINFO *,cchar *,int) ;

static int	config_start(CONFIG *,PROGINFO *,PARAMOPT *,cchar *) ;
static int	config_findfile(CONFIG *,char *,cchar *) ;
static int	config_cookbegin(CONFIG *) ;
static int	config_cookend(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_finish(CONFIG *) ;

#ifdef	COMMENT
static int	config_check(CONFIG *) ;
#endif /* COMMENT */

static int	mkourname(char *,const char *,const char *,const char *,int) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"cf",
	"lf",
	"db",
	"qs",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cf,
	argopt_lf,
	argopt_db,
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

static cchar	*akonames[] = {
	"print",
	"add",
	"inc",
	"base",
	"log",
	NULL
} ;

enum akonames {
	akoname_print,
	akoname_add,
	akoname_inc,
	akoname_base,
	akoname_log,
	akoname_overlast
} ;

static cchar	*csched[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static cchar	*cparams[] = {
	"basedir",
	"basedb",
	"logfile",
	"logsize",
	NULL
} ;

enum cparams {
	cparam_basedir,
	cparam_basedb,
	cparam_logfile,
	cparam_logsize,
	cparam_overlast
} ;

static cchar	*qkeys[] = {
	"db",
	"n",
	"c",
	NULL
} ;

enum qkeys {
	qkey_db,
	qkey_n,
	qkey_c,
	qkey_overlast
} ;


/* exported subroutines */


int b_webcounter(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_webcounter) */


int p_webcounter(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_webcounter) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cfname = NULL ;
	const char	*qs = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_webcounter: starting DFD=%d\n",rs) ;
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

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

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

/* verbose */
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

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
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

/* error file */
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

/* get an output file name other than using STDOUT! */
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

/* configuration file-name */
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log file-name */
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

/* database file */
	                case argopt_db:
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        lip->final.dbname = TRUE ;
	                        lip->dbname = cp ;
	                    }
	                    break ;

/* quert string */
	                case argopt_qs:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            qs = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                qs = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

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

/* quiet */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* add a counter to the DB if not already present */
	                    case 'a':
	                        lip->final.add = TRUE ;
	                        lip->have.add = TRUE ;
	                        lip->f.add = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.add = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->final.basedname = TRUE ;
	                                lip->basedname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* increment a counter */
	                    case 'i':
	                        lip->final.inc = TRUE ;
	                        lip->have.inc = TRUE ;
	                        lip->f.inc = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.inc = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* list */
	                    case 'l':
	                        lip->f.list = TRUE ;
	                        break ;

/* header */
	                    case 'h':
	                        lip->f.hdr = TRUE ;
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

/* print out counter values */
	                    case 'p':
	                        lip->final.print = TRUE ;
	                        lip->have.print = TRUE ;
	                        lip->f.print = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.print = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* other things */
	                    case 's':
	                        cp = NULL ;
	                        cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    cp = argp ;
	                                    cl = argl ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        if ((rs >= 0) && (cp != NULL)) {
	                            const char	*po = PO_OPTION ;
	                            rs = paramopt_loads(&aparams,po,cp,cl) ;
	                        }
	                        break ;

/* verbose output */
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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: f_print=%u\n",pn,lip->f.print) ;
	    shio_printf(pip->efp,"%s: f_add=%u\n",pn,lip->f.add) ;
	    shio_printf(pip->efp,"%s: f_inc=%u\n",pn,lip->f.inc) ;
	}

	if (qs == NULL) qs = getourenv(envv,VARQS) ;
	if (qs == NULL) qs = getourenv(envv,VARQUERYSTRING) ;

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (cfname == NULL) cfname = getourenv(envv,VARCFNAME) ;
	if (cfname == NULL) cfname = CONFIGFNAME ;

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (lip->basedname == NULL) {
	    lip->basedname = getourenv(envv,VARBASEDNAME) ;
	}

	if (lip->dbname == NULL) lip->dbname = getourenv(envv,VARDB) ;
	if (lip->dbname == NULL) lip->dbname = getourenv(envv,VARDBFNAME) ;

	if (rs >= 0) {
	    rs = locinfo_basedir(lip,pip->searchname,-1) ;
	}

/* argument information */

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
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = procourconf_begin(pip,&aparams,cfname)) >= 0) {
	                if ((rs = proglog_begin(pip,&u)) >= 0) {
	                    if ((rs = proguserlist_begin(pip)) >= 0) {
	                        SHIO	ofile ;
	                        cchar	*ofn = ofname ;
	                        cchar	*afn = afname ;
	                        if ((rs = procout_begin(pip,&ofile,ofn)) >= 0) {
	                            {
	                                ARGINFO	*aip = &ainfo ;
	                                BITS	*bop = &pargs ;
	                                rs = procargs(pip,aip,bop,afn,qs) ;
	                            }
	                            rs1 = procout_end(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (procout) */
	                        rs1 = proguserlist_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proguserlist) */
	                    rs1 = proglog_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (proglogfile) */
	                rs1 = procourconf_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procourconf) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pip->progname,rs) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    const char	*fmt = NULL ;
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOINPUT ;
	        fmt = "%s: database unavailable (%d)\n" ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        fmt = "%s: processing error (%d)\n" ;
	        break ;
	    } /* end switch */
	    if (! pip->f.quiet) {
	        if (fmt != NULL)
	            shio_printf(pip->efp,fmt,pip->progname,rs) ;
	    }
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d) \n",ex,rs) ;
#endif

/* we are out of here */
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
	    debugprintf("b_webcounter: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff comes here */
badarg:
	{
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_USAGE ;
	    fmt = "%s: invalid argument specified (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-db <db>] [<name(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-b <basedir>] [-p[=<b>]] [-l] [-a[=<b>]] [-i[=<b>]]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
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
	                case akoname_print:
	                    if (! lip->final.print) {
	                        lip->have.print = TRUE ;
	                        lip->final.print = TRUE ;
	                        lip->f.print = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.print = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_add:
	                    if (! lip->final.add) {
	                        lip->have.add = TRUE ;
	                        lip->final.add = TRUE ;
	                        lip->f.add = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.add = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_inc:
	                    if (! lip->final.inc) {
	                        lip->have.inc = TRUE ;
	                        lip->final.inc = TRUE ;
	                        lip->f.inc = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.inc = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_base:
	                    if (! lip->final.basedname) {
	                        if (vl > 0) {
	                            lip->final.basedname = TRUE ;
	                            rs = locinfo_basedir(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_log:
	                    if (! pip->final.logprog) {
	                        if (vl > 0) {
	                            pip->final.logprog = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
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
	    } /* end if (cursor) */
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
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
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
	            cchar	*nn = pip->nodename ;
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


static int procourconf_begin(PROGINFO *pip,PARAMOPT *app,cchar *cfname)
{
	const int	csize = sizeof(CONFIG) ;
	int		rs = SR_OK ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_webcounter/procourconf_begin: ent\n") ;
#endif

	if (cfname == NULL) cfname = CONFIGFNAME ;

	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    pip->config = csp ;
	    if ((rs = config_start(csp,pip,app,cfname)) >= 0) {
	        if ((rs = config_read(csp)) >= 0) {
	            rs = 1 ;
	        }
	        if (rs < 0)
	            config_finish(csp) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->config = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_webcounter/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_webcounter/procourconf_end: config=%u\n",
	        (pip->config != NULL)) ;
#endif

	if (pip->config != NULL) {
	    CONFIG	*csp = pip->config ;
	    rs1 = config_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}

	return rs ;
}
/* end subroutine (procourconf_end) */


static int procout_begin(PROGINFO *pip,void *ofp,cchar *ofname)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (lip->f.print || lip->f.list) {

	    if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	        ofname = STDOUTFNAME ;

	    if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {
	        const char	*fmt ;
	        fmt = "    count date			  counter-name" ;
	        lip->ofp = ofp ;

	        if (lip->f.hdr) {
	            shio_print(ofp,fmt,-1) ;
	        }

	    } else {
	        shio_printf(pip->efp,"%s: output unavailable (%d)\n",
	            pip->progname,rs) ;
	    }

	} /* end if (opening the output file) */

	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->ofp != NULL) {
	    rs1 = shio_close(lip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->ofp = NULL ;
	}

	return rs ;
}
/* end subroutine (procout_end) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *qs)
{
	LOCINFO		*lip = pip->lip ;
	MAPSTRINT	names ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = mapstrint_start(&names,10)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

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
	                    rs = procname(pip,&names,cp,-1) ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

/* process any names in the argument filename list file */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procname(pip,&names,cp,cl) ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

/* process regular requests */

	    if (rs >= 0) {
	        rs = procdebugdb(pip,qs) ;
	    }

	    if ((rs >= 0) && lip->f.list) {
	        rs = proclist(pip) ;
	    }

	    if ((rs >= 0) && (pan > 0)) {
	        rs = procreg(pip,&names) ;
	    }

	    if ((rs >= 0) && (qs != NULL)) {
	        rs = procqs(pip,qs) ;
	    }

	    rs1 = mapstrint_finish(&names) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (names) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procname(PROGINFO *pip,MAPSTRINT *nlp,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		v = -1 ;
	int		cl ;
	const char	*tp ;
	const char	*cp ;

	if (pip == NULL) return SR_FAULT ;
	if (nl < 0) nl = strnlen(np,nl) ;

	if ((tp = strnchr(np,nl,'=')) != NULL) {
	    nl = (tp-np) ;
	    cp = (tp+1) ;
	    cl = ((np+nl) - (tp+1)) ;
	    rs = cfdeci(cp,cl,&v) ;
	    if (v < 0) v = 0 ;
	}

	if (rs >= 0) {
	    rs = mapstrint_add(nlp,np,nl,v) ;
	}

	return rs ;
}
/* end subroutine (procname) */


static int procdebugdb(PROGINFO *pip,const char *qs)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (pip->debuglevel > 0) {
	    const char	*pn = pip->progname ;
	    if (lip->basedname != NULL) {
	        shio_printf(pip->efp,"%s: dbdir=%s\n",pn,lip->basedname) ;
	    }
	    if (lip->dbname != NULL) {
	        shio_printf(pip->efp,"%s: db=%s\n",pn,lip->dbname) ;
	    }
	    if (qs != NULL) {
	        shio_printf(pip->efp,"%s: qs=>%s<\n",pn,qs) ;
	    }
	} /* end if (debug) */
	if ((rs >= 0) && pip->open.logprog) {
	    LOGFILE	*lfp = &pip->lh ;
	    if (lip->basedname != NULL) {
	        logfile_printf(lfp,"dbdir=%s",lip->basedname) ;
	    }
	    if (lip->dbname != NULL) {
	        logfile_printf(lfp,"db=%s",lip->dbname) ;
	    }
	    if (qs != NULL) {
	        logfile_printf(lfp,"qs=>%s<",qs) ;
	    }
	}
	return rs ;
}
/* end subroutine (procdebugdb) */


static int proclist(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	*basedname ;
	const char	*dbfn ;
	char		tbuf[MAXPATHLEN + 1] ;

	basedname = lip->basedname ;
	dbfn = lip->dbname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclist: ent bd=%s dbfn=%s\n",basedname,dbfn) ;
#endif

	if ((dbfn != NULL) && (dbfn[0] != '\0')) {

	    if ((basedname != NULL) && (basedname[0] != '\0')) {
	        if (dbfn[0] != '/') {
	            rs = mkpath2(tbuf,basedname,dbfn) ;
	            dbfn = tbuf ;
	        }
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_webcounter/proclist: db=%s\n",dbfn) ;
#endif

	    if (rs >= 0) {
	        FILECOUNTS	fc ;
	        FILECOUNTS_CUR	cur ;
	        FILECOUNTS_INFO	fci ;
	        const mode_t	om = 0664 ;
	        const int		of = O_RDONLY ;
	        if ((rs = filecounts_open(&fc,dbfn,of,om)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/proclist: filecounts_open() rs=%d\n",rs) ;
#endif

	            if ((rs = filecounts_curbegin(&fc,&cur)) >= 0) {
	                if ((rs = filecounts_snap(&fc,&cur)) >= 0) {
	                    const int	nlen = REALNAMELEN ;
	                    int		nfl, tfl ;
	                    int		nl ;
	                    char	nbuf[REALNAMELEN + 1] ;
	                    char	tbuf[TIMEBUFLEN + 1] ;

	                    while (rs >= 0) {
	                        nl = filecounts_read(&fc,&cur,&fci,nbuf,nlen) ;
	                        if (nl == SR_NOTFOUND) break ;
	                        rs = nl ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(3)) {
	                            debugprintf("main/proclist: "
	                                "filecounts_read() rs=%d\n",nl) ;
	                            debugprintf("main/proclist: n=%t\n",nbuf,nl) ;
	                        }
#endif

	                        if (rs >= 0) {
	                            SHIO	*ofp = lip->ofp ;
	                            const int	v = fci.value ;
	                            cchar	*fmt = "%*u %-*s %t\n" ;
	                            c += 1 ;
	                            nfl = FILECOUNTS_NUMDIGITS ;
	                            tfl = FILECOUNTS_LOGZLEN ;
	                            timestr_logz(fci.utime,tbuf) ;
	                            rs = shio_printf(ofp,fmt,nfl,v,tfl,tbuf,nbuf,
	                                nl) ;
	                        }

	                    } /* end while (reading counters) */

	                } /* end if (took snapshot) */

	                rs1 = filecounts_curend(&fc,&cur) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (cursor) */

	            rs1 = filecounts_close(&fc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (filecounts) */
	    } /* end if (ok) */

	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclist: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclist) */


static int procreg(PROGINFO *pip,MAPSTRINT *nlp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		nc = 0 ;
	cchar		*dbfn ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	dbfn = lip->dbname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procreg: ent\n") ;
	    debugprintf("main/procreg: dbfn=%s\n",dbfn) ;
	}
#endif

	if ((dbfn != NULL) && (dbfn[0] != '\0')) {
	    char	tbuf[MAXPATHLEN + 1] ;

	    if (dbfn[0] != '/') {
	        rs = mkpath2(tbuf,lip->basedname,dbfn) ;
	        dbfn = tbuf ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procreg: rs=%d dbfn=%s\n",rs,dbfn) ;
#endif

	    if (rs >= 0) {
	        int	act = -1 ;

	        if (lip->f.add) act = 0 ;
	        if (lip->f.inc) act = 1 ;

/* preliminary (if we need to do anything at all) */

	        if ((nc = mapstrint_count(nlp)) > 0) {
	            if (pip->debuglevel > 0) {
	                fmt = "%s: processing regular\n" ;
	                shio_printf(pip->efp,fmt,pn) ;
	            }
/* try to open the DB */
	            rs = procreger(pip,dbfn,nc,act,nlp) ;
	        }

	    } /* end if (ok) */

	} else {
	    rs = SR_NOENT ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procreg: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? nc : rs ;
}
/* end subroutine (procreg) */


static int procreger(PROGINFO *pip,cchar *db,int nc,int ac,MAPSTRINT *nlp)
{
	LOCINFO		*lip = pip->lip ;
	FILECOUNTS	fc ;
	const mode_t	om = 0664 ;
	const int	of = (O_RDWR | O_CREAT) ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((rs = filecounts_open(&fc,db,of,om)) >= 0) {
	    SHIO		*ofp = lip->ofp ;
	    FILECOUNTS_N	*namelist = NULL ;
	    const int		nlsize = (nc + 1) * sizeof(FILECOUNTS_N) ;
	    if ((rs = uc_malloc(nlsize,&namelist)) >= 0) {
	        MAPSTRINT_CUR	cur ;
	        int		i = 0 ;

	        if ((rs = mapstrint_curbegin(nlp,&cur)) >= 0) {
	            int		v ;
	            cchar	*np ;
	            while (mapstrint_enum(nlp,&cur,&np,&v) >= 0) {
	                if (np != NULL) {
	                    if (i < nc) {
	                        namelist[i].name = np ;
	                        namelist[i].value = (v >= 0) ? v : ac ;
	                        i += 1 ;
	                    }
	                }
	            } /* end while */
	            rs1 = mapstrint_curend(nlp,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mapstrint-cursor) */

	        namelist[i].name = NULL ;
	        namelist[i].value = -1 ;

/* process entries */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            for (i = 0 ; namelist[i].name != NULL ; i += 1)
	                debugprintf("main/process: name=%s\n",
	                    namelist[i].name) ;
	        }
#endif

	        if (rs >= 0) {
	            rs = filecounts_process(&fc,namelist) ;
	        }

/* optional print result */

	        if ((rs >= 0) && lip->f.print) {
	            for (i = 0 ; i < nc ; i += 1) {
	                fmt = (namelist[i].value >= 0) ? "%u\n" : "\n" ;
	                rs = shio_printf(ofp,fmt,namelist[i].value) ;
	                if (rs < 0) break ;
	            } /* end for */
	        } else if ((rs < 0) && (! pip->f.quiet)) {
	            fmt = "%s: could not process names (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }

	        rs1 = uc_free(namelist) ;
	        if (rs >= 0) rs = rs1 ;
	        namelist = NULL ;
	    } /* end if (memory-allocations) */
	    rs1 = filecounts_close(&fc) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filecounts) */
	return (rs >= 0) ? nc : rs ;
}
/* end subroutine (procreger) */


static int procqs(PROGINFO *pip,cchar *qs)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procqs: ent qs=>%s<\n",qs) ;
	    debugprintf("main/procqs: bd=%s\n",lip->basedname) ;
	}
#endif

	if ((qs != NULL) && (qs[0] != '\0')) {
	    int		act = -1 ;

	    if (lip->f.add) act = 0 ;
	    if (lip->f.inc) act = 1 ;

	    if (rs >= 0) {
	        QUERYSTRING	ps ;
	        if ((rs = querystring_start(&ps,qs,-1)) >= 0) {
	            COUNTINFO	ci ;
	            if ((rs = procqsget(pip,&ps,&ci)) >= 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procqs: db=>%t< cn=>%t<\n", 
	                        ci.dbp,ci.dbl, ci.cnp,ci.cnl) ;
#endif

	                if ((ci.dbp != NULL) && (ci.cnp != NULL)) {
	    		    cchar	*basedname = lip->basedname ;
	    		    char	dbfname[MAXPATHLEN + 1] ;

	                    if (pip->debuglevel > 0) {
	                        fmt = "%s: processing query-string\n" ;
	                        shio_printf(pip->efp,fmt,pn) ;
	                    }

	                    rs = mkpath2w(dbfname,basedname,ci.dbp,ci.dbl) ;

	                    if (pip->debuglevel > 0) {
	                        fmt = "%s: qsdb=%s\n" ;
	                        shio_printf(pip->efp,fmt,pn,dbfname) ;
	                    }

	                    if (rs >= 0) {
	                        rs = procqser(pip,dbfname,ci.cnp,ci.cnl,act) ;
	                    }

	                } /* end if (have) */

	            } /* end if (procqsget) */

	            rs1 = querystring_finish(&ps) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (querystring_start) */
	    } /* end if (ok) */

	} /* end if (not empty) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procqs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procqs) */


static int procqsget(PROGINFO *pip,QUERYSTRING *qsp,COUNTINFO *cip)
{
	QUERYSTRING_CUR	cur ;
	int		rs ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = querystring_curbegin(qsp,&cur)) >= 0) {
	    cchar	*kp, *vp ;
	    while ((rs1 = querystring_enum(qsp,&cur,&kp,&vp)) >= 0) {
	        int		ki ;
	        int		vl = rs1 ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procqsget: "
			"enum() rs=%d kp=%s bp=%s\n",rs1,kp,vp) ;
#endif
	        if ((ki = matstr(qkeys,kp,-1)) >= 0) {
	            switch (ki) {
	            case qkey_db:
	                cip->dbp = vp ;
	                cip->dbl = vl ;
	                break ;
	            case qkey_n:
	            case qkey_c:
	                cip->cnp = vp ;
	                cip->cnl = vl ;
	                break ;
	            } /* end switch */
	        } /* end if (match) */
	    } /* end while */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = querystring_curend(qsp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (querystring-cur) */
	return rs ;
}
/* end subroutine (procqsget) */


static int procqser(PROGINFO *pip,cchar *dbfname,cchar *cnp,int cnl,int ac)
{
	LOCINFO		*lip = pip->lip ;
	FILECOUNTS	fc ;
	FILECOUNTS_N	*namelist = NULL ;
	const mode_t	om = 0664 ;
	const int	of = (O_RDWR | O_CREAT) ;
	int		rs ;
	int		rs1 ;
	int		nc = 1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((rs = filecounts_open(&fc,dbfname,of,om)) >= 0) {
	    const int	nlsize = (nc + 1) * sizeof(FILECOUNTS_N) ;
	    if ((rs = uc_malloc(nlsize,&namelist)) >= 0) {
	        int	n = 0 ;
	        char	cn[MAXNAMELEN + 1] ;

	        snwcpy(cn,MAXNAMELEN,cnp,cnl) ;

	        namelist[n].name = cn ;
	        namelist[n].value = ac ;
	        n += 1 ;

	        namelist[n].name = NULL ;
	        namelist[n].value = -1 ;

/* process entries */

	        rs = filecounts_process(&fc,namelist) ;

	        if ((rs >= 0) && lip->f.print) {
	            SHIO	*ofp = lip->ofp ;
	            int		i ;
	            for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	                fmt = (namelist[i].value >= 0) ? "%u\n" : "\n" ;
	                rs = shio_printf(ofp,fmt,namelist[i].value) ;
	            } /* end for */
	        } else if ((rs < 0) && (! pip->f.quiet)) {
	            fmt = "%s: could not process names (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }

	        rs1 = uc_free(namelist) ;
	        if (rs >= 0) rs = rs1 ;
	        namelist = NULL ;
	    } /* end if (memory allocation) */
	    rs1 = filecounts_close(&fc) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filecounts) */
	return rs ;
}
/* end subroutine (procqser) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->f.add = TRUE ;
	lip->f.inc = TRUE ;

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


static int locinfo_basedir(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->basedname == NULL) {
	    cchar	*inter = VDNAME ;
	    cchar	*pr = pip->pr ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(tbuf,pr,inter,vp,vl)) >= 0) {
	        const char	**vpp = &lip->basedname ;
	        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_basedir) */


static int config_start(CONFIG *csp,PROGINFO *pip,PARAMOPT *app,cchar *cfname)
{
	int		rs = SR_OK ;
	char		tbuf[MAXPATHLEN+1] = { 0 } ;

	if (csp == NULL) return SR_FAULT ;
	if (cfname == NULL) return SR_FAULT ;

	memset(csp,0,sizeof(CONFIG)) ;
	csp->pip = pip ;
	csp->app = app ;

	if (strchr(cfname,'/') == NULL) {
	    rs = config_findfile(csp,tbuf,cfname) ;
	    if (rs > 0) cfname = tbuf ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: mid rs=%d cfname=%s\n",rs,cfname) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: conf=%s\n",
	        pip->progname,cfname) ;
	}

	if (rs >= 0) {
	    const char	**envv = pip->envv ;
	    if ((rs = paramfile_open(&csp->p,envv,cfname)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = TRUE ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	if (rs >= 0) csp->magic = CONFIG_MAGIC ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d f=%u\n",rs,csp->f_p) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_findfile(CONFIG *csp,char *tbuf,cchar *cfname)
{
	PROGINFO	*pip = csp->pip ;
	VECSTR		sv ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

	tbuf[0] = '\0' ;
	if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	    const int	tlen = MAXPATHLEN ;

	    vecstr_envset(&sv,"p",pip->pr,-1) ;

	    vecstr_envset(&sv,"e","etc",-1) ;

	    vecstr_envset(&sv,"n",pip->searchname,-1) ;

	    rs = permsched(csched,&sv,tbuf,tlen,cfname,R_OK) ;
	    pl = rs ;

	    rs1 = vecstr_finish(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (finding file) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (config_findfile) */


static int config_cookbegin(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	int		rs ;

	if ((rs = expcook_start(&csp->cooks)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    const char	*ks = "PSNDHRU" ;
	    const char	*vp ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    char	kbuf[2] ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'P':
	            vp = pip->progname ;
	            break ;
	        case 'S':
	            vp = pip->searchname ;
	            break ;
	        case 'N':
	            vp = pip->nodename ;
	            break ;
	        case 'D':
	            vp = pip->domainname ;
	            break ;
	        case 'H':
	            {
	                const char	*nn = pip->nodename ;
	                const char	*dn = pip->domainname ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = pip->pr ;
	            break ;
	        case 'U':
	            vp = pip->username ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(&csp->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        csp->f_cooks = TRUE ;
	    } else {
	        expcook_finish(&csp->cooks) ;
	    }
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->f_cooks) {
	    csp->f_cooks = FALSE ;
	    rs1 = expcook_finish(&csp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


static int config_finish(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {

	    rs1 = config_cookend(csp) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


#ifdef	COMMENT
static int config_check(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {
	    time_t	dt = pip->daytime ;
	    if ((rs = paramfile_check(&csp->p,dt)) > 0)
	        rs = config_read(csp) ;
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* COMMENT */


static int config_read(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp = &csp->p ;
	PARAMFILE_CUR	cur ;
	const int	vlen = VBUFLEN ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		ml, vl, el ;
	int		v ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",csp->f_p) ;
#endif

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

	if (csp->f_p) {
	    cchar	*pr = pip->pr ;
	    for (i = 0 ; cparams[i] != NULL ; i += 1) {

	        if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	            while (rs >= 0) {
	                vl = paramfile_fetch(pfp,cparams[i],&cur,vbuf,vlen) ;
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
	                    el = expcook_exp(&csp->cooks,0,ebuf,elen,vbuf,vl) ;
	                    if (el >= 0) ebuf[el] = '\0' ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	                if (el > 0) {
	                    cchar	*sn = pip->searchname ;
	                    char	tbuf[MAXPATHLEN + 1] ;

	                    switch (i) {
	                    case cparam_logsize:
	                        if ((rs = cfdecmfi(ebuf,el,&v)) >= 0) {
	                            if (v >= 0) {
	                                switch (i) {
	                                case cparam_logsize:
	                                    pip->logsize = v ;
	                                    break ;
	                                } /* end switch */
	                            }
	                        } /* end if (valid number) */
	                        break ;
	                    case cparam_basedir:
	                    case cparam_basedb:
	                        if (! lip->final.basedname) {
	                            if (vl > 0) {
	                                lip->final.basedname = TRUE ;
	                                rs = locinfo_basedir(lip,ebuf,el) ;
	                            }
	                        }
	                        break ;
	                    case cparam_logfile:
	                        if (! pip->final.lfname) {
	                            cchar *lfn = pip->lfname ;
	                            cchar	*tfn = tbuf ;
	                            pip->final.lfname = TRUE ;
	                            pip->have.lfname = TRUE ;
	                            ml = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                                LOGCNAME,sn,"") ;
	                            if ((lfn == NULL) || 
	                                (strcmp(lfn,tfn) != 0)) {
	                                cchar	**vpp = &pip->lfname ;
	                                pip->changed.lfname = TRUE ;
	                                rs = proginfo_setentry(pip,vpp,
	                                    tbuf,ml) ;
	                            }
	                        }
	                        break ;
	                    } /* end switch */

	                } /* end if (got one) */

	            } /* end while (fetching) */

	            paramfile_curend(pfp,&cur) ;
	        } /* end if (cursor) */

	        if (rs < 0) break ;
	    } /* end for (parameters) */
	} /* end if (active) */

	return rs ;
}
/* end subroutine (config_read) */


static int mkourname(char *rbuf,cchar *pr,cchar *inter,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (strnchr(sp,sl,'/') != NULL) {
	    if (sp[0] != '/') {
	        rs = mkpath2w(rbuf,pr,sp,sl) ;
	    } else {
	        rs = mkpath1w(rbuf,sp,sl) ;
	    }
	} else {
	    rs = mkpath3w(rbuf,pr,inter,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (mkourname) */


