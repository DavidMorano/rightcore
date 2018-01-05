/* b_biblespeak */

/* translate a bible number to its corresponding name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_COOKIE	0		/* use cookie as separator */
#define	CF_DEFQUERY	1		/* allow default query */


/* revision history:

	= 2009-05-04, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ biblespeak <spec(s)>


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
#include	<time.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<char.h>
#include	<vecstr.h>
#include	<wordfill.h>
#include	<bcspec.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_biblespeak.h"
#include	"defs.h"
#include	"biblebook.h"
#include	"biblemeta.h"
#include	"bibleverse.h"
#include	"naturalwords.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum bible-verse length (?) */
#endif

#define	BOOKBUFLEN	100

#define	WORDBUFLEN	NATURALWORDLEN

#define	COLBUFLEN	(COLUMNS + 10)

#define	WORDLEN		80
#define	NBLANKS		20

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
extern int	sfshrink(const char *,int,const char **) ;
extern int	siskipwhite(const char *,int) ;
extern int	sicasesub(const char *,int,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	ndigits(int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;
extern char	*strwhite(const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

enum words {
	word_chapter,
	word_psalm,
	word_bookindex,
	word_page,
	word_booktitle,
	word_thebookof,
	word_book,
	word_overlast
} ;

struct str {
	const char	*sp ;
	int		sl ;
} ;

struct locinfo_flags {
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		ndb:1 ;
	uint		wdb:1 ;
	uint		bookname:1 ;
	uint		interactive:1 ;
	uint		bookchapters:1 ;
	uint		nchapters:1 ;
	uint		all:1 ;
} ;

struct locinfo {
	BIBLEBOOK	ndb ;		/* bible book-name DB */
	BIBLEMETA	wdb ;		/* bible meta-word DB */
	BIBLEVERSE	vdb ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	void		*ofp ;
	const char	*ndbname ;
	const char	*wdbname ;
	const char	*word[word_overlast + 1] ;
	int		linelen ;
	int		indent ;
	int		count, max, precision ;
	int		cout ;
	int		nchapters ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procspecs(PROGINFO *,const char *,int) ;
static int	procspec(PROGINFO *,const char *,int) ;
static int	procall(PROGINFO *) ;

static int	proctitlebook(PROGINFO *,int) ;
static int	proctitlechapter(PROGINFO *,int,int) ;

static int	proclines(PROGINFO *,WORDFILL *,const char *,int) ;
static int	procwords(PROGINFO *,WORDFILL *,const char *,int) ;
static int	procword(PROGINFO *,WORDFILL *,const char *,int) ;

static int	procout(PROGINFO *,BIBLEVERSE_CITE *,const char *,int) ;
static int	procoutline(PROGINFO *,int,const char *,int) ;
static int	procparse(PROGINFO *,BIBLEVERSE_CITE *,cchar *,int) ;

static int	metawordsbegin(PROGINFO *) ;
static int	metawordsfins(PROGINFO *) ;
static int	metawordsend(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_nlookup(LOCINFO *,char *,int,int) ;
static int	locinfo_bookmatch(LOCINFO *,const char *,int) ;

static int	mkwordclean(char *,int,const char *,int) ;

static char	*firstup(char *) ;
static char	*alldown(char *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"ndb",
	"wdb",
	"vdb",
	"bookname",
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
	argopt_ndb,
	argopt_wdb,
	argopt_vdb,
	argopt_bookname,
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
	"bookname",
	"interactive",
	"bookchapters",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_bookname,
	akoname_interactive,
	akoname_bookchapters,
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

static const char	*leaders[] = {
	"i",
	"ii",
	"iii",
	NULL
} ;

static const char	*leadsubs[] = {
	"first",
	"second",
	"third",
	NULL
} ;


/* exported subroutines */


int b_biblespeak(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_biblespeak) */


int p_biblespeak(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_biblespeak) */


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
	SHIO		outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		n ;
	int		v ;
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
	const char	*ndbname = NULL ;
	const char	*wdbname = NULL ;
	const char	*vdbname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_biblespeak: starting DFD=%d\n",rs) ;
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

	lip->ofp = ofp ;

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

/* BibleBook book-name DB name */
	                case argopt_ndb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ndbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ndbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* BibleBook meta-word DB name */
	                case argopt_wdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            wdbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                wdbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* BibleBook-verse DB name */
	                case argopt_vdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            vdbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                vdbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_bookname:
	                    lip->have.bookname = TRUE ;
	                    lip->final.bookname = TRUE ;
	                    lip->f.bookname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.bookname = (rs > 0) ;
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
	                        lip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* number-of-chapters */
	                    case 'c':
	                        lip->have.nchapters = TRUE ;
	                        lip->final.nchapters = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                lip->nchapters = rs ;
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

/* width (columns) */
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
	    debugprintf("b_biblespeak: debuglevel=%u\n",pip->debuglevel) ;
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
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

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

/* argument processing */

	rs1 = (DEFPRECISION + 2) ;
	if ((rs >= 0) && (lip->linelen < rs1) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&v) ;
	    lip->have.linelen = TRUE ;
	    lip->final.linelen = TRUE ;
	    lip->linelen = rs ;
	}

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

/* bible-name DB */

	if (ndbname == NULL) ndbname = getourenv(envv,VARNDB) ;

#ifdef	OPTIONAL
	if (ndbname == NULL)
	    ndbname = NDBNAME ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: ndbname=%s\n",
	        pip->progname,ndbname) ;
	}

	lip->ndbname = ndbname ;

/* bible-meta DB */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: wdbname=%s\n",
	        pip->progname,wdbname) ;
	}

	lip->wdbname = wdbname ;

/* bible-verse DB */

	if (vdbname == NULL) vdbname = getourenv(envv,VARVDB) ;
	if (vdbname == NULL) vdbname = VDBNAME ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: vdbname=%s\n",
	        pip->progname,vdbname) ;
	}

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* process */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_biblespeak: bibleverse_open() vdbname=%s\n",
	        vdbname) ;
#endif

	if (rs >= 0) {
	if ((rs = bibleverse_open(&lip->vdb,pip->pr,vdbname)) >= 0) {
	    const int	nverses = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_biblespeak: bibleverse_open() rs=%d\n",rs) ;
#endif

	    if (lip->f.audit) {
	        rs = bibleverse_audit(&lip->vdb) ;
	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,
	                "%s: biblespeak DB audit (%d)\n",
	                pip->progname,rs) ;
		}
	    }


	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"%s: total verses=%u\n",
	            pip->progname,nverses) ;
	    }

	    if (rs >= 0) {
	        if ((rs = metawordsbegin(pip)) >= 0) {
	            {
	                rs = process(pip,&ainfo,&pargs,ofname,afname) ;
	            }
	            rs1 = metawordsend(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (metawords) */
	    } /* end if (ok) */

	    rs1 = bibleverse_close(&lip->vdb) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_CONFIG ;
	    fmt = "%s: biblespeak DB unavailable (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: vdb=%s\n",pn,vdbname) ;
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
	    switch (rs) {
	    default:
	        if (! pip->f.quiet) {
	            const char	*fmt = "%s: could not perform function (%d)\n" ;
	            shio_printf(pip->efp,fmt,pip->progname,rs) ;
	        }
/* FALLTHROUGH */
	    case SR_PIPE:
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_biblespeak: done rs=%d ex=%u\n",rs,ex) ;
#endif

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
	    debugprintf("b_biblespeak: final mallout=%u\n",mo-mo_start) ;
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
/* end subroutine (b_biblespeak) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<number(s)>|<string(s)> ...] [-a]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
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
	                case akoname_bookname:
	                    if (! lip->final.bookname) {
	                        lip->have.bookname = TRUE ;
	                        lip->final.bookname = TRUE ;
	                        lip->f.bookname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.bookname = (rs > 0) ;
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
	                case akoname_bookchapters:
	                    if (! lip->final.bookchapters) {
	                        lip->have.bookchapters = TRUE ;
	                        lip->final.bookchapters = TRUE ;
	                        lip->f.bookchapters = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.bookchapters = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else {
			rs = SR_INVALID ;
		    }

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (key-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *ofn)
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

	    if (lip->f.all) {
	        rs = procall(pip) ;
	        wlen += rs ;
	    } else {
	        rs = procargs(pip,aip,bop,afn) ;
	        wlen += rs ;
	    }

	    lip->ofp = NULL ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    char	*fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afname)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		wlen = 0 ;
	int		cl ;
	const char	*cp ;

	if (rs >= 0) {
	    int	ai ;
	    int	f ;
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
	} /* end if (positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
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
	    } /* end if (shio) */

	} /* end if (argument-list) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procall(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		nbl ;
	int		i ;
	char		nbuf[3 + 1] ;

	for (i = 1 ; i < (BIBLEBOOK_NBOOKS+1) ; i += 1) {

	    nbl = ctdeci(nbuf,3,i) ;

	    rs = procspec(pip,nbuf,nbl) ;
	    wlen += rs ;

	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;

	if (sp == NULL) return SR_FAULT ;

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
	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
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
	BIBLEVERSE_CITE	q ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cbl ;
	int		i, j ;
	int		c_eoc = 0 ;
	int		wlen = 0 ;
	int		f_titlebook = FALSE ;
	int		f_titlechapter = FALSE ;

	if (sp == NULL) return SR_FAULT ;
	if (sp[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_biblespeak/procspec: spec>%t<\n",sp,sl) ;
#endif

	if ((rs = procparse(pip,&q,sp,sl)) >= 0) {
	    BIBLEVERSE	*bvp = &lip->vdb ;
	    uint	cstart, cend ;
	    const int	comlen = COMBUFLEN ;
	    char	combuf[COMBUFLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_biblespeak/procspec: get q=%u:%u:%u\n",
	            q.b,q.c,q.v) ;
#endif

	    f_titlebook = FALSE ;
	    f_titlechapter = FALSE ;
	    cstart = q.c ;
	    cend = (lip->nchapters > 0) ? (q.c + lip->nchapters) : UCHAR_MAX ;
	    for (i = cstart ; i < cend ; i += 1) {

	        q.c = i ;
	        f_titlechapter = FALSE ;
	        for (j = 0 ; j < UCHAR_MAX ; j += 1) {

	            q.v = j ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_biblespeak/procspec: q=<%u:%u:%u>\n",
	                    q.b,q.c,q.v) ;
#endif

	            rs1 = bibleverse_get(bvp,&q,combuf,comlen) ;
	            cbl = rs1 ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_biblespeak/procspec: "
	                    "bibleverse_get() rs=%d\n", rs1) ;
#endif

	            if ((rs1 < 0) && (q.c > 0) && (q.v > 0)) {
	                c_eoc += 1 ;
	                break ;
	            }

	            c_eoc = 0 ;
	            if (rs1 >= 0) {
	                int	f ;

	                if ((! f_titlebook) || (! f_titlechapter)) {
	                    rs = shio_printf(lip->ofp,"\n") ;
	                    wlen += rs ;
	                }

	                f = (! f_titlebook) ;
	                f = f || ((! f_titlechapter) && lip->f.bookchapters) ;
	                if (f) {
	                    f_titlebook = TRUE ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("b_biblespeak/procspec: "
	                            "proctitlebook() b=%u\n",q.b) ;
#endif
	                    rs = proctitlebook(pip,q.b) ;
	                    wlen += rs ;
	                }

	                if (! f_titlechapter) {
	                    f_titlechapter = TRUE ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("b_biblespeak/procspec: "
	                            "proctitlechapter() \n") ;
#endif
	                    rs = proctitlechapter(pip,q.b,q.c) ;
	                    wlen += rs ;
	                }

	                if ((rs >= 0) && (cbl > 0)) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("b_biblespeak/procspec: "
	                            "procout() \n") ;
#endif
	                    rs = procout(pip,&q,combuf,cbl) ;
	                    wlen += rs ;
	                }

	            } /* end if */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_biblespeak/procspec: for-bot rs=%d\n",
	                    rs) ;
#endif
	            if (rs < 0) break ;
	        } /* end for (verses) */

	        if ((lip->nchapters > 0) && (c_eoc >= lip->nchapters))
	            break ;

	        if (rs < 0) break ;
	    } /* end for (chapters) */

	} else if (rs == 0) {
	    if (lip->f.interactive)
	        rs = shio_printf(lip->ofp,"citation=>%t< invalid\n",
	            sp,sl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_biblespeak/procspec: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procparse(PROGINFO *pip,BIBLEVERSE_Q *qp,cchar sp[],int sl)
{
	BCSPEC		bb ;
	int		rs ;

	if ((rs = bcspec_load(&bb,sp,sl)) >= 0) {
	    const int	nl = bb.nl ;
	    cchar	*np = bb.np ;
	    qp->b = bb.b ;
	    qp->c = bb.c ;
	    qp->v = bb.v ;
	    if (np != NULL) {
	        LOCINFO		*lip = pip->lip ;
	        rs = locinfo_bookmatch(lip,np,nl) ;
	        qp->b = rs ;
	    }
	} /* end if (bcspec_load) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_biblespeak/procparse: ret rs=%d\n",rs) ;
	    debugprintf("b_biblespeak/procparse: ret q=%u:%u:%u\n",
	        qp->b,qp->c,qp->v) ;
	}
#endif

	return rs ;
}
/* end subroutine (procparse) */


static int proctitlebook(PROGINFO *pip,int nbook)
{
	LOCINFO		*lip = pip->lip ;
	const int	blen = BOOKBUFLEN ;
	int		rs = SR_OK ;
	int		bbl ;
	int		i ;
	int		bl ;
	int		wlen = 0 ;
	const char	*tp ;
	const char	*bp ;
	char		bbuf[BOOKBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proctitlebook: ent b=%u\n",nbook) ;
#endif

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = shio_printf(lip->ofp,"\n") ;
	    wlen += rs ;
	}
#endif

	bbuf[0] = '\0' ;
	if ((bbl = locinfo_nlookup(lip,bbuf,blen,nbook)) >= 0) {
	    bp = bbuf ;
	    bl = bbl ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("proctitlebook: bn=>%t<\n",bbuf,bbl) ;
#endif

	    if (rs >= 0) {
	        const char	*w = lip->word[word_thebookof] ;
	        rs = shio_printf(lip->ofp,"%s\n",w) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && ((tp = strwhite(bbuf)) != NULL)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("proctitlebook: leader=>%t<\n",
	                bbuf,(tp - bbuf)) ;
#endif

	        bl -= ((tp + 1) - bbuf) ;
	        bp = (tp + 1) ;
	        if ((i = matcasestr(leaders,bbuf,(tp - bbuf))) >= 0) {
	            rs = shio_printf(lip->ofp,"%s\n",leadsubs[i]) ;
	            wlen += rs ;
	        }

	    } /* end if */

	    if (rs >= 0) {
	        rs = shio_printf(lip->ofp,"%t\n",bp,bl) ;
	        wlen += rs ;
	    }

	} else {

	    rs = shio_printf(lip->ofp,"book %u of the bible\n",nbook) ;
	    wlen += rs ;

	} /* end if */

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = shio_printf(lip->ofp,"\n") ;
	    wlen += rs ;
	}
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proctitlebook: ret rs=%d wlen=%i\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proctitlebook) */


static int proctitlechapter(PROGINFO *pip,int nbook,int nchapter)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*n ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("proctitlechapter: nbook=%u\n",nbook) ;
#endif

	n = lip->word[word_chapter] ;
	if (nbook == 19)
	    n = lip->word[word_psalm] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("proctitlechapter: n=%s\n",n) ;
#endif

	rs = shio_printf(lip->ofp,"%s %u.\n\n",n,nchapter) ;
	wlen += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("proctitlechapter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proctitlechapter) */


static int procout(PROGINFO *pip,BIBLEVERSE_CITE *qp,cchar bvbuf[],int bvlen)
{
	LOCINFO		*lip = pip->lip ;
	WORDFILL	w ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		cbl ;
	int		line = 0 ;
	int		wlen = 0 ;
	int		f_blank = FALSE ;
	const char	*fmt ;
	char		cbuf[COLBUFLEN + 1] ;

	if (bvlen <= 0)
	    goto ret0 ;

	cbl = MIN((lip->linelen - lip->indent),clen) ;

#if	CF_COOKIE
	fmt = "%%\n" ;
#else
	fmt = "" ;
#endif

#ifdef	COMMENT
	rs = shio_printf(lip->ofp,"%2u >%t<\n",
	    n,bvbuf,MIN(bvlen,20)) ;
#endif

/* print out any necessary separator */

	if ((lip->cout++ > 0) && (fmt[0] != '\0')) {
	    rs = shio_printf(lip->ofp,fmt) ;
	    wlen += rs ;
	} /* end if (separator) */

/* for psalm-119 only, place blank-line before each 8-verse stanza */

	if (rs >= 0) {
	    f_blank = (qp->b == 19) ;
	    f_blank = f_blank && (qp->c == 119) ;
	    f_blank = f_blank && (qp->v > 1) ;
	    f_blank = f_blank && (((qp->v - 1) % 8) == 0) ;
	    if (f_blank) {
	        rs = shio_printf(lip->ofp,"\n") ;
	        wlen += rs ;
	    }
	}

/* print out the text-data itself */

	if (rs >= 0) {
	    if ((rs = wordfill_start(&w,NULL,0)) >= 0) {

	        if ((rs = proclines(pip,&w,bvbuf,bvlen)) >= 0) {

	            while ((cl = wordfill_mklinefull(&w,cbuf,cbl)) > 0) {

	                rs = procoutline(pip,line,cbuf,cl) ;
	                wlen += rs ;

	                line += 1 ;

	                if (rs < 0) break ;
	            } /* end while (full lines) */

	            if (rs >= 0) {
	                if ((cl = wordfill_mklinepart(&w,cbuf,cbl)) > 0) {

	                    rs = procoutline(pip,line,cbuf,cl) ;
	                    wlen += rs ;

	                    line += 1 ;

	                } /* end if */
	            } /* end if (partial lines) */

	        } /* end if */

/* for verse-zeros, place blank-line after them */

	        if ((rs >= 0) && (qp->v == 0)) {
	            rs = shio_printf(lip->ofp,"\n") ;
	            wlen += rs ;
	        }

	        wordfill_finish(&w) ;
	    } /* end if (word-fill) */
	} /* end if (ok) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


/* ARGSUSED */
static int procoutline(PROGINFO *pip,int line,cchar *lp,int ll)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		indent ;
	int		wlen = 0 ;

	indent = MIN(lip->indent,NBLANKS) ;
	rs = shio_printf(lip->ofp,"%t%t\n", blanks,indent, lp,ll) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


static int proclines(PROGINFO *pip,WORDFILL *wp,cchar bvbuf[],int bvlen)
{
	int		rs = SR_OK ;

	if (strnpbrk(bvbuf,bvlen,"[]") != NULL) {
	    rs = procwords(pip,wp,bvbuf,bvlen) ;
	} else {
	    rs = wordfill_addlines(wp,bvbuf,bvlen) ;
	}

	return rs ;
}
/* end subroutine (proclines) */


static int procwords(PROGINFO *pip,WORDFILL *wp,cchar buf[],int buflen)
{
	int		rs = SR_OK ;
	int		len ;
	int		bl, sl, cl ;
	const char	*bp, *sp ;
	const char	*tp ;
	const char	*cp ;

	bp = buf ;
	bl = buflen ;
	while (bl > 0) {

	    sp = bp ;
	    sl = bl ;
	    len = bl ;
	    if ((tp = strnchr(bp,bl,'\n')) != NULL) {
	        len = ((tp + 1) - bp) ;
	        sp = bp ;
	        sl -= (tp - bp) ;
	    }

	    while ((cl = nextfield(sp,sl,&cp)) > 0) {

	        rs = procword(pip,wp,cp,cl) ;

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	        if (rs < 0) break ;
	    } /* end while */

	    bp += len ;
	    bl -= len ;

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (procwords) */


static int procword(PROGINFO *pip,WORDFILL *wp,cchar *cp,int cl)
{
	int		rs = SR_OK ;
	char		wordbuf[WORDBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	if (cl > 0) {

	    if (strnpbrk(cp,cl,"[]") != NULL) {

	        if (cp[0] == CH_LBRACK) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if (strnpbrk(cp,cl,"[]") != NULL) {
	            rs = mkwordclean(wordbuf,WORDBUFLEN,cp,cl) ;
	            cl = rs ;
	            cp = wordbuf ;
	        }

	    } /* end if */

	    if ((rs >= 0) && (cl > 0)) {
	        rs = wordfill_addword(wp,cp,cl) ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (procword) */


static int metawordsbegin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	const char	*wdbname = lip->wdbname ;

	if ((rs = biblemeta_open(&lip->wdb,pip->pr,wdbname)) >= 0) {
	    int		mi ;
	    int		len ;
	    int		size ;
	    char	wordbuf[WORDLEN + 1] ;
	    char	*wp ;

	    for (mi = 0 ; mi < biblemeta_overlast ; mi += 1) {

	        rs = biblemeta_get(&lip->wdb,mi,wordbuf,WORDLEN) ;
	        len = rs ;
	        if (rs < 0) break ;

	        size = (len+1) ;
	        rs = uc_malloc(size,&wp) ;
	        if (rs < 0) break ;

	        strwcpy(wp,wordbuf,len) ;

	        switch (mi) {
	        case biblemeta_chapter:
	            lip->word[word_chapter] = firstup(wp) ;
	            break ;
	        case biblemeta_psalm:
	            lip->word[word_psalm] = firstup(wp) ;
	            break ;
	        case biblemeta_bookindex:
	            lip->word[word_bookindex] = alldown(wp) ;
	            break ;
	        case biblemeta_page:
	            lip->word[word_page] = alldown(wp) ;
	            break ;
	        case biblemeta_booktitle:
	            lip->word[word_booktitle] = alldown(wp) ;
	            break ;
	        case biblemeta_thebookof:
	            lip->word[word_thebookof] = firstup(wp) ;
	            break ;
	        case biblemeta_book:
	            lip->word[word_book] = alldown(wp) ;
	            break ;
	        } /* end switch */

	    } /* end for */

	    if (rs < 0) {
	        metawordsfins(pip) ;
	        biblemeta_close(&lip->wdb) ;
	    }

	} /* end if (opened) */

	if ((rs < 0) && (rs != SR_NOMEM)) {
	    shio_printf(pip->efp,
	        "%s: problem loading from metaword DB (%d)\n",
	        pip->progname,rs) ;
	}

	return rs ;
}
/* end subroutine (metawordsbegin) */


static int metawordsfins(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		mi ;

	for (mi = 0 ; mi < word_overlast ; mi += 1) {
	    if (lip->word[mi] != NULL) {
	        rs1 = uc_free(lip->word[mi]) ;
	        if (rs >= 0) rs = rs1 ;
	        lip->word[mi] = NULL ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (metawordsfins) */


static int metawordsend(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = metawordsfins(pip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.wdb) {
	    lip->open.wdb = FALSE ;
	    rs1 = biblemeta_close(&lip->wdb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (metawordsend) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->count = -1 ;
	lip->max = -1 ;
	lip->f.bookchapters = TRUE ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.wdb) {
	    lip->open.wdb = FALSE ;
	    rs1 = biblemeta_close(&lip->wdb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.ndb) {
	    lip->open.ndb = FALSE ;
	    rs1 = biblebook_close(&lip->ndb) ;
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


static int locinfo_nlookup(LOCINFO *lip,char *rbuf,int rlen,int bi)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	rbuf[0] = '\0' ;
	if (! lip->open.ndb) {
	    rs = biblebook_open(&lip->ndb,pip->pr,lip->ndbname) ;
	    lip->open.ndb = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = biblebook_get(&lip->ndb,bi,rbuf,rlen) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_nlookup) */


static int locinfo_bookmatch(LOCINFO *lip,cchar *mbuf,int mlen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		bi = 0 ;

	if (! lip->open.ndb) {
	    rs = biblebook_open(&lip->ndb,pip->pr,lip->ndbname) ;
	    lip->open.ndb = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = biblebook_match(&lip->ndb,mbuf,mlen) ;
	    bi = rs ;
	}

	return (rs >= 0) ? bi : rs ;
}
/* end subroutine (locinfo_bookmatch) */


static int mkwordclean(char *wordbuf,int wordbuflen,cchar *cp,int cl)
{
	int		i = 0 ;

	while ((i < wordbuflen) && cl && cp[0]) {
	    if ((cp[0] != '[') && (cp[0] != ']')) {
	        wordbuf[i++] = cp[0] ;
	    }
	    cp += 1 ;
	    cl -= 1 ;
	} /* end while */
	wordbuf[i] = '\0' ;

	return i ;
}
/* end subroutine (mkwordclean) */


static char *firstup(char *cp)
{
	int		ch ;
	int		nch ;

	if (*cp != '\0') {
	    ch = (*cp & 0xff) ;
	    nch = CHAR_TOUC(ch) ;
	    if (ch != nch) *cp = nch ;
	    alldown(cp + 1) ;
	}

	return cp ;
}
/* end subroutine (firstup) */


static char *alldown(char *cp)
{
	int		ch ;
	int		nch ;

	if (*cp != '\0') {
	    int	i ;
	    for (i = 0 ; cp[i] ; i += 1) {
	        ch = (cp[i] & 0xff) ;
	        nch = CHAR_TOLC(ch) ;
	        if (ch != nch) cp[i] = nch ;
	    } /* end for */
	}

	return cp ;
}
/* end subroutine (alldown) */


