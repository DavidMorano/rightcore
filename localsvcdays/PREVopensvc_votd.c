/* opensvc_votds */

/* LOCAL facility open-service (votds) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_COOKIE	0		/* use cookie as separator */
#define	CF_BOOKMATCH	1		/* use 'biblebook_match(3dam)' */


/* revision history:

	= 2003-11-04, David A­D­ Morano

	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather
	I should have started this code by using the corresponding UUX
	dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a facility-open-service module.

	Synopsis:

	int opensvc_votds(pr,prn,of,om,argv,envv,to)
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
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<estrings.h>
#include	<field.h>
#include	<vecstr.h>
#include	<wordfill.h>
#include	<tmtime.h>
#include	<dayspec.h>
#include	<filebuf.h>
#include	<bfile.h>
#include	<biblebook.h>
#include	<biblepara.h>
#include	<bibleverse.h>
#include	<bvs.h>
#include	<bvsmk.h>
#include	<manstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_votds.h"
#include	"defs.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum bibleverse length (?) */
#endif

#ifndef	BVBUFLEN
#define	BVBUFLEN	1024		/* maximum bibleverse length (?) */
#endif

#ifndef	SPECBUFLEN
#define	SPECBUFLEN	40
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#define	BOOKBUFLEN	100
#define	COLBUFLEN	(COLUMNS + 10)

#define	NBLANKS		20

#define	TIMECOUNT	5

#ifndef	TO_MKWAIT
#define	TO_MKWAIT	(1 * 50)
#endif

#ifndef	TO_TMTIME
#define	TO_TMTIME	5		/* time-out for TMTIME */
#endif

#define	TO_MJD		5		/* time-out for MJD */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	NDFNAME		"bibleverse.nd"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpyopaque(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sicasesub(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	ndigits(int,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
extern int	getmjd(int,int,int) ;
extern int	msleep(int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	hasourmjd(const char *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* external variables */

extern const char	**environ ;


/* local structures */

struct subinfo_flags {
	uint		akopts:1 ;
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		nverses:1 ;
	uint		ndb:1 ;
	uint		bookname:1 ;
	uint		interactive:1 ;
	uint		separate:1 ;
	uint		tmtime:1 ;
	uint		bibleverse:1 ;
	uint		bvs:1 ;
	uint		bvsmk:1 ;
	uint		mjd:1 ;
	uint		defnull:1 ;
	uint		nitems:1 ;
	uint		para:1 ;
	uint		gmt:1 ;
	uint		all:1 ;
} ;

struct subinfo {
	TMTIME		tm ;		/* holds today's date, when set */
	BIBLEBOOK	ndb ;		/* bible-book-name DB */
	BIBLEVERSE	vdb ;
	BVS		sdb ;
	BVSMK		bsmk ;
	BIBLEPARA	pdb ;
	void		*ofp ;
	const char	**envv ;
	const char	*pr ;
	const char	*sn ;
	const char	*ndbname ;	/* name-db name */
	const char	*pdbname ;	/* paragraph-db name */
	const char	*vdbname ;	/* verse-db name */
	const char	*sdbname ;	/* struture-db name */
	SUBINFO_FL	have, f, changed, final ;
	SUBINFO_FL	open ;
	time_t		dt ;
	time_t		ti_tmtime ;
	time_t		ti_mjd ;
	int		timecount ;
	int		linelen ;
	int		indent ;
	int		nitems ;
	int		count, max, precision ;
	int		cout ;
	int		mjd ;
	int		ncites ;
	int		year ;
	int		qtype ;		/* query type */
	int		nverses ;
} ;

struct verlang {
	const char	*lang ;
	const char	*version ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char **) ;
static int	subinfo_setroot(SUBINFO *,const char *,const char *) ;
static int	subinfo_nlookup(struct subinfo *,int,char *,int) ;
static int	subinfo_match(struct subinfo *,const char *,int) ;
static int	subinfo_today(struct subinfo *) ;
static int	subinfo_defdayspec(SUBINFO *,DAYSPEC *) ;
static int	subinfo_year(struct subinfo *) ;
static int	subinfo_tmtime(struct subinfo *) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_mkmodquery(struct subinfo *,BIBLEVERSE_Q *,int) ;
static int	subinfo_bvs(struct subinfo *) ;
static int	subinfo_bvsbuild(struct subinfo *) ;
static int	subinfo_edays(struct subinfo *,BIBLEVERSE_Q *,int) ;
static int	subinfo_ispara(struct subinfo *,BIBLEVERSE_Q *) ;

static int	procopts(SUBINFO *,KEYOPT *) ;
static int	procdbs(SUBINFO *,ARGINFO *,BITS *,int,const char *,int,int) ;
static int	procargs(SUBINFO *,ARGINFO *,BITS *,int,const char *,int,int) ;
static int	procspecs(SUBINFO *,const char *,int) ;
static int	procspec(SUBINFO *,const char *,int) ;
static int	procall(SUBINFO *) ;
static int	procparse(SUBINFO *,BIBLEVERSE_Q *,
			const char *,int) ;
static int	proccites(SUBINFO *,BIBLEVERSE_Q *,int) ;
static int	proctoday(SUBINFO *,int,int) ;
static int	procmjds(SUBINFO *,int,int) ;
static int	procoutcite(SUBINFO *,BIBLEVERSE_Q *,int) ;
static int	procout(SUBINFO *,BIBLEVERSE_Q *,const char *,int) ;
static int	procoutline(SUBINFO *,int,const char *,int) ;

#if	CF_BOOKMATCH
static int	procbookmatch(SUBINFO *,const char *,int) ;
#else
static int	procbookget(SUBINFO *,const char *,int) ;
#endif

static int	sibook(const char *,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"sn",
	"af",
	"ndb",
	"pdb",
	"vdb",
	"sdb",
	"bookname",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ndb,
	argopt_pdb,
	argopt_vdb,
	argopt_sdb,
	argopt_bookname,
	argopt_overlast
} ;

static const char *akonames[] = {
	"audit",
	"linelen",
	"indent",
	"bookname",
	"interactive",
	"separate",
	"defnull",
	"default",
	"qtype",
	"atype",
	"para",
	"gmt",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_bookname,
	akoname_interactive,
	akoname_separate,
	akoname_defnull,
	akoname_default,
	akoname_qtype,
	akoname_atype,
	akoname_para,
	akoname_gmt,
	akoname_overlast
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

static const char	*qtypes[] = {
	"verses",
	"days",
	"mjds",
	NULL
} ;

enum qtypes {
	qtype_verse,
	qtype_day,
	qtype_mjd,
	qtype_overlast
} ;

#ifdef	COMMENT
static struct verlang	vers[] = {
	{ "english", "av" },
	{ "spanish", "rvv" },
	{ NULL, NULL }
} ;
#endif /* COMMENT */

static const char	blanks[NBLANKS+1] = "                    " ;


/* exported subroutines */


/* ARGSUSED */
int opensvc_votds(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO		si, *sip = &si ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argc = 0 ;
	int		rs, rs1 ;
	int		n ;
	int		argvalue = -1 ;
	int		v ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_apm = FALSE ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ndbname = NULL ;
	const char	*pdbname = NULL ;
	const char	*vdbname = NULL ;
	const char	*sdbname = NULL ;
	const char	*qtypestr = NULL ;
	const char	*cp ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

/* local information */

	rs = subinfo_start(sip,envv) ;
	if (rs < 0) goto badsubstart ;

	sip->linelen = 0 ;
	sip->count = -1 ;
	sip->max = -1 ;

	sip->f.separate = TRUE ;
	sip->f.bookname = TRUE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	sip->open.akopts = (rs >= 0) ;

	ai = 0 ;
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
	        const int ch = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ch)) {

	            if (f_optplus) f_apm = TRUE ;
	            argval = (argp+1) ;

	        } else if (ch == '-') {

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

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    }
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
	                    }
	                    break ;

/* BibleBook-name DB name */
	                case argopt_ndb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ndbname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ndbname = argp ;
	                    }
	                    break ;

/* paragraph-db name */
	                case argopt_pdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pdbname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pdbname = argp ;
	                    }
	                    break ;

/* BibleBook-verse DB name */
	                case argopt_vdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            vdbname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            vdbname = argp ;
	                    }
	                    break ;

/* structure-db name */
	                case argopt_sdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sdbname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sdbname = argp ;
	                    }
	                    break ;

	                case argopt_bookname:
	                    sip->have.bookname = TRUE ;
	                    sip->final.bookname = TRUE ;
	                    sip->f.bookname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            sip->f.bookname = (rs > 0) ;
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
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                        break ;

	                    case 'a':
	                        sip->f.all = TRUE ;
	                        break ;

/* type of argument-input */
	                    case 'i':
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            qtypestr = argp ;
	                        break ;

/* number of verses to print */
	                    case 'n':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            sip->have.nitems = TRUE ;
	                            sip->final.nitems = TRUE ;
	                            rs = optvalue(argp,argl) ;
	                            sip->nitems = rs ;
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

/* line-width (columns) */
	                    case 'w':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            sip->have.linelen = TRUE ;
	                            sip->final.linelen = TRUE ;
	                            rs = optvalue(argp,argl) ;
	                            sip->linelen = rs ;
	                        }
	                        break ;

/* default year */
	                    case 'y':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            sip->year = rs ;
	                        }
	                        break ;

/* use GMT */
	                    case 'z':
	                        sip->final.gmt = TRUE ;
	                        sip->f.gmt = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                sip->f.gmt = (rs > 0) ;
	                            }
	                        }
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

/* argument processing */

	rs = subinfo_setroot(sip,pr,sn) ;

#if	CF_DEBUGS
	debugprintf("opensvc_votds: subinfo_setroot() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&argvalue) ;
	}

	if ((sip->nitems <= 0) && (argvalue > 0)) {
	    sip->have.nitems = TRUE ;
	    sip->final.nitems = TRUE ;
	    sip->nitems = argvalue ;
	}

/* load up the environment options */

	if (rs >= 0)
	    rs = procopts(sip,&akopts) ;

/* argument defaults */

	if (afname == NULL) afname = getourenv(sip->envv,VARAFNAME) ;

/* verse-db name */

	if (ndbname == NULL) ndbname = getourenv(envv,VARNDB) ;

#ifdef	OPTIONAL
	if (ndbname == NULL) ndbname = NDBNAME ;
#endif

#if	CF_DEBUGS
	    debugprintf("opensvc_votds: ndbname=%s\n",
	        ((ndbname != NULL) ? ndbname : "NULL")) ;
#endif

	sip->ndbname = ndbname ;

/* verse-db name */

	if (vdbname == NULL) vdbname = getourenv(envv,VARVDB) ;
	if (vdbname == NULL) vdbname = VDBNAME ;

	sip->vdbname = vdbname ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds: vdbname=%s\n",
	        ((vdbname != NULL) ? vdbname : "NULL")) ;
#endif

/* structure-db name */

	if (sdbname == NULL) sdbname = getourenv(envv,VARSDB) ;
	if (sdbname == NULL) sdbname = SDBNAME ;

	sip->sdbname = sdbname ;

/* paragraph-db name */

	if (pdbname == NULL) pdbname = getourenv(envv,VARPDB) ;

/* no default */

	sip->pdbname = pdbname ;

/* type of argument-input */

	if ((rs >= 0) && (qtypestr != NULL)) {
	    v = matostr(qtypes,1,qtypestr,-1) ;
	    if (v < 0) rs = SR_INVALID ;
	    sip->qtype = v ;
	}

#if	CF_DEBUGS
	if (qtypestr != NULL)
	    debugprintf("opensvc_votds: qtype=%s(%u)\n",qtypestr,sip->qtype) ;
#endif

/* debugging */

	rs1 = (DEFPRECISION + 2) ;
	if ((rs >= 0) && (sip->linelen < rs1)) {
	    cp = getourenv(envv,VARLINELEN) ;
	    if (cp == NULL) cp = getourenv(envv,VARCOLUMNS) ;
	    if (cp != NULL) {
	        if (((rs = cfdeci(cp,-1,&n)) >= 0) && (n >= rs1)) {
	            sip->have.linelen = TRUE ;
	            sip->final.linelen = TRUE ;
	            sip->linelen = n ;
	        }
	    }
	}

	if (sip->linelen < rs1)
	    sip->linelen = COLUMNS ;

	if ((sip->nitems < 1) && (! sip->have.nitems))
	    sip->nitems = 1 ;

	if (sip->nitems < 0)
	    sip->nitems = 1 ;

	if (rs < 0) goto badarg ;

/* process */

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

#if	CF_DEBUGS
	debugprintf("opensvc_votds: pr=%s\n",sip->pr) ;
	debugprintf("opensvc_votds: bibleverse_open() vdbname=%s\n",
	        vdbname) ;
#endif

	if (rs >= 0) {
	    int	pipes[2] ;
	    if ((rs = u_pipe(pipes)) >= 0) {
	        const int	aval = argvalue ;
	        int		wfd = pipes[1] ;
	        const char	*af = afname ;
	        fd = pipes[0] ;
	        rs = procdbs(sip,&ainfo,&pargs,wfd,af,aval,f_apm) ;
		u_close(wfd) ;
	        if (rs < 0) {
		    u_close(fd) ;
		    fd = -1 ;
	        }
	    } /* end if (u_pipe) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("opensvc_votds: done rs=%d fd=%u\n",rs,fd) ;
#endif

/* finish up */
badarg:
	if (sip->open.akopts) {
	    sip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	subinfo_finish(sip) ;

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

badsubstart:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_votds) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,const char **envv)
{
	int		rs = SR_OK ;

	if (envv == NULL) envv = environ ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->envv = envv ;
	sip->dt = time(NULL) ;
	sip->indent = 1 ;
	sip->year = -1 ;
	sip->f.defnull = DEFNULL ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->open.para) {
	    sip->open.para = FALSE ;
	    rs1 = biblepara_close(&sip->pdb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->f.bvs) {
	    sip->f.bvs = FALSE ;
	    rs1 = bvs_close(&sip->sdb) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	debugprintf("subinfo_finish: bvs_close() rs=%d\n",rs1) ;
#endif
	}

	if (sip->open.ndb) {
	    sip->open.ndb = FALSE ;
	    rs1 = biblebook_close(&sip->ndb) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("subinfo_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_setroot(SUBINFO *sip,const char *pr,const char *sn)
{
	if (pr == NULL) return SR_FAULT ;
	sip->pr = pr ;
	if (sn == NULL) sn = getourenv(sip->envv,VARSEARCHNAME) ;
	if (sn == NULL) sn = SEARCHNAME ;
	sip->sn = sn ;
	return SR_OK ;
}
/* end subroutine (subinfo_setroot) */


static int subinfo_nlookup(sip,bi,buf,buflen)
struct subinfo	*sip ;
int		bi ;
char		buf[] ;
int		buflen ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (! sip->open.ndb) {
	    sip->open.ndb = TRUE ;
	    rs = biblebook_open(&sip->ndb,sip->pr,sip->ndbname) ;
	    sip->open.ndb = (rs >= 0) ;
	}

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_nlookup: start rs=%d ndb=%u\n",
	        rs,sip->f.ndb) ;
#endif

	buf[0] = '\0' ;
	if (rs >= 0)  {
	    rs = biblebook_lookup(&sip->ndb,bi,buf,buflen) ;
	    len = rs ;
	}

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_nlookup: ret rs=%d len=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_nlookup) */


static int subinfo_match(sip,mbuf,mlen)
struct subinfo	*sip ;
const char	mbuf[] ;
int		mlen ;
{
	int		rs = SR_OK ;
	int		bi = 0 ;

	if (! sip->open.ndb) {
	    rs = biblebook_open(&sip->ndb,sip->pr,sip->ndbname) ;
	    sip->open.ndb = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = biblebook_match(&sip->ndb,mbuf,mlen) ;
	    bi = rs ;
	}

	return (rs >= 0) ? bi : rs ;
}
/* end subroutine (subinfo_match) */


/* get (find out) the current MJD (for today) */
static int subinfo_today(SUBINFO *sip)
{
	const int	to = TO_MJD ;
	int		rs ;
	int		yr ;
	int		mjd = sip->mjd ;

	if ((rs = subinfo_tmtime(sip)) >= 0) {
	    if ((! sip->f.mjd) || ((sip->dt - sip->ti_mjd) >= to)) {
	        TMTIME	*tmp = &sip->tm ;
	        sip->ti_mjd = sip->dt ;
	        yr = (tmp->year + TM_YEAR_BASE) ;
	        rs = getmjd(yr,tmp->mon,tmp->mday) ;
	        mjd = rs ;
	        sip->f.mjd = TRUE ;
	        sip->mjd = mjd ;
	    }
	} /* end if (subinfo-tmtime) */

	return (rs >= 0) ? mjd : rs ;
}
/* end subroutine (subinfo_today) */


static int subinfo_defdayspec(SUBINFO *sip,DAYSPEC *dsp)
{
	int		rs = SR_OK ;
	if ((rs >= 0) && (dsp->y <= 0)) {
	                   rs = subinfo_year(sip) ;
	                   dsp->y = sip->year ;
	}
	if ((rs >= 0) && (dsp->m < 0)) {
	                   rs = subinfo_tmtime(sip) ;
	                   dsp->m = sip->tm.mon ;
	}
	return rs ;
}
/* end subroutine (subinfo_defdayspec) */


/* get the current year (if necessary) */
static int subinfo_year(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;

	if (sip->year <= 0) {
	    if ((rs = subinfo_tmtime(sip)) >= 0) {
	        sip->year = (sip->tm.year + TM_YEAR_BASE) ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_year) */


static int subinfo_tmtime(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		tc = TIMECOUNT ;
	int		to = TO_TMTIME ;

	if ((! sip->f.tmtime) || (sip->timecount++ >= tc)) {
	    if ((sip->dt == 0) || (sip->timecount == tc)) {
	        sip->dt = time(NULL) ;
	    }
	    sip->timecount = 0 ;
	    if ((! sip->f.tmtime) || ((sip->dt - sip->ti_tmtime) >= to)) {
	        sip->ti_tmtime = sip->dt ;
	        sip->f.tmtime = TRUE ;
		if (sip->f.gmt) {
	            rs = tmtime_gmtime(&sip->tm,sip->dt) ;
		} else {
	            rs = tmtime_localtime(&sip->tm,sip->dt) ;
		}
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_tmtime) */


static int subinfo_mkmodquery(sip,qp,mjd)
struct subinfo	*sip ;
BIBLEVERSE_Q	*qp ;
int		mjd ;
{
	BVS		*bsp = &sip->sdb ;
	BVS_VERSE	bsq ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("opensvc_votds/subinfo_mkmodquery: mjd=%u\n",mjd) ;
#endif

	if (! sip->f.bvs)
	    rs = subinfo_bvs(sip) ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_mkmodquery: "
	        "subinfo_bvs() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = bvs_mkmodquery(bsp,&bsq,mjd) ;
	    qp->b = bsq.b ;
	    qp->c = bsq.c ;
	    qp->v = bsq.v ;
	}

#if	CF_DEBUGS
	{
	    debugprintf("opensvc_votds/subinfo_mkmodquery: "
	        "ret rs=%d\n",rs) ;
	    debugprintf("opensvc_votds/subinfo_mkmodquery: b=%u",qp->b) ;
	    debugprintf("opensvc_votds/subinfo_mkmodquery: c=%u",qp->c) ;
	    debugprintf("opensvc_votds/subinfo_mkmodquery: v=%u",qp->v) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (subinfo_mkmodquery) */


static int subinfo_bvs(SUBINFO *sip)
{
	BVS		*bsp = &sip->sdb ;
	int		rs = SR_OK ;

	if (sip->f.bvs) /* already open (and built?) */
	    goto ret0 ;

#if	CF_DEBUGS
	{
	    debugprintf("opensvc_votds/subinfo_bvs: pr=%s\n",sip->pr) ;
	    debugprintf("opensvc_votds/subinfo_bvs: sdbname=%s\n",
	        sip->sdbname) ;
	}
#endif

	rs = bvs_open(bsp,sip->pr,sip->sdbname) ;
	sip->f.bvs = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvs: bvs_open()¹ rs=%d\n",rs) ;
#endif

	if (rs == SR_NOENT) {
	    if ((rs = subinfo_bvsbuild(sip)) >= 0) {
	        rs = bvs_open(bsp,sip->pr,sip->sdbname) ;
	        sip->f.bvs = (rs >= 0) ;
#if	CF_DEBUGS
	        debugprintf("opensvc_votds/subinfo_bvs: bvs_open()² rs=%d\n",
	                rs) ;
#endif
	    }
	}

#if	CF_DEBUGS
	{
	    BVS_INFO	bi ;
	    int		rs1 ;
	    rs1 = bvs_info(bsp,&bi) ;
	    debugprintf("opensvc_votds/subinfo_bvs: bvs_info() rs=%d\n",rs1) ;
	    debugprintf("opensvc_votds/subinfo_bvs: nverses=%u\n",
	        bi.nverses) ;
	    debugprintf("opensvc_votds/subinfo_bvs: nzverses=%u\n",
	        bi.nzverses) ;
	}
#endif /* CF_DEBUGS */

	if ((rs >= 0) && sip->f.audit) {

	    rs = bvs_audit(bsp) ;

#if	CF_DEBUGS
	        debugprintf("opensvc_votds/subinfo_bvs: bvs_audit() rs=%d\n",
	            rs) ;
#endif /* CF_DEBUGS */

	} /* end if */

ret0:

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_bvs) */


/* build the BVS database */
static int subinfo_bvsbuild(SUBINFO *sip)
{
	BIBLEVERSE	*vdbp = &sip->vdb ;
	BIBLEVERSE_INFO	binfo ;
	BVSMK		*bmp = &sip->bsmk ;
	const mode_t	operms = 0666 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		oflags ;
	int		b ;
	int		nc ;
	int		to = TO_MKWAIT ;
	int		t ;
	const char	*sdbname = sip->sdbname ;
	uchar		*bap = NULL ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvsbuild: ent\n") ;
#endif

/* the "verse" data-base object must already be open */

	if (! sip->f.bibleverse) {
	    rs = SR_NOTOPEN ;
	    goto ret0 ;
	}

	oflags = 0 ;
	rs = bvsmk_open(bmp,sip->pr,sdbname,oflags,operms) ;
	sip->f.bvsmk = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvsbuild: bvsmk_open() rs=%d\n",
	        rs) ;
#endif

	if (rs == SR_INPROGRESS)
	    goto retinprogress ;

	if (rs < 0) goto ret0 ;

/* mkgo */

	if (rs >= 0) {
	    if ((rs = bibleverse_info(vdbp,&binfo)) >= 0) {
	    const int	maxbook = binfo.maxbook ;
	    const int	maxchapter = binfo.maxchapter ;
	    int		bal ;
	    int		size ;
	    bal = (maxchapter + 1) ;
	    size = bal * sizeof(uchar) ;
	    if ((rs = uc_malloc(size,&bap)) >= 0) {

	        for (b = 0 ; (rs >= 0) && (b <= maxbook) ; b += 1) {

#if	CF_DEBUGS
	            debugprintf("opensvc_votds/subinfo_bvsbuild: b=%u\n",
	                    b) ;
#endif

	            rs1 = bibleverse_chapters(vdbp,b,bap,bal) ;
	            nc = rs1 ;

#if	CF_DEBUGS
	                debugprintf("opensvc_votds/subinfo_bvsbuild: "
	                    "bibleverses_chapters() rs=%d\n",rs1) ;
#endif

	            if (rs1 == SR_NOTFOUND) {
	                nc = 0 ;
	            } else
	                rs = rs1 ;

	            if (rs >= 0)
	                rs = bvsmk_add(bmp,b,bap,nc) ;

#if	CF_DEBUGS
	                debugprintf("opensvc_votds/subinfo_bvsbuild: "
	                    "bvsmk_add() rs=%d\n",rs) ;
#endif

	        } /* end for (looping through the books) */

	        uc_free(bap) ;
	        bap = NULL ;
	    } /* end if (temporary memory allocation) */
	} /* end if (bibleverse_info) */
	} /* end if (ok) */

ret1:
	sip->f.bvsmk = FALSE ;
	rs1 = bvsmk_close(bmp) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvsbuild: bvsmk_close() rs=%d\n",
	        rs) ;
#endif

ret0:

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_bvsbuild: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* other stuff */
retinprogress:
	rs1 = SR_EXIST ;
	oflags = (O_CREAT | O_EXCL) ;
	for (t = 0 ; t < to ; t += 1) {
	    msleep(300) ;
	    rs1 = bvsmk_open(bmp,sip->pr,sdbname,oflags,operms) ;
	    sip->f.bvsmk = (rs1 >= 0) ;
	    if ((rs1 >= 0) || (rs1 == SR_EXIST))
	        break ;
	} /* end while */

	if (t < to) {
	    if (rs1 >= 0) {
	        rs = SR_OK ;
	        goto ret1 ;
	    } else if (rs1 == SR_EXIST)
	        rs1 = SR_OK ;
	    rs = rs1 ;
	} else
	    rs = SR_TIMEDOUT ;

	goto ret0 ;

}
/* end subroutine (subinfo_bvsbuild) */


/* return maximum days possible for this citation */
static int subinfo_edays(sip,qp,edays)
SUBINFO		*sip ;
BIBLEVERSE_Q	*qp ;
int		edays ;
{
	BIBLEVERSE_Q	q ;
	uint		v ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (edays == 0)
	    goto ret0 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_edays: edays=%u\n",edays) ;
#endif

	q = *qp ;
	rs1 = SR_NOTFOUND ;
	while ((rs1 == SR_NOTFOUND) && (edays > 0)) {

#if	CF_DEBUGS
	        debugprintf("opensvc_votds/subinfo_edays: loop edays=%u\n",
	            edays) ;
#endif

	    v = qp->v ;
	    v += edays ;
	    if (v <= UCHAR_MAX) {

#if	CF_DEBUGS
	            debugprintf("opensvc_votds/subinfo_edays: q=(%u:%u:%u)\n",
	                q.b,q.b,(q.v & 0xff)) ;
#endif

	        q.v = v ;
	        rs1 = bibleverse_get(&sip->vdb,&q,NULL,0) ;

	    } /* end if */

	    if (rs1 == SR_NOTFOUND)
	        edays -= 1 ;

#if	CF_DEBUGS
	    {
	        debugprintf("opensvc_votds/subinfo_edays: rs1=%d\n",rs1) ;
	        debugprintf("opensvc_votds/subinfo_edays: mod edays=%u\n",
	            edays) ;
	    }
#endif

	} /* end while */

ret0:
	return (rs >= 0) ? edays : rs ;
}
/* end subroutine (subinfo_edays) */


static int subinfo_ispara(SUBINFO *sip,BIBLEVERSE_Q *qp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (qp == NULL) return SR_FAULT ;

	if (! sip->f.para) goto ret0 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/subinfo_ispara: active\n") ;
#endif

	if (! sip->open.para) {
	    if ((rs = biblepara_open(&sip->pdb,sip->pr,sip->pdbname)) >= 0) {
	        sip->open.para = TRUE ;
	    } else if (isNotPresent(rs)) 
		rs = SR_OK ;
	}

	if ((rs >= 0) && sip->open.para) {
	    BIBLEPARA_QUERY	pq ;
	    pq.b = qp->b ;
	    pq.c = qp->c ;
	    pq.v = qp->v ;
	    rs = biblepara_ispara(&sip->pdb,&pq) ;
	    f = (rs > 0) ;
#if	CF_DEBUGS
	        debugprintf("opensvc_votds/subinfo_ispara: "
	            "biblepara_ispara() rs=%d\n",rs) ;
#endif
	}

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_ispara) */


/* process the program ako-names */
static int procopts(sip,kop)
SUBINFO		*sip ;
KEYOPT		*kop ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(sip->envv,VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                switch (oi) {

	                case akoname_audit:
	                    if (! sip->final.audit) {
	                        sip->have.audit = TRUE ;
	                        sip->final.audit = TRUE ;
	                        sip->f.audit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.audit = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_linelen:
	                    if (! sip->final.linelen) {
	                        sip->have.linelen = TRUE ;
	                        sip->final.linelen = TRUE ;
	                        sip->f.linelen = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            sip->linelen = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_indent:
	                    if (! sip->final.indent) {
	                        sip->have.indent = TRUE ;
	                        sip->final.indent = TRUE ;
	                        sip->f.indent = TRUE ;
	                        sip->indent = 1 ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            sip->indent = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_bookname:
	                    if (! sip->final.bookname) {
	                        sip->have.bookname = TRUE ;
	                        sip->final.bookname = TRUE ;
	                        sip->f.bookname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.bookname = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_interactive:
	                    if (! sip->final.interactive) {
	                        sip->have.interactive = TRUE ;
	                        sip->final.interactive = TRUE ;
	                        sip->f.interactive = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.interactive = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_separate:
	                    if (! sip->final.separate) {
	                        sip->have.separate = TRUE ;
	                        sip->final.separate = TRUE ;
	                        sip->f.separate = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.separate = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_default:
	                case akoname_defnull:
	                    if (! sip->final.defnull) {
	                        sip->have.defnull = TRUE ;
	                        sip->final.defnull = TRUE ;
	                        sip->f.defnull = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.defnull = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_para:
	                    if (! sip->final.para) {
	                        sip->have.para = TRUE ;
	                        sip->final.para = TRUE ;
	                        sip->f.para = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.para = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_gmt:
	                    if (! sip->final.gmt) {
	                        sip->have.gmt = TRUE ;
	                        sip->final.gmt = TRUE ;
	                        sip->f.gmt = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            sip->f.gmt = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_atype:
	                case akoname_qtype:
	                    if (vl) {
	                        rs = matostr(qtypes,1,vp,vl) ;
	                        sip->qtype = rs ;
	                    }
	                    break ;

	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procdbs(sip,aip,bop,fd,afn,aval,f_apm)
SUBINFO		*sip ;
ARGINFO		*aip ;
BITS		*bop ;
int		fd ;
const char	*afn ;
int		aval ;
int		f_apm ;
{
	int		rs ;
	int		rs1 ;
	const char	*vdb = sip->vdbname ;
	if ((rs = bibleverse_open(&sip->vdb,sip->pr,vdb)) >= 0) {
	    sip->nverses = rs ;
	    sip->f.bibleverse = TRUE ;

#if	CF_DEBUGS
	        debugprintf("opensvc_votds: bibleverse_open() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	    {
	        BIBLEVERSE_INFO	bi ;
	        rs1 = bibleverse_info(&sip->vdb,&bi) ;
	        debugprintf("opensvc_votds: info() rs=%d\n",rs1) ;
	        debugprintf("opensvc_votds: maxbook=%u\n",bi.maxbook) ;
	        debugprintf("opensvc_votds: maxchapter=%u\n",bi.maxchapter) ;
	        debugprintf("opensvc_votds: nverses=%u\n",bi.nverses) ;
	        debugprintf("opensvc_votds: nzverses=%u\n",bi.nzverses) ;
	    }
#endif /* CF_DEBUGS */

	        rs = procargs(sip,aip,bop,fd,afn,aval,f_apm) ;

	    sip->f.bibleverse = FALSE ;
	    rs1 = bibleverse_close(&sip->vdb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bibleverse) */
	return rs ;
}
/* end subroutine (procdbs) */


static int procargs(sip,aip,bop,fd,afname,argvalue,f_apm)
SUBINFO		*sip ;
struct arginfo	*aip ;
BITS		*bop ;
int		fd ;
const char	*afname ;
int		argvalue ;
int		f_apm ;
{
	FILEBUF		b ;
	int		rs ;

	if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;
	    sip->ofp = &b ;

	    if (! sip->f.all) {
	        int	ai ;
	        int	f ;

	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                pan += 1 ;
	                rs = procspec(sip,cp,-1) ;
		    }

	            if (rs < 0) break ;
	        } /* end for (looping through positional arguments) */

	    } /* end if (positional arguments) */

	    if ((! sip->f.all) && (rs >= 0) && 
	        (afname != NULL) && (afname[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afname,"-") == 0)
	            afname = STDINFNAME ;

	        if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
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
	                	rs = procspecs(sip,cp,cl) ;
			    }
			}

	                if (rs < 0) break ;
	            } /* end while */

	            bclose(afp) ;
	        } /* end if */

	    } /* end if (afile arguments) */

	    if ((rs >= 0) && (! sip->f.all) && f_apm) {

#if	CF_DEBUGS
	            debugprintf("opensvc_votds: +<n>\n") ;
#endif

	        pan += 1 ;
	        rs = proctoday(sip,f_apm,argvalue) ;

	    } /* end if */

	    if ((rs >= 0) && (! sip->f.all) && (pan == 0) && sip->f.defnull) {
	            int	edays = 0 ;

	            if (argvalue > 1) edays = (argvalue-1) ;
	            rs = proctoday(sip,TRUE,edays) ;

	    } /* end if */

/* handle the case of printing all bible-verses out */

	    if (sip->f.all && (rs >= 0)) {
	        rs = procall(sip) ;
	    }

#if	CF_DEBUGS
	        debugprintf("opensvc_votds: before-close rs=%d\n",rs) ;
#endif

	    sip->ofp = NULL ;
	    filebuf_finish(&b) ;
	} /* end if (filebuf) */

	return rs ;
}
/* end subroutine (procargs) */


static int procspecs(sip,sp,sl)
SUBINFO		*sip ;
const char	*sp ;
int		sl ;
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (sp == NULL) return SR_FAULT ;

	if (sip->f.interactive) sip->cout = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    const int	flen = sl ;
	    char	*fbuf ;
	    if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	        int	fl ;
	        while ((fl = field_sharg(&fsb,aterms,fbuf,flen)) >= 0) {
		    if (fl > 0) {
	               rs = procspec(sip,fbuf,fl) ;
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


static int procspec(sip,sp,sl)
SUBINFO		*sip ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	int		edays = (sip->nitems-1) ; /* extra days to process */
	int		wlen = 0 ;

	if (sp == NULL)
	    return SR_FAULT ;

	if (sp[0] == '\0')
	    goto ret0 ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/procspec: spec>%t<\n",sp,sl) ;
#endif

	if ((sp[0] == '+') || (sp[0] == '-')) {
	    const int	f_plus = (sp[0] == '+') ;
	    int		v = (sip->nitems-1) ;

	    if (sl > 1) { /* cannot now happen!? */
	        sl -= 1 ;
	        sp += 1 ;
	        rs = cfdeci(sp,sl,&v) ;
	    }

	    if (rs >= 0) {
	        rs = proctoday(sip,f_plus,v) ;
	        wlen += rs ;
	    }

	} else if (sip->qtype == qtype_verse) {
	    BIBLEVERSE_Q	q ;

	    rs = procparse(sip,&q,sp,sl) ;

#if	CF_DEBUGS
	        debugprintf("opensvc_votds/procspec: parse rs=%d q=%u:%u:%u\n",
	            rs,q.b,q.c,q.v) ;
#endif

	    if (rs > 0) {
	        rs = proccites(sip,&q,edays) ;
	        wlen += rs ;
	    }

	} else {
	    int	mjd = -1 ;

	    if (sip->qtype == qtype_day) {
		if ((rs = hasourmjd(sp,sl)) > 0) {
		    mjd = rs ;
		} else {
	            DAYSPEC	ds ;
	            if ((rs = dayspec_load(&ds,sp,sl)) >= 0) {
			if ((rs = subinfo_defdayspec(sip,&ds)) >= 0) {
	                   rs = getmjd(ds.y,ds.m,ds.d) ;
	                   mjd = rs ;
	               }
	           } /* end if (dayspec) */
		} /* end if (type-day) */
	    } else if (sip->qtype == qtype_mjd) {
		if ((rs = hasourmjd(sp,sl)) > 0) {
		    mjd = rs ;
		} else {
	            uint	uv ;
	            rs = cfdecui(sp,sl,&uv) ;
	            mjd = (int) uv ;
		}
	    } else
	        rs = SR_INVALID ;

	    if ((rs >= 0) && (mjd >= 0)) {
	        rs = procmjds(sip,mjd,edays) ;
	        wlen += rs ;
	    }

	} /* end if (handling different query types) */

ret0:

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/procspec: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procall(SUBINFO *sip)
{
	BIBLEVERSE	*vdbp ;
	BIBLEVERSE_CUR	cur ;
	BIBLEVERSE_Q	q ;	/* result */
	const int	bvlen = BVBUFLEN ;
	int		rs ;
	int		bvl ;
	int		wlen = 0 ;
	char		bvbuf[BVBUFLEN + 1] ;

	vdbp = &sip->vdb ;
	if ((rs = bibleverse_curbegin(vdbp,&cur)) >= 0) {

	    while (rs >= 0) {

	        bvl = bibleverse_enum(vdbp,&cur,&q,bvbuf,bvlen) ;
	        if ((bvl == SR_NOTFOUND) || (bvl == 0)) break ;

#if	CF_DEBUGS
	        {
	            debugprintf("opensvc_votds: bvl=%u\n",bvl) ;
	            debugprintf("opensvc_votds: q=%u:%u:%u\n",q.b,q.c,q.v) ;
	        }
#endif

	        rs = bvl ;
	        if (rs >= 0)
	            rs = procout(sip,&q,bvbuf,bvl) ;

	    } /* end while */

	    bibleverse_curend(vdbp,&cur) ;
	} /* end if (printing all book titles) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procall) */


static int procparse(sip,qp,spec,speclen)
SUBINFO		*sip ;
BIBLEVERSE_Q	*qp ;
const char	spec[] ;
int		speclen ;
{
	MANSTR		so ;
	uint		val ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ch ;
	int		sl, cl ;
	int		si ;
	int		f_success = FALSE ;
	const char	*sp, *cp ;

	qp->b = 0 ;
	qp->c = 0 ;
	qp->v = 0 ;

	sl = sfshrink(spec,speclen,&sp) ;
	if (sl == 0)
	    goto ret0 ;

	si = sibook(sp,sl) ;

#if	CF_DEBUGS
	{
	    debugprintf("opensvc_votds/procparse: sibook() si=%d\n",si) ;
	    if (si > 0)
	        debugprintf("opensvc_votds/procparse: bs=>%t<\n",sp,si) ;
	}
#endif

	if (si == 0)
	    goto ret0 ;

	rs1 = SR_NOTFOUND ;
	ch = sp[0] & UCHAR_MAX ;
	if (isalphalatin(ch)) {

#if	CF_BOOKMATCH
	    rs1 = procbookmatch(sip,sp,si) ;
	    qp->b = (uchar) rs1 ;
#else
	    rs1 = procbookget(sip,sp,si) ;
	    qp->b = (uchar) rs1 ;
#endif

	} else {

	    rs1 = cfdecui(sp,si,&val) ;
	    qp->b = (uchar) val ;

	} /* end if */

	if ((rs1 == SR_NOTFOUND) || (rs1 == SR_INVALID))
	    goto ret0 ;

	rs = rs1 ;
	if (rs < 0)
	    goto ret0 ;

/* we have a book */

	sp += si ;
	sl -= si ;

	rs1 = SR_OK ;
	if ((rs = manstr_start(&so,sp,sl)) >= 0) {

	    manstr_whitecolon(&so) ;

/* chapter */

	    cl = manstr_breakfield(&so,": \t",&cp) ;
	    if (cl <= 0)
	        rs1 = SR_INVALID ;

	    if (rs1 >= 0) {
	        rs1 = cfdecui(cp,cl,&val) ;
	        qp->c = val ;
	    }

	    if (rs1 >= 0) {

	        manstr_whitecolon(&so) ;

/* verse */

	        cl = manstr_breakfield(&so,": \t",&cp) ;
	        if (cl <= 0)
	            rs1 = SR_INVALID ;
	    }

	    if (rs1 >= 0) {
	        rs1 = cfdecui(cp,cl,&val) ;
	        qp->v = val ;
	    }

	    manstr_finish(&so) ;
	} /* end if (str) */

	f_success = (rs1 >= 0) ;

ret0:

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/procparse: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_success : rs ;
}
/* end subroutine (procparse) */


#if	CF_BOOKMATCH

static int procbookmatch(sip,sp,sl)
SUBINFO		*sip ;
const char	*sp ;
int		sl ;
{
	int		rs ;

	rs = subinfo_match(sip,sp,sl) ;

	return rs ;
}
/* end subroutine (procbookmatch) */

#else /* CF_BOOKMATCH */

static int procbookget(sip,sp,sl)
SUBINFO		*sip ;
const char	*sp ;
int		sl ;
{
	vecstr		ps ;
	const int	blen = BOOKBUFLEN ;
	int		rs ;
	int		opts ;
	int		j ;
	int		bbl ;
	int		m, cl ;
	int		i = 0 ;
	int		f ;
	const char	*tp, *cp ;
	const char	*ep ;
	char		bbuf[BOOKBUFLEN + 1] ;

	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&ps,4,opts) ;
	if (rs < 0)
	    goto ret0 ;

#ifdef	COMMENT
	rs = vecstr_adds(&ps,sp,sl) ;
#else
	if (sl < 0)
	    sl = strlen(sp) ;

	while ((rs >= 0) && (sl > 0)) {

	    cp = sp ;
	    cl = sl ;
	    if ((tp = strnpbrk(sp,sl," \t,")) != NULL)
	        cl = (tp - sp) ;

	    if (cl > 0)
	        rs = vecstr_add(&ps,cp,cl) ;

	    sl -= ((cp + cl) + 1) - sp ;
	    sp = ((cp + cl) + 1) ;

	} /* end while */
#endif /* COMMENT */

	if (rs < 0)
	    goto ret1 ;

	f = FALSE ;

/* leading-substring match */

	for (i = 1 ; 
	    (bbl = subinfo_nlookup(sip,i,bbuf,blen)) > 0 ; 
	    i += 1) {

	    f = TRUE ;
	    for (j = 0 ; vecstr_get(&ps,j,&ep) >= 0 ; j += 1) {
	        if (ep == NULL) continue ;

	        if (j == 0) {
	            m = nleadcasestr(ep,bbuf,bbl) ;
	            f = ((m > 0) && (ep[m] == '\0')) ;
	        } else {
	            f = (sicasesub(bbuf,bbl,ep) >= 0) ;
	        }

	        if (! f) break ;
	    } /* end for */

	    if (f) break ;
	} /* end for */

/* compressed-leading-substring match */

	if ((rs >= 0) && (! f)) {
	    char	cbbuf[BOOKBUFLEN + 1] ;

	    for (i = 1 ; 
	        (bbl = subinfo_nlookup(sip,i,bbuf,blen)) > 0 ; 
	        i += 1) {

	        cp = cbbuf ;
	        cl = snwcpyopaque(cbbuf,blen,bbuf,bbl) ;

	        f = TRUE ;
	        for (j = 0 ; vecstr_get(&ps,j,&ep) >= 0 ; j += 1) {
	            if (ep == NULL) continue ;

	            if (sicasesub(cp,cl,ep) < 0) {
	                f = FALSE ;
	                break ;
	            }

	        } /* end for */

	        if (f) break ;
	    } /* end for */

	} /* end if */

/* any-substring match */

	if ((rs >= 0) && (! f)) {

	    for (i = 1 ; 
	        (bbl = subinfo_nlookup(sip,i,bbuf,blen)) > 0 ; 
	        i += 1) {

	        f = TRUE ;
	        for (j = 0 ; vecstr_get(&ps,j,&ep) >= 0 ; j += 1) {
	            if (ep == NULL) continue ;

	            if (sicasesub(bbuf,bbl,ep) < 0) {
	                f = FALSE ;
	                break ;
	            }

	        } /* end for */

	        if (f) break ;
	    } /* end for */

	} /* end if */

/* finish up */

	if (rs >= 0)
	    rs = (f) ? i : SR_NOTFOUND ;

ret1:
	vecstr_finish(&ps) ;

ret0:
	return rs ;
}
/* end subroutine (procbookget) */

#endif /* CF_BOOKMATCH */


static int proctoday(sip,f_plus,edays)
SUBINFO		*sip ;
int		f_plus ;
int		edays ; /* extra days */
{
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/proctoday: ent f_plus=%u edays=%d\n",
	        f_plus,edays) ;
#endif

	if ((rs = subinfo_today(sip)) >= 0) {
	    int	mjd = rs ;
	    rs = procmjds(sip,mjd,edays) ;
	    wlen += rs ;
	}

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/proctoday: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proctoday) */


static int procmjds(sip,mjd,edays)
SUBINFO		*sip ;
int		mjd ;
int		edays ;
{
	BIBLEVERSE_Q	q ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/procmjds: mjd=%u edays=%d\n",mjd,edays) ;
#endif

	rs = subinfo_mkmodquery(sip,&q,mjd) ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/procmjds: subinfo_mkmodquery() rs=%d\n",
	        rs) ;
#endif

	if (rs >= 0) {
	    rs = proccites(sip,&q,edays) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmjds) */


static int proccites(sip,qp,edays)
SUBINFO		*sip ;
BIBLEVERSE_Q	*qp ;
int		edays ;
{
	const int	bvlen = BVBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		bvl ;
	int		wlen = 0 ;
	char		bvbuf[BVBUFLEN + 1] ;

	if (edays < 0) edays = 0 ;

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/proccites: q=%u:%u:%u edays=%d\n",
	        qp->b,qp->c,qp->v,edays) ;
#endif

	if (edays > 0) {
	    rs = subinfo_edays(sip,qp,edays) ;
	    edays = rs ;
	}

	if (rs < 0)
	    goto ret0 ;

	sip->ncites += 1 ;
	rs = bibleverse_get(&sip->vdb,qp,bvbuf,bvlen) ;
	bvl = rs ;

	if ((rs >= 0) && (bvl > 0)) {
	    int	vn ;

	    rs = procoutcite(sip,qp,edays) ;
	    wlen += rs ;

	    if (rs >= 0) {
	        rs = procout(sip,qp,bvbuf,bvl) ;
	        wlen += rs ;
	    }

	    for (vn = 1 ; (rs >= 0) && (vn < (edays+1)) ; vn += 1) {
	        if (qp->v == 255) break ;

	        qp->v += 1 ;
	        rs1 = bibleverse_get(&sip->vdb,qp,bvbuf,bvlen) ;
	        bvl  = rs1 ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;

	        if (rs >= 0) {
	            rs = procout(sip,qp,bvbuf,bvl) ;
	            wlen += rs ;
	        }

	    } /* end for */

	} /* end if */

ret0:

#if	CF_DEBUGS
	    debugprintf("opensvc_votds/proccite: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proccites) */


static int procoutcite(sip,qp,edays)
SUBINFO		*sip ;
BIBLEVERSE_Q	*qp ;
int		edays ;
{
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		b = qp->b ;
	int		c = qp->c ;
	int		v = qp->v ;
	int		wlen = 0 ;
	int		f_havebook = FALSE ;
	const char	*fmt ;
	char		cbuf[COLBUFLEN + 1] ;

	if (clen <= 0)
	    goto ret0 ;

#if	CF_COOKIE
	fmt = "%%\n" ;
#else
	fmt = "\n" ;
#endif

/* print out any necessary separator */

	if (sip->f.separate && (sip->cout++ > 0)) {
	    rs = filebuf_printf(sip->ofp,fmt) ;
	    wlen += rs ;
	} /* end if (separator) */

/* print out the text-data itself */

	if (rs >= 0) {

	if (sip->f.bookname) {
	    const int	blen = BOOKBUFLEN ;
	    int		bbl ;
	    char	bbuf[BOOKBUFLEN + 1] ;

	    if ((bbl = subinfo_nlookup(sip,qp->b,bbuf,blen)) > 0) {

	        f_havebook = TRUE ;
	        fmt = (edays > 0) ? "%t %u:%u (%u)" : "%t %u:%u" ;
	        rs = bufprintf(cbuf,clen,fmt,bbuf,bbl,c,v,(edays+1)) ;
	        cl = rs ;
	        if (rs >= 0) {
	            rs = filebuf_print(sip->ofp,cbuf,cl) ;
	            wlen += rs ;
	        }

	    } /* end if (nlookup) */

	} /* end if (book-name) */

	if ((rs >= 0) && (! f_havebook)) {

	    fmt = (edays > 0) ? "%u:%u:%u (%u)" : "%u:%u:%u" ;
	    rs = bufprintf(cbuf,clen,fmt,b,c,v,(edays+1)) ;
	    cl = rs ;
	    if (rs >= 0) {
	        rs = filebuf_print(sip->ofp,cbuf,cl) ;
	        wlen += rs ;
	    }

	} /* end if (type of book-name display) */

	} /* end if (ok) */

ret0:

#if	CF_DEBUGS
	debugprintf("opensvc_votds/procoutcite: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutcite) */


static int procout(sip,qp,vp,vl)
SUBINFO		*sip ;
BIBLEVERSE_Q	*qp ;
const char	vp[] ;
int		vl ;
{
	WORDFILL	w ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		sl = vl ;
	int		cl ;
	int		cbl ;
	int		line = 0 ;
	int		wlen = 0 ;
	int		f_p = FALSE ;
	const char	*sp = vp ;
	char		cbuf[COLBUFLEN + 1] ;

	if (vl <= 0)
	    goto ret0 ;

	cbl = MIN((sip->linelen - sip->indent),clen) ;

	if ((rs >= 0) && sip->f.para) {
	    rs = subinfo_ispara(sip,qp) ;
	    f_p = (rs > 0) ;
	}
	if (rs < 0) goto ret0 ;

/* print out the text-data itself */

	if (f_p) sp = NULL ;

	if ((rs = wordfill_start(&w,sp,sl)) >= 0) {

	    if (f_p) {
	        if ((rs = wordfill_addword(&w,"¶",1)) >= 0)
	            rs = wordfill_addlines(&w,vp,vl) ;
	    }

	    while (rs >= 0) {
	        cl = wordfill_mklinefull(&w,cbuf,cbl) ;
	        if ((cl == 0) || (cl == SR_NOTFOUND)) break ;

	        rs = cl ;
	        if (rs >= 0) {
	            rs = procoutline(sip,line,cbuf,cl) ;
	            wlen += rs ;
	            line += 1 ;
	        }

	    } /* end while (full lines) */

	    if (rs >= 0) {
	        cl = wordfill_mklinepart(&w,cbuf,cbl) ;

	        if (cl > 0) {
	            rs = procoutline(sip,line,cbuf,cl) ;
	            wlen += rs ;
	            line += 1 ;
	        } else if (cl != SR_NOTFOUND)
	            rs = cl ;

	    } /* end if (partial lines) */

	    wordfill_finish(&w) ;
	} /* end if (word-fill) */

ret0:

#if	CF_DEBUGS
	debugprintf("opensvc_votds/procout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


/* ARGSUSED */
static int procoutline(sip,line,lp,ll)
SUBINFO		*sip ;
int		line ;
const char	*lp ;
int		ll ;
{
	int		rs ;
	int		indent ;
	int		wlen = 0 ;
	const char	*fmt ;

	indent = MIN(sip->indent,NBLANKS) ;
	fmt = "%t%t\n" ;
	rs = filebuf_printf(sip->ofp,fmt,blanks,indent,lp,ll) ;
	wlen += rs ;

#if	CF_DEBUGS
	debugprintf("opensvc_votds/procoutline: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


static int sibook(sp,sl)
const char	*sp ;
int		sl ;
{
	int		i = 0 ;

	if (sl < 0) sl = strlen(sp) ;

#ifdef	OPTIONAL
	if ((si = siskipwhite(sp,sl)) > 0) {
	    sp += si ;
	    sl -= si ;
	}
#endif /* OPTIONAL */

	if (sl > 0) {
	    int	f ;
	    int	ch = sp[0] & UCHAR_MAX ;
	    f = isalphalatin(ch) ;
	    for (i = 0 ; i < sl ; i += 1) {
	        ch = sp[i] & UCHAR_MAX ;
	        if (f && isdigitlatin(ch)) break ;
	        if (sp[i] == ':') break ;
	    } /* end for */
	} /* end if (positive) */

	return i ;
}
/* end subroutine (sibook) */


