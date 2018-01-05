/* b_holiday */

/* translate a bible number to its corresponding name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGN	0		/* extra (special) debugging */
#define	CF_COOKIE	0		/* use cookie as separator */
#define	CF_DBFNAME	0		/* give a DB by default */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ holiday <query(s)>


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
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<estrings.h>
#include	<field.h>
#include	<char.h>
#include	<wordfill.h>
#include	<tmtime.h>
#include	<dayspec.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_holiday.h"
#include	"defs.h"
#include	"holidayer.h"
#include	"manstr.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum length (?) */
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	80
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		256
#endif

#define	CITEBUFLEN	10
#define	COLBUFLEN	(COLUMNS + 10)

#define	NBLANKS		20

#define	PO_NAME		"name"

#define	NDEBFNAME	"holiday.nd"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	ndigits(int,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;

static int	isNotGoodCite(int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		nitems:1 ;
	uint		monthname:1 ;
	uint		separate:1 ;
	uint		interactive:1 ;
	uint		names:1 ;
	uint		citebreak:1 ;
	uint		tmtime:1 ;
	uint		holiday:1 ;
	uint		defnull:1 ;
	uint		defall:1 ;
	uint		allents:1 ;
	uint		gmt:1 ;
	uint		year:1 ;
	uint		apm:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	void		*ofp ;
	HOLIDAYER	holdb ;
	TMTIME		tm ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	int		linelen ;
	int		indent ;
	int		nitems ;
	int		count, max ;
	int		cout ;
	int		year ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,
			PARAMOPT *,cchar *,cchar *) ;
static int	procsome(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,cchar *) ;
static int	procspecs(PROGINFO *,cchar *,int) ;
static int	procspec(PROGINFO *,cchar *,int) ;

static int	procnow(PROGINFO *,int,int) ;

static int	procallents(PROGINFO *) ;
static int	procnames(PROGINFO *,PARAMOPT *) ;
static int	procname(PROGINFO *,const char *,int) ;
static int	procqueries(PROGINFO *,HOLIDAYER_CITE *,int) ;
static int	procquery(PROGINFO *,HOLIDAYER_CITE *) ;

static int	procoutents(PROGINFO *,HOLIDAYER_CITE *,cchar *,int) ;
static int	procoutenters(PROGINFO *,HOLIDAYER_CITE *,cchar *,int) ;
static int	procoutline(PROGINFO *,int,const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_tmtime(LOCINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"db",
	"monthname",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_db,
	argopt_monthname,
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
	"audit",
	"linelen",
	"indent",
	"monthname",
	"separate",
	"interactive",
	"citebreak",
	"default",
	"defnull",
	"defall",
	"gmt",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_monthname,
	akoname_separate,
	akoname_interactive,
	akoname_citebreak,
	akoname_default,
	akoname_defnull,
	akoname_defall,
	akoname_gmt,
	akoname_overlast
} ;

static const char	blanks[] = "                    " ;

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

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
} ;


/* exported subroutines */


int b_holiday(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_holiday) */


int p_holiday(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_holiday) */


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
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*dbfname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_holiday: starting DFD=%d\n",rs) ;
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

#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"b_holiday: a=>%t<\n",argp,argl) ;
#endif

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            if (f_optplus) lip->f.apm = TRUE ;
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

/* BibleBook-verse DB name */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_monthname:
	                    lip->have.monthname = TRUE ;
	                    lip->final.monthname = TRUE ;
	                    lip->f.monthname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.monthname = (rs > 0) ;
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

	                    case 'a':
	                        lip->f.allents = TRUE ;
	                        break ;

	                    case 'i':
	                        lip->have.interactive = TRUE ;
	                        lip->final.interactive = TRUE ;
	                        lip->f.interactive = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.interactive = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'n':
	                        lip->f.names = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &aparams ;
	                                cchar		*po = PO_NAME ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
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

/* line-width (columns) */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.linelen = TRUE ;
	                                lip->final.linelen = TRUE ;
	                                rs = optvalue(argp,argl) ;
	                                lip->linelen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* default year */
	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                lip->final.year = TRUE ;
	                                lip->have.year = TRUE ;
	                                lip->year = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* use GMT */
	                    case 'z':
	                        lip->final.gmt = TRUE ;
	                        lip->have.gmt = TRUE ;
	                        lip->f.gmt = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.gmt = (rs > 0) ;
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

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"b_holiday: while-out rs=%d\n",rs) ;
#endif

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
	    debugprintf("b_holiday: debuglevel=%u\n",pip->debuglevel) ;
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

/* argument processing */

	if ((lip->nitems <= 0) && (argval != NULL)) {
	    lip->have.nitems = TRUE ;
	    lip->final.nitems = TRUE ;
	    rs = optvalue(argval,-1) ;
	    lip->nitems = rs ;
	}

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (dbfname == NULL) dbfname = getourenv(envv,VARDBNAME) ;

#if	CF_DBFNAME
	if (dbfname == NULL) dbfname = DBFNAME ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_holiday: dbfname=%s\n",
	        ((dbfname != NULL) ? dbfname : "NULL")) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: dbfname=%s\n",
	        pip->progname,((dbfname != NULL) ? dbfname : "NULL")) ;
	}

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

	if ((lip->nitems < 1) && (! lip->have.nitems)) lip->nitems = 1 ;

	if (lip->nitems < 0) lip->nitems = 1 ;

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* process */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_holiday: holidayer_open()\n") ;
#endif

	if (rs >= 0) {
	    if ((rs = holidayer_open(&lip->holdb,pip->pr)) >= 0) {
	        const int	nverses = rs ;
	        const char	*pn = pip->progname ;
	        lip->open.holiday = TRUE ;

	        if (lip->f.audit) {
	            rs = holidayer_audit(&lip->holdb) ;
	            if (pip->debuglevel > 0) {
	                shio_printf(pip->efp,"%s: DB audit (%d)\n",pn,rs) ;
	            }
	        }

	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,"%s: entries=%u\n",pn,nverses) ;
	        }

	        if (rs >= 0) {
	            if ((rs = locinfo_tmtime(lip)) >= 0) {
	                PARAMOPT	*pop = &aparams ;
		        ARGINFO		*aip = &ainfo ;
		        BITS		*bop = &pargs ;
	                const char	*ofn = ofname ;
	                const char	*afn = afname ;
	                rs = process(pip,aip,bop,pop,ofn,afn) ;
	            } /* end if (locinfo_tmtime) */
	        } /* end if (ok) */

	        lip->open.holiday = FALSE ;
	        rs1 = holidayer_close(&lip->holdb) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: could not load DB (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        const char	*pn = pip->progname ;
	        const char	*fmt = "%s: could not perform function (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	    ex = mapex(mapexs,rs) ;
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
	    debugprintf("b_holiday: exiting ex=%u (%d)\n",ex,rs) ;
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
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

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
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<mon><day> ...] [-af <afile>] [+[<days>]]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-<n>] [-n <name(s)>] [-a] [-w <width>] [-db <db>]\n" ;
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
	                case akoname_linelen:
	                    if (! lip->final.linelen) {
	                        lip->have.linelen = TRUE ;
	                        lip->final.linelen = TRUE ;
	                        lip->f.linelen = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->linelen = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_indent:
	                    if (! lip->final.indent) {
	                        lip->have.indent = TRUE ;
	                        lip->final.indent = TRUE ;
	                        lip->f.indent = TRUE ;
	                        lip->indent = 1 ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->indent = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_monthname:
	                    if (! lip->final.monthname) {
	                        lip->have.monthname = TRUE ;
	                        lip->final.monthname = TRUE ;
	                        lip->f.monthname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.monthname = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_separate:
	                    if (! lip->final.separate) {
	                        lip->have.separate = TRUE ;
	                        lip->final.separate = TRUE ;
	                        lip->f.separate = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.separate = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_interactive:
	                    if (! lip->final.interactive) {
	                        lip->have.interactive = TRUE ;
	                        lip->final.interactive = TRUE ;
	                        lip->f.interactive = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.interactive = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_citebreak:
	                    if (! lip->final.citebreak) {
	                        lip->have.citebreak = TRUE ;
	                        lip->final.citebreak = TRUE ;
	                        lip->f.citebreak = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.citebreak = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_default:
	                case akoname_defnull:
	                    if (! lip->final.defnull) {
	                        lip->final.defnull = TRUE ;
	                        lip->have.defnull = TRUE ;
	                        lip->f.defnull = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.defnull = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_defall:
	                    if (! lip->final.defall) {
	                        lip->final.defall = TRUE ;
	                        lip->have.defall = TRUE ;
	                        lip->f.defall = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.defall = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_gmt:
	                    if (! lip->final.gmt) {
	                        lip->final.gmt = TRUE ;
	                        lip->have.gmt = TRUE ;
	                        lip->f.gmt = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.gmt = (rs > 0) ;
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


static int process(pip,aip,bop,pop,ofn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PARAMOPT	*pop ;
const char	*ofn ;
const char	*afn ;
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    lip->ofp = ofp ;

	    if (lip->f.allents) {
	        rs = procallents(pip) ;
	        wlen += rs ;
	    } else {
	        rs = procsome(pip,aip,bop,pop,afn) ;
	        wlen += rs ;
	    }

	    lip->ofp = NULL ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n",
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procsome(pip,aip,bop,pop,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PARAMOPT	*pop ;
const char	*afn ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		wlen = 0 ;
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
	                rs = procspec(pip,cp,-1) ;
	                wlen += rs ;
	            }
	        }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end for (looping through positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

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
	                    rs = procspecs(pip,cp,cl) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (afile arguments) */

	if ((rs >= 0) && lip->f.apm) {

	    pan += 1 ;
	    rs = procnow(pip,lip->f.apm,lip->nitems) ;
	    wlen += rs ;

	} /* end if */

	if ((rs >= 0) && (pan == 0) && lip->f.defnull) {
	    int	ndays = 1 ;

	    pan += 1 ;
	    if (lip->nitems > 1) ndays = lip->nitems ;
	    rs = procnow(pip,TRUE,ndays) ;
	    wlen += rs ;

	} /* end if */

	if ((rs >= 0) && (pan == 0) && lip->f.defall) {
	    rs = procallents(pip) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && lip->f.names) {
	    rs = procnames(pip,pop) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsome) */


static int procspecs(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;

	if (lip->f.interactive) lip->cout = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    const int	flen = sl ;
	    char	*fbuf ;
	    if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	        int	fl ;
	        while ((fl = field_sharg(&fsb,aterms,fbuf,flen)) >= 0) {
	            if (fl > 0) {
	                rs = procspec(pip,fbuf,fl) ;
	                wlen += rs ;
	            }
	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while */
	        uc_free(fbuf) ;
	    } /* end if (m-a) */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,cchar sp[],int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*fmt ;

	if (sp == NULL) return SR_FAULT ;

	if (sp[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_holiday/procspec: spec>%t<\n",sp,sl) ;
#endif

	if ((sp[0] == '+') || (sp[0] == '-')) {
	    const int	f_plus = (sp[0] == '+') ;
	    int		v = (lip->nitems-1) ;

	    if (sl > 1) {
	        const int	cl = (sl - 1) ;
	        const char	*cp = (sp + 1) ;
	        rs = cfdeci(cp,cl,&v) ;
	    }

	    if (rs >= 0) {
	        rs = procnow(pip,f_plus,v) ;
	        wlen += rs ;
	    }

	} else {

	    if ((rs = locinfo_tmtime(lip)) >= 0) {
	        DAYSPEC		ds ;
	        if ((rs = dayspec_load(&ds,sp,sl)) >= 0) {
	            HOLIDAYER_CITE	q ;
	            memset(&q,0,sizeof(HOLIDAYER_CITE)) ;
	            q.y = (ds.y >= 0) ? ds.y : lip->year ;
	            q.m = ds.m ;
	            q.d = ds.d ;
	            rs = procqueries(pip,&q,(lip->nitems-1)) ;
	            wlen += rs ;
	        } else {
	            if (lip->f.interactive) {
	                cchar	*fmt = "citation=>%t< invalid\n" ;
	                rs = shio_printf(lip->ofp,fmt,sp,sl) ;
	            }
	        } /* end if */
	    } /* end if (locinfo_tmtime) */

	} /* end if */

	if ((rs < 0) && isNotGoodCite(rs) && lip->f.interactive) {
	    fmt = "invalid citation=>%t< (%d)\n" ;
	    rs = shio_printf(lip->ofp,fmt,sp,sl,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_holiday/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


/* ARGSUSED */
static int procnow(PROGINFO *pip,int f_plus,int ndays)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procqueries: f_plus=%u ndays=%u\n",
	        f_plus,ndays) ;
#endif

	if ((rs = locinfo_tmtime(lip)) >= 0) {
	    TMTIME		tm = lip->tm ;
	    HOLIDAYER_CITE	q ;

	    if (pip->debuglevel > 0) {
	        const int	y = (tm.year + TM_YEAR_BASE) ;
	        const char	*pn = pip->progname ;
	        const char	*fmt = "%s: year=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,y) ;
	    }

	    memset(&q,0,sizeof(HOLIDAYER_CITE)) ;
	    q.y = (lip->have.year) ? lip->year : (tm.year+TM_YEAR_BASE) ;
	    q.m = tm.mon ;
	    q.d = tm.mday ;
	    rs = procqueries(pip,&q,ndays) ;
	    wlen += rs ;

	} /* end if (locinfo_tmtime) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnow) */


static int procallents(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	HOLIDAYER	*holp ;
	HOLIDAYER_CUR	cur ;
	HOLIDAYER_CITE	q ;	/* result */
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	holp = &lip->holdb ;
	if ((rs = holidayer_curbegin(holp,&cur)) >= 0) {
	    const int	y = lip->year ;
	    const int	clen = COMBUFLEN ;
	    int		cbl ;
	    char	cbuf[COMBUFLEN + 1] ;

	    while (rs >= 0) {
	        cbl = holidayer_enum(holp,&cur,&q,cbuf,clen,y) ;
	        if (cbl == SR_NOTFOUND) break ;
	        rs = cbl ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_holiday/procallents: clb=%u\n",cbl) ;
	            debugprintf("b_holiday/procallents: q=%u:%u\n",q.m,q.d) ;
	        }
#endif

	        if (rs >= 0) {
	            rs = procoutents(pip,&q,cbuf,cbl) ;
	            wlen += rs ;
	        }

	    } /* end while */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_holiday/procallents: while-out rs=%d\n",rs) ;
#endif

	    rs1 = holidayer_curend(holp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (holiday-cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_holiday/procallents: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procallents) */


static int procnames(PROGINFO *pip,PARAMOPT *app)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	    int		nl ;
	    cchar	*po = PO_NAME ;
	    cchar	*np ;

	    while (rs >= 0) {
	        nl = paramopt_fetch(app,po,&cur,&np) ;
	        if (nl == SR_NOTFOUND) break ;
	        rs = nl ;

	        if ((rs >= 0) && (nl > 0)) {
	            rs = procname(pip,np,nl) ;
	            wlen += rs ;
	        }

	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnames) */


static int procname(PROGINFO *pip,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	HOLIDAYER	*hop ;
	HOLIDAYER_CUR	cur ;
	HOLIDAYER_CITE	q ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	hop = &lip->holdb ;
	if ((rs = holidayer_curbegin(hop,&cur)) >= 0) {
	    const int	vlen = VBUFLEN ;
	    const int	y = lip->year ;
	    int		vl ;
	    char	vbuf[VBUFLEN + 1] ;

	    while (rs >= 0) {

	        vl = holidayer_fetchname(hop,y,np,nl,&cur,&q,vbuf,vlen) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;

	        if ((rs >= 0) && (vl > 0)) {
	            rs = procoutents(pip,&q,vbuf,vl) ;
	            wlen += rs ;
	        }

	    } /* end while */

	    rs1 = holidayer_curend(hop,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


static int procqueries(PROGINFO *pip,HOLIDAYER_CITE *qp,int ndays)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_calyear/procqueries: q=%u:%u\n",qp->m,qp->d) ;
	    debugprintf("b_calyear/procqueries: ndays=%u\n",ndays) ;
	}
#endif

	if (ndays < 0) ndays = 1 ;

	rs = procquery(pip,qp) ;
	wlen += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procqueries: procquery() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (ndays > 1)) {
	    if ((rs = locinfo_tmtime(lip)) >= 0) {
	        TMTIME	tm = lip->tm ;
	        int	i ;

	        tm.year = (qp->y-TM_YEAR_BASE) ;
	        tm.mon = qp->m ;
	        tm.mday = qp->d ;
	        for (i = 1 ; i < ndays ; i += 1) {
	            qp->d += 1 ;
	            if (qp->d > 27) {
	                time_t	dummy ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("b_calyear/procqueries: adj\n") ;
#endif
	                tm.mday = qp->d ;
	                if ((rs = tmtime_adjtime(&tm,&dummy)) >= 0) {
	                    qp->y = (ushort) (tm.year+TM_YEAR_BASE) ;
	                    qp->m = (uchar) tm.mon ;
	                    qp->d = (uchar) tm.mday ;
	                }
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("b_calyear/procqueries: "
	                        "tmtime_adjtime() rs=%d\n",rs) ;
#endif
	            }
	            if (rs >= 0) {
	                rs = procquery(pip,qp) ;
	                wlen += rs ;
	            }
	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (locinfo-tmtime) */
	} /* end if (extra days) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueries) */


static int procquery(PROGINFO *pip,HOLIDAYER_CITE *qp)
{
	LOCINFO		*lip = pip->lip ;
	HOLIDAYER_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_holiday/procquery: q=%u:%u\n",
	        qp->m,qp->d) ;
#endif

	if ((rs = holidayer_curbegin(&lip->holdb,&cur)) >= 0) {
	    const int	vlen = VBUFLEN ;
	    int		vl ;
	    char	vbuf[VBUFLEN + 1] ;

	    while (rs >= 0) {

	        vl = holidayer_fetchcite(&lip->holdb,qp,&cur,vbuf,vlen) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_holiday/procquery: "
	                "holidayer_fetchcite() rs=%d\n",
	                rs) ;
#endif

	        if ((rs >= 0) && (pip->verboselevel > 0)) {
	            rs = procoutents(pip,qp,vbuf,vl) ;
	            wlen += rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_holiday/procquery: "
	                "procoutents() rs=%d\n",rs) ;
#endif

	    } /* end while */

	    rs1 = holidayer_curend(&lip->holdb,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (holidays-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_holiday/procquery: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int procoutents(PROGINFO *pip,HOLIDAYER_CITE *qp,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_holiday/procoutents: m=%u d=%u\n",
	        qp->m,qp->d) ;
#endif

	if ((qp->m < 12) && (qp->d < 32)) {
	    const char	*fmt ;

/* print out any necessary separator */

#if	CF_COOKIE
	    fmt = "%\n" ;
#else
	    fmt = "\n" ;
#endif

	    if (lip->f.separate && (lip->cout++ > 0)) {
	        rs = shio_printf(lip->ofp,fmt) ;
	        wlen += rs ;
	    } /* end if (separator) */

/* print out the text-data itself */

	    if (rs >= 0) {
	        rs = procoutenters(pip,qp,sp,sl) ;
	        wlen += rs ;
	    } /* end if (ok) */

	} else
	    rs = SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_holiday/procoutents: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutents) */


static int procoutenters(PROGINFO *pip,HOLIDAYER_CITE *qp,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	WORDFILL	w ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = wordfill_start(&w,NULL,0)) >= 0) {
	    const int	clen = COLBUFLEN ;
	    int		cl ;
	    int		line = 0 ;
	    int		cbl ;
	    char	cbuf[COLBUFLEN + 1] ;
	    char	citebuf[CITEBUFLEN + 1] ;

	    cbl = MIN((lip->linelen - lip->indent),clen) ;

	    if (lip->f.monthname) {
	        const char	*mon = months[qp->m] ;
	        rs = bufprintf(citebuf,CITEBUFLEN,"%t-%02u",mon,3,qp->d) ;
	        cl = rs ;
	    } else {
	        const int	m = (qp->m + 1) ;
	        rs = bufprintf(citebuf,CITEBUFLEN,"%02u-%02u",m,qp->d) ;
	        cl = rs ;
	    }

	    if (rs >= 0) {
	        if (lip->f.citebreak) {
	            rs = shio_printf(lip->ofp,"%t\n",citebuf,cl) ;
	            wlen += rs ;
	            line += 1 ;
	        } else {
	            rs = wordfill_addword(&w,citebuf,cl) ;
	        } /* end if (monthname requested) */
	    }

	    if (rs >= 0) {
	        if ((rs = wordfill_addlines(&w,sp,sl)) >= 0) {

	            while ((cl = wordfill_mklinefull(&w,cbuf,cbl)) > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_holiday/procoutents: "
	                        "m line=>%t<¬\n",
	                        cbuf,strnlen(cbuf,MIN(cl,40))) ;
#endif

	                rs = procoutline(pip,line,cbuf,cl) ;
	                wlen += rs ;

	                line += 1 ;

	                if (rs < 0) break ;
	            } /* end while (full lines) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_holiday/procoutents: mid rs=%d\n",
	                    rs) ;
#endif

	            if (rs >= 0) {
	                if ((cl = wordfill_mklinepart(&w,cbuf,cbl)) > 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_holiday/procoutents: "
	                            "e line=>%t<¬\n",
	                            cbuf,strnlen(cbuf,MIN(cl,40))) ;
#endif

	                    rs = procoutline(pip,line,cbuf,cl) ;
	                    wlen += rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_holiday/procoutents: "
	                            "procoutline() rs=%d\n",rs) ;
#endif

	                    line += 1 ;
	                } /* end if (wordfill_mklinepart) */
	            } /* end if (partial lines) */

	        } /* end if (add-lines) */
	    } /* end if (ok) */

	    rs1 = wordfill_finish(&w) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (word-fill) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutenters) */


/* ARGSUSED */
static int procoutline(PROGINFO *pip,int line,cchar *lp,int ll)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		indent ;
	int		wlen = 0 ;

	indent = MIN(lip->indent,NBLANKS) ;
	rs = shio_printf(lip->ofp,"%t%t\n",blanks,indent,lp,ll) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->count = -1 ;
	lip->max = -1 ;
	lip->f.separate = FALSE ;
	lip->f.monthname = TRUE ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_deflinelen(LOCINFO *lip)
{
	const int	def = (DEFPRECISION + 2) ;
	int		rs = SR_OK ;
	if (lip->linelen < def) {
	    PROGINFO	*pip = lip->pip ;
	    cchar	*cp = NULL ;
	    if (isStrEmpty(cp,-1)) {
		cp = getourenv(pip->envv,VARLINELEN) ;
	    }
	    if (isStrEmpty(cp,-1)) {
		cp = getourenv(pip->envv,VARCOLUMNS) ;
	    }
	    if (hasnonwhite(cp,-1)) {
	        if ((rs = optvalue(cp,-1)) >= 0) {
		    if (rs >= def) {
	                lip->have.linelen = TRUE ;
	                lip->final.linelen = TRUE ;
	                lip->linelen = rs ;
		    }
	        }
	    }
	}
	if (lip->linelen < def ) lip->linelen = COLUMNS ;
	return rs ;
}
/* end subroutine (locinfo_deflinelen) */


static int locinfo_tmtime(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		y = lip->year ;

	if (! lip->f.tmtime) {
	    lip->f.tmtime = TRUE ;
	    if (pip->daytime == 0) pip->daytime = time(NULL) ;
	    if (lip->f.gmt) {
	        rs = tmtime_gmtime(&lip->tm,pip->daytime) ;
	    } else {
	        rs = tmtime_localtime(&lip->tm,pip->daytime) ;
	    }
	    if (lip->year <= 0) {
	        lip->year = (lip->tm.year+TM_YEAR_BASE) ;
	        y = lip->year ;
	    }
	} /* end if (needed) */

	return (rs >= 0) ? y : rs ;
}
/* end subroutine (locinfo_tmtime) */


static int isNotGoodCite(int rs)
{
	int		f = FALSE ;
	f = f || (rs == SR_NOTFOUND) ;
	f = f || isNotValid(rs) ;
	return f ;
}
/* end subroutine (isNotGoodCite) */


