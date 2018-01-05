/* b_calyear */

/* translate a bible number to its corresponding name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGSHORT	0		/* debug short */
#define	CF_DEBUGN	0		/* extra special debugging */
#define	CF_COOKIE	0		/* use cookie as separator */
#define	CF_DBFNAME	0		/* give a DB by default */
#define	CF_ALLOUT	0		/* enable "all-out" option */
#define	CF_LOCSETENT	0		/* compile |locinfo_setentry()| */
#define	CF_CONFIGCHECK	0		/* compile |config_check()| */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ calyear [<query(s)>|-a] [-n <name>]


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
#include	<userinfo.h>
#include	<field.h>
#include	<char.h>
#include	<vecstr.h>
#include	<wordfill.h>
#include	<linefold.h>
#include	<tmtime.h>
#include	<expcook.h>
#include	<paramfile.h>
#include	<dayspec.h>
#include	<prsetfname.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_calyear.h"
#include	"defs.h"
#include	"calyear.h"
#include	"manstr.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum length (?) */
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(6 * MAXPATHLEN)
#endif

#define	CITEBUFLEN	20
#define	COLBUFLEN	(COLUMNS + 10)

#define	NBLANKS		20

#define	PO_NAME		"name"

#define	TIMECOUNT	5

#ifndef	TO_TMTIME
#define	TO_TMTIME	5		/* time-out for TMTIME */
#endif

#define	NDEBFNAME	"calyear.deb"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_TODAY	struct locinfo_today

#define	CONFIG		struct config
#define	CONFIG_FL	struct config_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
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
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	ndigits(int,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_today {
	int		y ;
	int		m ;
	int		d ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		nitems:1 ;
	uint		monthname:1 ;
	uint		separate:1 ;
	uint		interactive:1 ;
	uint		citebreak:1 ;
	uint		caldirs:1 ;
	uint		calnames:1 ;
	uint		defnull:1 ;
	uint		tmtime:1 ;
	uint		allcals:1 ;
	uint		allents:1 ;
	uint		gmt:1 ;
	uint		apm:1 ;
} ;

struct locinfo {
	VECSTR		stores ;
	CALYEAR		holdb ;
	TMTIME		tm ;
	vecstr		caldirs ;
	vecstr		calnames ;
	const char	*homedname ;
	const char	*org ;
	const char	*name ;
	const char	*fullname ;
	void		*ofp ;
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	LOCINFO_TODAY	today ;
	time_t		ti_tmtime ;
	int		timecount ;
	int		linelen ;
	int		indent ;
	int		nitems ;
	int		count, max ;
	int		cout ;
	int		ncites ;
	int		year ;
} ;

struct config_flags {
	uint		a:1 ;		/* active */
	uint		params:1 ;	/* params open */
	uint		cooks:1 ;	/* expcooks open */
} ;

struct config {
	PROGINFO	*pip ;
	PARAMFILE	params ;
	EXPCOOK		cooks ;
	CONFIG_FL	f ;
} ;

static cchar	*months[] = {
	    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

#if	CF_CONFIGCHECK
static int	config_check(CONFIG *) ;
#endif /* CF_CONFIGCHECK */

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,const char *,int) ;
static int	procspec(PROGINFO *,const char *,int) ;
static int	procnames(PROGINFO *,PARAMOPT *) ;

static int	procval(PROGINFO *,int,int) ;
static int	procqueries(PROGINFO *,CALYEAR_CITE *,int) ;
static int	procquery(PROGINFO *,CALYEAR_CITE *) ;

static int	procoutcite(PROGINFO *,CALYEAR_CITE *,cchar *,int) ;
static int	procoutline(PROGINFO *,int,const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_userinfo(LOCINFO *) ;
static int	locinfo_loaddirs(LOCINFO *,const char *,int) ;
static int	locinfo_loaddir(LOCINFO *,const char *,int) ;
static int	locinfo_loadnames(LOCINFO *,const char *,int) ;
static int	locinfo_loadname(LOCINFO *,const char *,int) ;
static int	locinfo_getstuff(LOCINFO *,cchar ***,cchar ***) ;
static int	locinfo_defdayspec(LOCINFO *,DAYSPEC *) ;
static int	locinfo_today(LOCINFO *) ;
static int	locinfo_tmtime(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,const char **,const char *,int) ;
#endif

static int	config_start(CONFIG *,PROGINFO *,const char *) ;
static int	config_read(CONFIG *) ;
static int	config_filespec(CONFIG *,const char *) ;
static int	config_filedefs(CONFIG *) ;
static int	config_filefind(CONFIG *,vecstr *) ;
static int	config_fileadd(CONFIG *,const char *) ;
static int	config_params(CONFIG *) ;
static int	config_cooks(CONFIG *) ;
static int	config_cooksload(CONFIG *) ;
static int	config_finish(CONFIG *) ;

static int	vecstr_addourkeys(vecstr *,PROGINFO *) ;

static int	isNotGoodCite(int) ;


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
	akoname_gmt,
	akoname_overlast
} ;

static const char	*params[] = {
	"logsize",
	"logfile",
	"caldirs",
	"calnames",
	NULL
} ;

enum params {
	param_logsize,
	param_logfile,
	param_caldirs,
	param_calnames,
	param_overlast
} ;

static const char	*schedconf[] = {
	"%r/etc/%n/%n.%f",
	"%r/etc/%n/%f",
	"%r/etc/%n.%f",
	"%r/%n.%f",
	NULL
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


/* exported subroutines */


int b_calyear(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_calyear) */


int p_calyear(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_calyear) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	CONFIG		co ;
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
	int		i ;
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
	const char	*cfname = NULL ;
	const char	*dbfname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_calyear: starting DFD=%d\n",rs) ;
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

/* configuration file */
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

	                    case 'c':
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &aparams ;
	                                cchar		*po = PO_NAME ;
	                                lip->have.calnames = TRUE ;
	                                lip->final.calnames = TRUE ;
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

/* year */
	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
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
	                                rs = optvalue(avp,avl) ;
	                                lip->f.gmt = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* line-width (columns) */
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_calyear: arg-loop-exit rs=%d\n",rs) ;
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
	    debugprintf("b_calyear: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* argument processing */

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

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

	if (dbfname == NULL) dbfname = getourenv(envv,VARDBNAME) ;

#if	CF_DBFNAME
	if (dbfname == NULL) dbfname = DBFNAME ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_calyear: dbfname=%s\n",
	        ((dbfname != NULL) ? dbfname : "NULL")) ;
#endif

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

	if (lip->nitems < 1) lip->nitems = 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_calyear: linelen=%u\n",lip->linelen) ;
#endif

/* identification */

	if (rs >= 0) {
	    rs = locinfo_userinfo(lip) ;
	}

/* calendar directories */

	if ((rs >= 0) && (! lip->final.caldirs)) {
	    if ((cp = getourenv(envv,VARCALDNAMES)) != NULL) {
	        lip->final.caldirs = TRUE ;
	        lip->have.caldirs = TRUE ;
	        rs = locinfo_loaddirs(lip,cp,-1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_calyear: locinfo_loaddirs() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (pip->debuglevel > 0)) {
	            shio_printf(pip->efp,"%s: ENV caldir\n",
	                pip->progname) ;
	        }

	    } /* end if */
	} /* end if (cal-dirs from environment) */

/* calendar names */

	if ((rs >= 0) && lip->have.calnames) {
	    rs = procnames(pip,&aparams) ;
	} /* end if */

	if ((rs >= 0) && (! lip->final.calnames) && (! lip->f.allcals)) {
	    if ((cp = getourenv(envv,VARCALNAMES)) != NULL) {
	        lip->final.calnames = TRUE ;
	        lip->have.calnames = TRUE ;
	        rs = locinfo_loadnames(lip,cp,-1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_calyear: locinfo_loadnames() rs=%d\n",rs) ;
#endif

	    } /* end if */
	}

/* configuration */

	if (cfname == NULL) cfname = getourenv(envv,VARCFNAME) ;
	if (cfname == NULL) cfname = getourenv(envv,VARCONFIG) ;

	if ((pip->debuglevel > 0) && (cfname != NULL)) {
	    shio_printf(pip->efp,"%s: conf=%s\n",
	        pip->progname,cfname) ;
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    pip->config = &co ;
	    if ((rs = config_start(pip->config,pip,cfname)) >= 0) {
	        cchar	**caldirs = NULL ;
	        cchar	**calnames = NULL ;
	        pip->open.config = TRUE ;

/* more initialization */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("b_calyear: process\n") ;
	            shio_flush(pip->efp) ;
	        }
#endif

	        if (rs >= 0) {
	            rs = locinfo_getstuff(lip,&caldirs,&calnames) ;
	        }

	        if ((rs >= 0) && lip->f.allcals)
	            calnames = NULL ;

	        if ((rs >= 0) && (pip->debuglevel > 0)) {
	            if (caldirs != NULL) {
	                fmt = "%s: caldir=%s\n" ;
	                for (i = 0 ; caldirs[i] != NULL ; i += 1) {
	                    shio_printf(pip->efp,fmt,pn,caldirs[i]) ;
	                }
	            }
	            if (calnames != NULL) {
	                fmt = "%s: calname=%s\n" ;
	                for (i = 0 ; calnames[i] != NULL ; i += 1) {
	                    shio_printf(pip->efp,fmt,pn,calnames[i]) ;
	                }
	            }
	        } /* end if (debug printout) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("b_calyear: caldirs:\n") ;
	            if (caldirs != NULL) {
	                for (i = 0 ; caldirs[i] != NULL ; i += 1) {
	                    debugprintf("b_calyear: caldir%02u=%s\n",
	                        i,caldirs[i]) ;
	                }
	            }
	            debugprintf("b_calyear: calnames:\n") ;
	            if (calnames != NULL) {
	                for (i = 0 ; calnames[i] != NULL ; i += 1) {
	                    debugprintf("b_calyear: calname%02u=%s\n",
	                        i,calnames[i]) ;
	                }
	            }
	        }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("b_calyear: calyear_open()\n") ;
	            shio_flush(pip->efp) ;
	        }
#endif

	        if (rs >= 0) {
	            cchar	*pr = pip->pr ;
	            cchar	**cd = caldirs ;
	            cchar	**cn = calnames ;
	            if ((rs = calyear_open(&lip->holdb,pr,cd,cn)) >= 0) {
	                const int	ncalendars = rs ;
	                cchar		*of = ofname ;
	                cchar		*af = afname ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("b_calyear: calyear_open() rs=%d\n",
				rs) ;
#endif

	                if (lip->f.audit) {
	                    rs = calyear_audit(&lip->holdb) ;
	                    if (pip->debuglevel > 0) {
	                        fmt = "%s: DB audit (%d)\n" ;
	                        shio_printf(pip->efp,fmt,pn,rs) ;
	                    }
	                }

	                if (pip->debuglevel > 0) {
	                    fmt = "%s: calendars=%u\n" ;
	                    shio_printf(pip->efp,fmt,pn,ncalendars) ;
	                }

	                if (rs >= 0) {
	                    ARGINFO	*aip = &ainfo ;
	                    BITS	*bop = &pargs ;
	                    rs = procargs(pip,aip,bop,of,af) ;
	                }

	                rs1 = calyear_close(&lip->holdb) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                fmt = "%s: calendars inaccessible (%d)\n" ;
	                ex = EX_CONFIG ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	            } /* end if (calyear) */
	        } /* end if */

	        pip->open.config = FALSE ;
	        config_finish(pip->config) ;
	        pip->config = NULL ;
	    } else {
	        fmt = "%s: invalid configuration (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        ex = EX_OSERR ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    cchar	*fmt ;
	    if (! pip->f.quiet) {
	        fmt = "%s: could not perform function (%d)\n" ;
	        shio_printf(pip->efp,fmt,pip->progname,rs) ;
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
	    debugprintf("b_calyear: exiting ex=%u (%d)\n",ex,rs) ;
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

	fmt = "%s: USAGE> %s [<datespec(s)>] [-a] [-w <width>] [-<n>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-y <year>] [-c <calname(s)>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int vecstr_addourkeys(vecstr *svp,PROGINFO *pip)
{
	int		rs = SR_OK ;
	const char	*keys = "ren" ;
	const char	*kp ;
	const char	*vp ;
	char		kbuf[2] ;
	kbuf[1] = '\0' ;
	for (kp = keys ; kp[0] != '\0' ; kp += 1) {
	    int	kch = MKCHAR(kp[0]) ;
	    vp = NULL ;
	    switch (kch) {
	    case 'r':
	        vp = pip->pr ;
	        break ;
	    case 'e':
	        vp = "etc" ;
	        break ;
	    case 'n':
	        vp = pip->searchname ;
	        break ;
	    } /* end switch */
	    if (vp != NULL) {
	        kbuf[0] = kch ;
	        rs = vecstr_envadd(svp,kbuf,vp,-1) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (vecstr_addourkeys) */


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
	                        lip->indent = DEFINDENT ;
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
	                    if (! lip->final.defnull) {
	                        lip->have.defnull = TRUE ;
	                        lip->final.defnull = TRUE ;
	                        lip->f.defnull = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.defnull = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_gmt:
	                    if (! lip->final.gmt) {
	                        lip->have.gmt = TRUE ;
	                        lip->final.gmt = TRUE ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    int		f_a = FALSE ;
	    cchar	*cp ;
	    lip->ofp = ofp ;

	    if (! f_a) {
	        int	ai ;
	        int	f ;

	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                pan += 1 ;
	                rs = procspec(pip,cp,-1) ;
	                wlen += rs ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (looping through positional arguments) */

	    } /* end if (positional arguments) */

	    if ((rs >= 0) && (! f_a) &&
	        (afn != NULL) && (afn[0] != '\0')) {
	        SHIO		afile, *afp = &afile ;

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
	            fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (afile arguments) */

	    if ((rs >= 0) && (! f_a) && lip->f.apm) {
		const int	f_apm = lip->f.apm ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_calyear: +<n>\n") ;
#endif

	        pan += 1 ;
	        rs = procval(pip,f_apm,lip->nitems) ;
	        wlen += rs ;

	    } /* end if */

	    if ((rs >= 0) && (! f_a) && (lip->ncites == 0) && lip->f.defnull) {
		const int	aval = lip->nitems ;
	        int		edays = 0 ;

	        if (aval > 1) edays = (aval-1) ;
	        rs = procval(pip,TRUE,edays) ;
	        wlen += rs ;

	    } /* end if */

/* handle the case of printing out all entries */

#if	CF_ALLOUT
	    if ((rs >= 0) && f_a) {
	        CALYEAR_CUR	cur ;
	        CALYEAR_CITE	q ;	/* result */
	        int		cbl ;
	        char		cbuf[COMBUFLEN + 1] ;

	        if ((rs = calyear_curbegin(&lip->holdb,&cur)) >= 0) {
	            const int	clen = COMBUFLEN ;

	            while (rs >= 0) {

	                cbl = calyear_enum(&lip->holdb,&cur,&q,cbuf,clen) ;
	                if (cbl == SR_NOTFOUND) break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2)) {
	                    debugprintf("b_calyear: clb=%u\n",cbl) ;
	                    debugprintf("b_calyear: q=%u:%u\n",q.m,q.d) ;
	                }
#endif

	                rs = cbl ;
	                if (rs >= 0) {
	                    rs = procoutcite(pip,&q,cbuf,cbl) ;
			}

	            } /* end while */

	            calyear_curend(&lip->holdb,&cur) ;
	        } /* end if (cursor) */

	    } /* end if (printing all book titles) */
#endif /* CF_ALLOUT */

	    lip->ofp = NULL ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,PARAMOPT *app)
{
	LOCINFO		*lip = pip->lip ;
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		nl ;
	int		c = 0 ;
	const char	*po_name = PO_NAME ;
	const char	*np ;

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {

	    while (rs >= 0) {
	        nl = paramopt_fetch(app,po_name,&cur,&np) ;
	        if (nl == SR_NOTFOUND) break ;
	        rs = nl ;
	        if (rs < 0) break ;
	        if (nl == 0) continue ;

	        if ((nl == 1) && (np[0] == '+')) {
	            lip->f.allcals = TRUE ;
	            break ;
	        }

	        rs = locinfo_loadname(lip,np,nl) ;
	        c += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procnames: locinfo_loadname() rs=%d\n",rs) ;
#endif

	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (paramopt-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procnames: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


static int procspecs(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

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
	                c += rs ;
	            }
	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while */
	        uc_free(fbuf) ;
	    } /* end if (m-a) */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,cchar sp[],int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procspec: spec=>%t<\n",sp,sl) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: spec=%t\n",pn,sp,sl) ;
	}

	if ((sp[0] == '+') || (sp[0] == '-')) {
	    const int	f_plus = (sp[0] == '+') ;
	    int		v = (lip->nitems-1) ;

	    if (sl > 1) {
	        const int	cl = (sl - 1) ;
	        const char	*cp = (sp + 1) ;
	        rs = cfdeci(cp,cl,&v) ;
	    }

	    if (rs >= 0) {
	        rs = procval(pip,f_plus,v) ;
	        wlen += rs ;
	    }

	} else {
	    DAYSPEC	ds ;

	    if ((rs = dayspec_load(&ds,sp,sl)) >= 0) {
	        if ((rs = locinfo_defdayspec(lip,&ds)) >= 0) {
	            CALYEAR_CITE	q ;
	            memset(&q,0,sizeof(CALYEAR_CITE)) ;
	            q.y = ds.y ;
	            q.m = ds.m ;
	            q.d = ds.d ;
	            rs = procqueries(pip,&q,(lip->nitems-1)) ;
	            wlen += rs ;
	        } /* end if (locinfo_defdayspec) */
	    } else {
	        if (lip->f.interactive) {
	            const char	*fmt = "citation=>%t< invalid\n" ;
	            rs = shio_printf(lip->ofp,fmt,sp,sl) ;
	        }
	    } /* end if (dayspec) */

	} /* end if */

	if ((rs < 0) && isNotGoodCite(rs) && lip->f.interactive) {
	    fmt = "invalid citation=>%t< (%d)\n" ;
	    rs = shio_printf(lip->ofp,fmt,sp,sl,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procval(PROGINFO *pip,int f_plus,int edays)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = locinfo_tmtime(lip)) >= 0) {
	    TMTIME		tm = lip->tm ;
	    CALYEAR_CITE	q ;
	    memset(&q,0,sizeof(CALYEAR_CITE)) ;
	    q.m = tm.mon ;
	    q.d = tm.mday ;
	    rs = procqueries(pip,&q,edays) ;
	    wlen += rs ;
	} /* end if (locinfo_tmtime) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procval: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procval) */


static int procqueries(PROGINFO *pip,CALYEAR_CITE *qp,int edays)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_calyear/procqueries: q=%u:%u:%u\n",
	        qp->y,qp->m,qp->d) ;
	    debugprintf("b_calyear/procqueries: edays=%u\n",edays) ;
	}
#endif

	if (edays < 0) edays = 0 ;

	rs = procquery(pip,qp) ;
	wlen += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procqueries: procquery() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (edays > 0)) {
	    if ((rs = locinfo_tmtime(lip)) >= 0) {
	        TMTIME	tm = lip->tm ;
	        int	i ;

	        tm.mon = qp->m ;
	        tm.mday = qp->d ;
	        for (i = 0 ; i < edays ; i += 1) {
	            qp->d += 1 ;
	            if (qp->d > 27) {
	                time_t	dummy ;
	                tm.mday = qp->d ;
	                if ((rs = tmtime_adjtime(&tm,&dummy)) >= 0) {
	                    qp->m = (uchar) tm.mon ;
	                    qp->d = (uchar) tm.mday ;
	                }
	            }
	            if (rs >= 0) {
	                rs = procquery(pip,qp) ;
	                wlen += rs ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("b_calyear/procqueries: "
	                        "procquery() rs=%d\n",rs) ;
#endif
	            }
	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (locinfo-tmtime) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_calyear/procqueries: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueries) */


static int procquery(PROGINFO *pip,CALYEAR_CITE *qp)
{
	LOCINFO		*lip = pip->lip ;
	CALYEAR		*calp ;
	CALYEAR_CUR	cur ;
	CALYEAR_CITE	cc ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_calyear/procquery: q=%u:%u:%u\n",
	        qp->y,qp->m,qp->d) ;
#endif

	lip->ncites += 1 ;
	calp = &lip->holdb ;
	if ((rs = calyear_curbegin(calp,&cur)) >= 0) {
	    if ((rs = calyear_lookcite(calp,&cur,qp)) >= 0) {
	        const int	vlen = VBUFLEN ;
	        int		vl ;
	        char		vbuf[VBUFLEN + 1] ;
	        while (rs >= 0) {

	            vl = calyear_read(calp,&cur,&cc,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

	            if (pip->verboselevel > 0) {
	                rs = procoutcite(pip,&cc,vbuf,vl) ;
	                wlen += rs ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_calyear/procquery: "
	                        "procoutcite() rs=%d\n",rs) ;
#endif

	            }

	        } /* end while */
	    } /* end if (calyear_lookcite) */
	    rs1 = calyear_curend(calp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (calyear-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_calyear/procquery: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int procoutcite(PROGINFO *pip,CALYEAR_CITE *qp,cchar vbuf[],int vlen)
{
	LOCINFO		*lip = pip->lip ;
	WORDFILL	w ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		cbl ;
	int		line = 0 ;
	int		wlen = 0 ;
	const char	*fmt ;
	char		citebuf[CITEBUFLEN + 1] ;
	char		cbuf[COLBUFLEN + 1] ;

	if (vlen <= 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    LINEFOLD	liner ;
	    int		rs1, i ;
	    const char	*ccp ;
	    debugprintf("b_calyear/procoutcite: y=%y m=%u d=%u\n",
	        qp->y,qp->m,qp->d) ;
	    if ((rs1 = linefold_start(&liner,50,1,vbuf,vlen)) >= 0) {
	        for (i = 0 ; (cl = linefold_get(&liner,i,&ccp)) >= 0 ; i += 1)
	            debugprintf("b_calyear/procoutcite: >%t<\n",ccp,cl) ;
	        linefold_finish(&liner) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((qp->m >= 12) || (qp->d >= 32))
	    goto ret0 ;

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

	if (rs < 0)
	    goto ret1 ;

/* print out the text-data itself */

	line = 0 ;
	if ((rs = wordfill_start(&w,NULL,0)) >= 0) {

	    if (lip->f.monthname) {
	        const char	*mon = months[qp->m] ;
	        rs = bufprintf(citebuf,CITEBUFLEN,"%s-%02u",mon,qp->d) ;
	        cl = rs ;
	    } else {
	        const int	m = (qp->m + 1) ;
	        rs = bufprintf(citebuf,CITEBUFLEN,"%02u-%02u",m,qp->d) ;
	        cl = rs ;
	    }

	    if (rs >= 0) {
	        if (lip->f.citebreak) {
	            cbl = MIN((lip->linelen - lip->indent),COLBUFLEN) ;
	            rs = shio_printf(lip->ofp,"%t\n",citebuf,cl) ;
	            wlen += rs ;
	            line += 1 ;
	        } else {
	            cbl = MIN(lip->linelen,COLBUFLEN) ;
	            rs = wordfill_addword(&w,citebuf,cl) ;
	        } /* end if (monthname requested) */
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_calyear/procoutcite: cbl=%d\n",cbl) ;
#endif

	    if (rs >= 0) {
	        if ((rs = wordfill_addlines(&w,vbuf,vlen)) >= 0) {

	            while ((cl = wordfill_mklinefull(&w,cbuf,cbl)) > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_calyear/procoutcite: f cl=%u\n",cl) ;
	                    debugprintf("b_calyear/procoutcite: f line=>%t<¬\n",
	                        cbuf,strnlen(cbuf,MIN(cl,40))) ;
	                }
#endif

	                rs = procoutline(pip,line,cbuf,cl) ;
	                wlen += rs ;
	                if (rs < 0) break ;

	                line += 1 ;
	                cbl = MIN((lip->linelen - lip->indent),clen) ;

	            } /* end while (full lines) */

	            if (rs >= 0) {
	                if ((cl = wordfill_mklinepart(&w,cbuf,cbl)) > 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4)) {
	                        debugprintf("b_calyear/procoutcite: "
	                            "p cl=%u\n",cl) ;
	                        debugprintf("b_calyear/procoutcite: "
	                            "p line=>%t<¬ \n",
	                            cbuf,strnlen(cbuf,MIN(cl,40))) ;
	                    }
#endif /* CF_DEBUG */

	                    rs = procoutline(pip,line,cbuf,cl) ;
	                    wlen += rs ;

	                    line += 1 ;
	                    cbl = MIN((lip->linelen - lip->indent),clen) ;

	                } /* end if */
	            } /* end if (partial lines) */

	        } /* end if (wordfill_addlines) */
	    } /* end if (ok) */

	    wordfill_finish(&w) ;
	} /* end if (word-fill) */

/* done */
ret1:
ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutcite) */


static int procoutline(PROGINFO *pip,int line,cchar *lp,int ll)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		indent ;
	int		wlen = 0 ;

	indent = 0 ;
	if (lip->f.citebreak || (line > 0))
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
	lip->indent = DEFINDENT ;
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
	int		rs1 ;

	if (lip->open.calnames) {
	    lip->open.calnames = FALSE ;
	    rs1 = vecstr_finish(&lip->calnames) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.caldirs) {
	    lip->open.caldirs = FALSE ;
	    rs1 = vecstr_finish(&lip->caldirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

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


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	vecstr		*vsp = &lip->stores ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(vsp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(vsp,*epp) ;
	    }
	    if (vp != NULL) { 
		len = strnlen(vp,vl) ;
	        rs = vecstr_store(vsp,vp,len,epp) ;
	    } else {
		*epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(vsp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_userinfo(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	USERINFO	u ;
	int		rs ;
	int		rs1 ;

	if ((rs = userinfo_start(&u,NULL)) >= 0) {
	    const char	**vpp ;
	    const char	*vp ;
	    int		i ;
	    for (i = 0 ; i < 4 ; i += 1) {
	        vp = NULL ;
	        switch (i) {
	        case 0:
	            vpp = &pip->username ;
	            vp = u.username ;
	            break ;
	        case 1:
	            vpp = &pip->nodename ;
	            vp = u.nodename ;
	            break ;
	        case 2:
	            vpp = &pip->domainname ;
	            vp = u.domainname ;
	            break ;
	        case 3:
	            vpp = &lip->homedname ;
	            vp = u.homedname ;
	            break ;
	        } /* end switch */
	        if (vp != NULL) {
	            rs = proginfo_setentry(pip,vpp,vp,-1) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = userinfo_finish(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (userinfo) */

	return rs ;
}
/* end subroutine (locinfo_userinfo) */


static int locinfo_loaddirs(LOCINFO *lip,cchar dp[],int dl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dp != NULL) {
	    int		sl = (dl >= 0) ? dl : strlen(dp) ;
	    cchar	*sp = dp ;
	    cchar	*tp ;
	    while ((rs >= 0) && ((tp = strnpbrk(sp,sl,":;")) != NULL)) {
	        rs = locinfo_loaddir(lip,sp,(tp - sp)) ;
	        c += rs ;
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	    } /* end while */
	    if ((rs >= 0) && sp[0]) {
	        rs = locinfo_loaddir(lip,sp,sl) ;
	        c += rs ;
	    }
	} /* end if (non-NULL) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loaddirs) */


static int locinfo_loaddir(LOCINFO *lip,cchar *dp,int dl)
{
	int		rs ;
	int		c = 0 ;
	char		pathbuf[MAXPATHLEN + 1] ;

	if ((rs = pathclean(pathbuf,dp,dl)) > 0) {
	    int	pl = rs ;
	    if ((rs = perm(pathbuf,-1,-1,NULL,R_OK)) >= 0) {
	        if (! lip->open.caldirs) {
	            int opts = VECSTR_OCOMPACT ;
	            rs = vecstr_start(&lip->caldirs,0,opts) ;
	            lip->open.caldirs = (rs >= 0) ;
	        }
	        if (rs >= 0) {
	            rs = vecstr_adduniq(&lip->caldirs,pathbuf,pl) ;
	            if (rs < INT_MAX) c += rs ;
	        }
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loaddir) */


static int locinfo_loadnames(LOCINFO *lip,cchar dp[],int dl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dp != NULL) {
	    const char	*tp ;
	    const char	*sp = dp ;
	    int		sl = (dl >= 0) ? dl : strlen(dp) ;
	    while ((tp = strnpbrk(sp,sl,":;. \t\r\n")) != NULL) {
	        rs = locinfo_loadname(lip,sp,(tp - sp)) ;
	        c += rs ;
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && sp[0]) {
	        rs = locinfo_loadname(lip,sp,sl) ;
	        c += rs ;
	    }
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadnames) */


static int locinfo_loadname(LOCINFO *lip,cchar *dp,int dl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (! lip->open.calnames) {
	    int opts = VECSTR_OCOMPACT ;
	    rs = vecstr_start(&lip->calnames,1,opts) ;
	    lip->open.calnames = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(dp,dl,&cp)) > 0) {
	        rs = vecstr_adduniq(&lip->calnames,cp,cl) ;
	        if (rs < INT_MAX) c += rs ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadname) */


static int locinfo_getstuff(LOCINFO *lip,cchar ***cdpp,cchar ***cnpp)
{
	int		rs = SR_OK ;

	if ((rs >= 0) && (cdpp != NULL)) {
	    *cdpp = NULL ;
	    if (lip->open.caldirs)
	        rs = vecstr_getvec(&lip->caldirs,cdpp) ;
	}

	if ((rs >= 0) && (cnpp != NULL)) {
	    *cnpp = NULL ;
	    if (lip->open.calnames)
	        rs = vecstr_getvec(&lip->calnames,cnpp) ;
	}

	return rs ;
}
/* end subroutine (locinfo_getstuff) */


static int locinfo_defdayspec(LOCINFO *lip,DAYSPEC *dsp)
{
	int		rs = SR_OK ;
	if ((dsp->y <= 0) || (dsp->m <= 0) || (dsp->d <= 0)) {
	    if ((rs = locinfo_today(lip)) >= 0) {
	        if (dsp->y <= 0) dsp->y = lip->year ;
	        if (dsp->m < 0) dsp->m = lip->today.m ;
	        if (dsp->d <= 0) dsp->d = lip->today.d ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_defdayspec) */


/* get the current year (if necessary) */
static int locinfo_today(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->today.y == 0)) {
	    if ((rs = locinfo_tmtime(lip)) >= 0) {
	        lip->today.y = (lip->tm.year + TM_YEAR_BASE) ;
	        lip->today.m = lip->tm.mon ;
	        lip->today.d = lip->tm.mday ;
	    }
	}

	if ((rs >= 0) && (lip->year == 0)) {
	    if ((rs = locinfo_tmtime(lip)) >= 0) {
	        lip->year = (lip->tm.year + TM_YEAR_BASE) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_today) */


static int locinfo_tmtime(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		tc = TIMECOUNT ;
	int		to = TO_TMTIME ;

	if ((! lip->f.tmtime) || (lip->timecount++ >= tc)) {
	    if ((pip->daytime == 0) || (lip->timecount == tc)) {
	        pip->daytime = time(NULL) ;
	    }
	    lip->timecount = 0 ;
	    if ((! lip->f.tmtime) || ((pip->daytime - lip->ti_tmtime) >= to)) {
	        lip->ti_tmtime = pip->daytime ;
	        lip->f.tmtime = TRUE ;
	        if (lip->f.gmt) {
	            rs = tmtime_gmtime(&lip->tm,pip->daytime) ;
	        } else {
	            rs = tmtime_localtime(&lip->tm,pip->daytime) ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_tmtime) */


#ifdef	COMMENT

static int locinfo_setfolder(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	char		tmpdname[MAXPATHLEN + 1] ;

	if (lip == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	pip = lip->pip ;
	if (lip->final.folder)
	    goto ret0 ;

	if (vl < 0)
	    vl = strlen(vp) ;

	if (vl == 0)
	    goto ret0 ;

	if (vp[0] != '/') {
	    if (vl == 0) {
	        vp = FOLDERDNAME ;
	        vl = -1 ;
	    }
	    vl = mkpath2w(tmpdname,lip->homedname,vp,vl) ;
	    vp = tmpdname ;
	}

	if (vl > 0) {
	    int		f = TRUE ;
	    const char	*ep = lip->folder ;
	    if (ep != NULL) {
	        int	m = nleadstr(ep,vp,vl) ;
	        f = (m != vl) || (ep[m] != '\0') ;
	    }
	    if (f)
	        rs = locinfo_setentry(lip,&lip->folder,vp,vl) ;
	}

ret0:
	return rs ;
}
/* end subroutine (locinfo_setfolder) */


static int locinfo_setmbname(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	pip = lip->pip ;
	if (! lip->final.mbname) {
	    if (vl < 0) vl = strlen(vp) ;
	    if (vl > 0) {
	        int		f = TRUE ;
	        const char	*ep = lip->mbname ;
	        if (ep != NULL) {
	            int	m = nleadstr(ep,vp,vl) ;
	            f = (m != vl) || (ep[m] != '\0') ;
	        }
	        if (f) {
	            const char	*mb ;
	            rs = locinfo_setentry(lip,&lip->mbname,vp,vl) ;
	            if (rs >= 0) {
	                mb = lip->mbname ;
	                lip->f.copy = (mb[0] != '\0') && (mb[0] != '-') ;
	            }
	        }
	    } /* end if (non-zero) */
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_setmbname) */

#endif /* COMMENT */


/* configuration maintenance */
static int config_start(CONFIG *cfp,PROGINFO *pip,cchar *cfname)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (cfp == NULL) return SR_FAULT ;

	memset(cfp,0,sizeof(CONFIG)) ;
	cfp->pip = pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: cfname=%s\n",cfname) ;
#endif

	if ((cfname != NULL) && (cfname[0] != '\0')) {
	    rs = config_filespec(cfp,cfname) ;
	    c = rs ;
	} else {
	    rs = config_filedefs(cfp) ;
	    c = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: mid rs=%d c=%u\n",rs,c) ;
#endif

	if (rs >= 0) {
	    if (c > 0) {
	        if ((rs = config_cooks(cfp)) >= 0) {
	            rs = config_read(cfp) ;
	        }
	    }
	    if (rs < 0)
	        config_finish(cfp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_start) */


static int config_finish(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;
	if (cfp == NULL) return SR_FAULT ;

	if (cfp->f.cooks) {
	    rs1 = expcook_finish(&cfp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (cfp->f.params) {
	    rs1 = paramfile_close(&cfp->params) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_finish) */


static int config_filespec(CONFIG *cfp,const char *cfname)
{
	int		rs ;
	if ((rs = perm(cfname,-1,-1,NULL,R_OK)) >= 0) {
	    rs = config_fileadd(cfp,cfname) ;
	}
	return rs ;
}
/* end subroutine (config_filespec) */


static int config_filedefs(CONFIG *cfp)
{
	vecstr		files ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = vecstr_start(&files,2,0)) >= 0) {
	    if ((rs = config_filefind(cfp,&files)) > 0) {
	        int		i ;
	        const char	*fp ;
	        for (i = 0 ; vecstr_get(&files,i,&fp) >= 0 ; i += 1) {
	            if (fp == NULL) continue ;
	            c += 1 ;
	            rs = config_fileadd(cfp,fp) ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (filefind) */
	    rs1 = vecstr_finish(&files) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_filedefs) */


static int config_fileadd(CONFIG *cfp,const char *cfname)
{
	int		rs ;
	if ((rs = config_params(cfp)) >= 0) {
	    PARAMFILE	*pfp = &cfp->params ;
	    rs = paramfile_fileadd(pfp,cfname) ;
	}
	return rs ;
}
/* end subroutine (config_fileadd) */


static int config_params(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_OK ;
	if (! cfp->f.params) {
	    PARAMFILE	*pfp = &cfp->params ;
	    if ((rs = paramfile_open(pfp,pip->envv,NULL)) >= 0) {
	        cfp->f.params = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (config_params) */


static int config_cooks(CONFIG *cfp)
{
	int		rs = SR_OK ;
	if (! cfp->f.cooks) {
	    EXPCOOK	*ecp = &cfp->cooks ;
	    if ((rs = expcook_start(ecp)) >= 0) {
	        cfp->f.cooks = TRUE ;
	        rs = config_cooksload(cfp) ;
	        if (rs < 0) {
	            cfp->f.cooks = FALSE ;
	            expcook_finish(ecp) ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (config_cooks) */


static int config_filefind(CONFIG *cfp,vecstr *flp)
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO		*lip ;
	vecstr		sv ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	const char	*cfn = CONFIGFNAME ;

	lip = pip->lip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("config_filefind: ent\n") ;
	    debugprintf("config_filefind: pr=%s\n",pip->pr) ;
	    debugprintf("config_filefind: userhome=%s\n",lip->homedname) ;
	}
#endif

	if ((rs = vecstr_start(&sv,4,0)) >= 0) {
	    if ((rs = vecstr_addourkeys(&sv,pip)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        const char	**sc = schedconf ;
	        char		tbuf[MAXPATHLEN+1] ;

	        if ((rs = permsched(sc,&sv,tbuf,tlen,cfn,R_OK)) >= 0) {
	            c += 1 ;
	            rs = vecstr_add(flp,tbuf,rs) ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;

	        if (rs >= 0) {
	            if ((vecstr_envset(&sv,"r",lip->homedname,-1)) >= 0) {
	                if ((rs = permsched(sc,&sv,tbuf,tlen,cfn,R_OK)) >= 0) {
	                    c += 1 ;
	                    rs = vecstr_add(flp,tbuf,rs) ;
	                } else if (isNotPresent(rs))
	                    rs = SR_OK ;
	            }
	        } /* end if (ok) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_filefind: mid rs=%d c=%u\n",rs,c) ;
#endif

	        if ((rs >= 0) && (pip->debuglevel > 0) && (c > 0)) {
	            int		i ;
	            const char	*pn = pip->progname ;
	            const char	*fmt = "%s: conf=%s\n" ;
	            const char	*cp ;
	            for (i = 0 ; vecstr_get(flp,i,&cp) >= 0 ; i += 1) {
	                if (cp == NULL) continue ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_filefind: conf=%s\n",cp) ;
#endif
	                shio_printf(pip->efp,fmt,pn,cp) ;
	            }
	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_filefind: vecstr_finish()\n") ;
#endif

	    } /* end if (addourkeys) */
	    rs1 = vecstr_finish(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sv) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_filefind: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_filefind) */


static int config_cooksload(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	EXPCOOK		*ckp = &cfp->cooks ;
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;
	int		kch ;
	int		i ;
	int		vl ;
	const char	*ks = "PSNDHRU" ;
	const char	*vp ;
	char		hbuf[MAXHOSTNAMELEN + 1] ;
	char		kbuf[2] ;

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
	        } /* end block */
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
	        rs = expcook_add(ckp,kbuf,vp,vl) ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (config_cooksload) */


#if	CF_CONFIGCHECK
static int config_check(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_OK ;

	if (cfp == NULL)
	    return SR_FAULT ;

	if (cfp->f.p) {
	    if ((rs = paramfile_check(&cfp->params,pip->daytime)) > 0) {
	        rs = config_read(cfp) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (config_check) */
#endif /* CF_CONFIGCHECK */


static int config_read(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp = &cfp->params ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;
	int		rs1 ;
	int		v ;
	const char	*ccp ;

	if (cfp == NULL) return SR_FAULT ;

	lip = pip->lip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: ent\n") ;
#endif

	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    EXPCOOK	*ckp = &cfp->cooks ;
	    const int	plen = PBUFLEN ;
	    int		i ;
	    int		el ;
	    int		kl, vl ;
	    const char	*kp, *vp ;
	    cchar	*pr = pip->progname ;
	    char	pbuf[PBUFLEN + 1] ;
	    char	ebuf[EBUFLEN + 1] ;
	    char	tbuf[MAXPATHLEN + 1] ;

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs < 0) break ;

	        kp = pe.key ;
	        vp = pe.value ;
	        vl = pe.vlen ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_read: enum k=%t\n",kp,kl) ;
#endif

	        i = matpstr(params,2,kp,kl) ;

	        if (i < 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_read: v=%t\n",vp,vl) ;
#endif

	        ebuf[0] = '\0' ;
	        el = 0 ;
	        if (vl > 0) {
	            el = expcook_exp(ckp,0,ebuf,EBUFLEN,vp,vl) ;
	            if (el >= 0) ebuf[el] = '\0' ;
	        } /* end if */

	        if (el < 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("config_read: e=%t\n",ebuf,el) ;
	            if (i < param_overlast)
	                debugprintf("config_read: switch=%s(%u)\n",
	                    params[i],i) ;
	        }
#endif /* CF_DEBUG */

	        switch (i) {
	        case param_logsize:
	            if ((! pip->final.logsize) && (el > 0)) {
	                pip->have.logsize = TRUE ;
	                if ((rs1 = cfdecmfi(ebuf,el,&v)) >= 0)
	                    pip->logsize = v ;
	            }
	            break ;
	        case param_logfile:
	            if (! pip->final.lfname) {
	                pip->have.lfname = TRUE ;
	                rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                    LOGDNAME,pip->searchname,"") ;
	                ccp = pip->lfname ;
	                if ((ccp == NULL) || (strcmp(ccp,tbuf) != 0)) {
	                    const char	**vpp = &pip->lfname ;
	                    pip->changed.lfname = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                }
	            }
	            break ;
	        case param_caldirs:
	            if ((! lip->final.caldirs) && (el > 0)) {
	                rs = locinfo_loaddirs(lip,ebuf,el) ;
	            }
	            break ;
	        case param_calnames:
	            if ((! lip->final.calnames) && (el > 0)) {
	                rs = locinfo_loadnames(lip,ebuf,el) ;
	            }
	            break ;
	        } /* end switch */

	        if (rs < 0) break ;
	    } /* end while (enumerating) */

	    paramfile_curend(pfp,&cur) ;
	} /* end if (cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_read) */


static int isNotGoodCite(int rs)
{
	int		f = FALSE ;
	f = f || (rs == SR_NOTFOUND) ;
	f = f || isNotValid(rs) ;
	return f ;
}
/* end subroutine (isNotGoodCite) */


