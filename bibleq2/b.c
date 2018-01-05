/* b_bibleq */

/* query the bible database using words as the query keys */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_COOKIE	0		/* use cookie as separator */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ bibleq <word(s)>


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

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<estrings.h>
#include	<field.h>
#include	<vecstr.h>
#include	<wordfill.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_bibleq.h"
#include	"defs.h"
#include	"biblebook.h"
#include	"biblepara.h"
#include	"bibleq.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	BVBUFLEN
#define	BVBUFLEN	512		/* maximum bibleq length (?) */
#endif

#define	COLBUFLEN	(COLUMNS + 10)

#define	NBLANKS		20

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	ndigits(int,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_adduniqs(vecstr *,cchar *,int) ;
extern int	field_wordphrase(FIELD *,const uchar *,char *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
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

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		n:1 ;
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		nverses:1 ;
	uint		ndb:1 ;
	uint		bookname:1 ;
	uint		interactive:1 ;
	uint		prefix:1 ;
	uint		separate:1 ;
	uint		para:1 ;
	uint		clump:1 ;
} ;

struct locinfo {
	BIBLEBOOK	ndb ;		/* bible-book-name DB */
	BIBLEQ		*dbp ;
	BIBLEPARA	pdb ;
	void		*ofp ;
	cchar		*ndbname ;	/* name-db */
	cchar		*vdbname ;	/* verse-db name */
	cchar		*pdbname ;	/* paragraph-db name */
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	int		linelen ;
	int		indent ;
	int		nverses ;
	int		count, max, precision ;
	int		cout ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_nlookup(LOCINFO *,int,char *,int) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_ispara(LOCINFO *,BIBLEQ_Q *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,cchar *,int) ;
static int	procspec(PROGINFO *,vecstr *) ;
static int	procoutcite(PROGINFO *,BIBLEQ_Q *,int) ;
static int	procout(PROGINFO *,BIBLEQ_QUERY *,cchar *,int) ;
static int	procoutline(PROGINFO *,cchar *,int) ;


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
	"pdb",
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
	argopt_pdb,
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
	"prefix",
	"separate",
	"para",
	"clump",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_bookname,
	akoname_interactive,
	akoname_prefix,
	akoname_separate,
	akoname_para,
	akoname_clump,
	akoname_overlast
} ;

static const char	blanks[NBLANKS+1] = "                    " ;

static const uchar	aterms[] = {
	0x00, 0x0A, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int b_bibleq(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_bibleq) */


int p_bibleq(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_bibleq) */


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
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ndbname = NULL ;
	cchar		*pdbname = NULL ;
	cchar		*vdbname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("bibleq: starting DFD=%d\n",rs) ;
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

/* keyword match or only key letters? */

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

/* BibleName DB name */
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

/* paragraph-db name */
	                case argopt_pdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pdbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pdbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* BibleVverse DB name */
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

	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.nverses = TRUE ;
	                                lip->final.nverses = TRUE ;
	                                rs = optvalue(argp,argl) ;
	                                lip->nverses = rs ;
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

	                    case 'p':
	                        lip->f.prefix = TRUE ;
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

#if	CF_DEBUGS
	debugprintf("bibleq: debuglevel=%u\n",pip->debuglevel) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bibleq: debuglevel=%u\n",pip->debuglevel) ;
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

/* argument processing */

	if ((rs >= 0) && (lip->nverses <= 0) && (argval != NULL)) {
	    if ((rs = optvalue(argval,-1)) >= 0) {
	        lip->have.nverses = TRUE ;
	        lip->final.nverses = TRUE ;
	        lip->nverses = rs ;
	    }
	}

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ndbname == NULL) ndbname = getourenv(envv,VARNDB) ;
	lip->ndbname = ndbname ;

/* paragraph-db name */

	if (pdbname == NULL) pdbname = getourenv(envv,VARPDB) ;
	lip->pdbname = pdbname ;

/* verse-db name */

	if (vdbname == NULL) vdbname = getourenv(envv,VARVDB) ;
	if (vdbname == NULL) vdbname = VDBNAME ;

/* display some answers */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: ndb=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,
		((ndbname != NULL) ? ndbname : "NULL")) ;
	    fmt = "%s: pdb=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,
	        ((pdbname != NULL) ? pdbname : "NULL")) ;
	    fmt = "%s: vdb=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,
	        ((vdbname != NULL) ? vdbname : "NULL")) ;
	}

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

	if ((lip->nverses < 1) && (! lip->have.nverses)) lip->nverses = 1 ;

	if (lip->nverses < 0) lip->nverses = 1 ;

/* process */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bibleq: bibleq_open() vdbname=%s\n",
	        vdbname) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    BIBLEQ	vdb ;
	    lip->dbp = &vdb ;
	    if ((rs = bibleq_open(&vdb,pip->pr,vdbname)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bibleq: bibleq_open() rs=%d\n",rs) ;
#endif

	        if (lip->f.audit) {
	            rs = bibleq_audit(&vdb) ;
	            if (pip->debuglevel > 0) {
			cchar	*pn = pip->progname ;
			cchar	*fmt = "%s: bibleq DB audit (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
		    }
	        }

	        if (rs >= 0) {
	            rs = procargs(pip,&ainfo,&pargs,ofname,afname) ;
	        }

	        rs1 = bibleq_close(&vdb) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: could not load bibleq DB (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: vdb=%s\n",pn,vdbname) ;
	        ex = EX_CONFIG ;
	    }
	    lip->dbp = NULL ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* finish up */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: could not perform function (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
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
	    debugprintf("bibleq: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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
	    debugprintf("bibleq: mallout=%u\n",mo-mo_start) ;
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

	fmt = "%s: USAGE> %s [<word(s)> ...] [-w <width>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-ndb <booknamedb>] [-vdb <versedb>]\n" ;
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

	                case akoname_prefix:
	                    if (! lip->final.prefix) {
	                        lip->have.prefix = TRUE ;
	                        lip->final.prefix = TRUE ;
	                        lip->f.prefix = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.prefix = (rs > 0) ;
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

	                case akoname_para:
	                    if (! lip->final.para) {
	                        lip->have.para = TRUE ;
	                        lip->final.para = TRUE ;
	                        lip->f.para = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.para = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_clump:
	                    if (! lip->final.clump) {
	                        lip->have.clump = TRUE ;
	                        lip->final.clump = TRUE ;
	                        lip->f.clump = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.clump = (rs > 0) ;
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
	    VECSTR	qstr ;
	    const int	opts = VECSTR_OCOMPACT ;
	    int		cl ;
	    int		pan = 0 ;
	    cchar	*cp ;
	    lip->ofp = ofp ;

	    if ((rs = vecstr_start(&qstr,5,opts)) >= 0) {

	        if (rs >= 0) {
	            const int	argc = aip->argc ;
	            int		ai ;
	            int		f ;
	            cchar	**argv = aip->argv ;
	            for (ai = 1 ; ai < argc ; ai += 1) {
	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = argv[ai] ;
			    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        if (lip->f.clump) {
	                            rs = vecstr_adduniqs(&qstr,cp,-1) ;
	                        } else {
	                            rs = vecstr_adduniq(&qstr,cp,-1) ;
				}
	                    }
	                } /* end if (non-zero) */
	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end for (looping through positional arguments) */
	        } /* end if (ok) */

	        if ((rs >= 0) && (pan > 0)) {
	            if ((rs = vecstr_count(&qstr)) > 0) {
	                rs = procspec(pip,&qstr) ;
	                wlen += rs ;
	            }
	        }

	        rs1 = vecstr_finish(&qstr) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (qstr) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

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
		    fmt = "%s: inaccesible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (argument-file arguments) */

	    lip->ofp = NULL ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bibleq/procargs: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	VECSTR		qstr ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (sp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bibleq/procspecs: ent sl=%d\n",sl) ;
#endif

	if (lip->f.interactive) lip->cout = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = vecstr_start(&qstr,5,0)) >= 0) {
	    FIELD	fsb ;
	    int		c = 0 ;

	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	        const int	flen = sl ;
	        const uchar	*at = aterms ;
	        char		*fbuf ;
	        if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	            int		fl ;
	            while ((fl = field_wordphrase(&fsb,at,fbuf,flen)) >= 0) {
	                if (fl > 0) {
	                    rs = vecstr_adduniq(&qstr,fbuf,fl) ;
	                    c += ((rs < INT_MAX) ? 1 : 0) ;
	                }
	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while */
	            uc_free(fbuf) ;
	        } /* end if (m-a) */
	        field_finish(&fsb) ;
	    } /* end if (field) */

	    if ((rs >= 0) && (c > 0)) {
	        rs = procspec(pip,&qstr) ;
	        wlen += rs ;
	    }

	    rs1 = vecstr_finish(&qstr) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (query-strings) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bibleq/procspecs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,vecstr *qsp)
{
	LOCINFO		*lip = pip->lip ;
	BIBLEQ		*bqp ;
	BIBLEQ_CUR	cur ;
	BIBLEQ_CITE	q ;
	int		rs ;
	int		rs1 ;
	int		qopts = 0 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bibleq/procspec: ent\n") ;
#endif

	if (qsp == NULL) return SR_FAULT ;

	if (lip->f.prefix) qopts |= BIBLEQ_OPREFIX ;

	bqp = lip->dbp ;
	if ((rs = bibleq_curbegin(bqp,&cur)) >= 0) {
	    const int	comlen = BVBUFLEN ;
	    int		cbl ;
	    cchar	**qkeya ;
	    char	combuf[BVBUFLEN + 1] ;

	    vecstr_getvec(qsp,&qkeya) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        int	i ;
	        for (i = 0 ; qkeya[i] != NULL ; i += 1)
	            debugprintf("bibleq/procspec: sk=>%s<\n",qkeya[i]) ;
	    }
#endif /* CF_DEBUG */

	    if ((rs = bibleq_lookup(bqp,&cur,qopts,qkeya)) >= 0) {
	        const int	ntags = rs ;

	        while ((rs >= 0) && (ntags > 0)) {

	            cbl = bibleq_read(bqp,&cur,&q,combuf,comlen) ;
	            if (cbl == SR_NOTFOUND) break ;
	            rs = cbl ;
	            if (rs >= 0) {
	                rs = procoutcite(pip,&q,0) ;
	                wlen += rs ;
	            }

	            if (rs >= 0) {
	                rs = procout(pip,&q,combuf,cbl) ;
	                wlen += rs ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	        } /* end while */

	    } /* end if (lookup) */

	    rs1 = bibleq_curend(bqp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bibleq/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procoutcite(PROGINFO *pip,BIBLEQ_Q *qp,int edays)
{
	LOCINFO		*lip = pip->lip ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		b = qp->b ;
	int		c = qp->c ;
	int		v = qp->v ;
	int		wlen = 0 ;
	int		f_havebook = FALSE ;
	cchar		*fmt ;
	char		cbuf[COLBUFLEN + 1] ;

#if	CF_COOKIE
	fmt = "%%\n" ;
#else
	fmt = "\n" ;
#endif

/* print out any necessary separator */

	if (lip->f.separate && (lip->cout++ > 0)) {
	    rs = shio_printf(lip->ofp,fmt) ;
	    wlen += rs ;
	} /* end if (separator) */

	if (rs >= 0) {

/* print out the text-data itself */

	    if (lip->f.bookname) {
	        const int	blen = BIBLEBOOK_LEN ;
	        int		bbl ;
	        char	bbuf[BIBLEBOOK_LEN + 1] ;

	        if ((bbl = locinfo_nlookup(lip,qp->b,bbuf,blen)) > 0) {

	            f_havebook = TRUE ;
	            fmt = (edays > 0) ? "%t %u:%u (%u)" : "%t %u:%u" ;
	            rs = bufprintf(cbuf,clen,fmt,bbuf,bbl,c,v,(edays+1)) ;
	            cl = rs ;
	            if (rs >= 0) {
	                rs = shio_print(lip->ofp,cbuf,cl) ;
	                wlen += rs ;
	            }

	        } /* end if (nlookup) */

	    } /* end if (book-name) */

	    if ((rs >= 0) && (! f_havebook)) {

	        fmt = (edays > 0) ? "%u:%u:%u (%u)" : "%u:%u:%u" ;
	        rs = bufprintf(cbuf,clen,fmt,b,c,v,(edays+1)) ;
	        cl = rs ;
	        if (rs >= 0) {
	            rs = shio_print(lip->ofp,cbuf,cl) ;
	            wlen += rs ;
	        }

	    } /* end if (type of book-name display) */

	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutcite) */


static int procout(PROGINFO *pip,BIBLEQ_Q *qp,cchar *vp,int vl)
{
	LOCINFO		*lip = pip->lip ;
	WORDFILL	w ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl = vl ;
	int		cl ;
	int		cbl ;
	int		line = 0 ;
	int		wlen = 0 ;
	int		f_p = FALSE ;
	cchar		*sp = vp ;
	char		cbuf[COLBUFLEN + 1] ;

	cbl = MIN((lip->linelen - lip->indent),clen) ;

	if ((rs >= 0) && lip->f.para) {
	    rs = locinfo_ispara(lip,qp) ;
	    f_p = (rs > 0) ;
	}

/* print out the text-data itself */

	if (f_p) sp = NULL ;

	if (rs >= 0) {
	    if ((rs = wordfill_start(&w,sp,sl)) >= 0) {

	        if (f_p) {
	            rs = wordfill_addword(&w,"¶",1) ;
	            if (rs >= 0)
	                rs = wordfill_addlines(&w,vp,vl) ;
	        }

	        while (rs >= 0) {
	            cl = wordfill_mklinefull(&w,cbuf,cbl) ;
	            if ((cl == 0) || (cl == SR_NOTFOUND)) break ;
	            rs = cl ;

	            if (rs >= 0) {
	                rs = procoutline(pip,cbuf,cl) ;
	                wlen += rs ;
	                line += 1 ;
	            }

	        } /* end while (full lines) */

	        if (rs >= 0) {
	            if ((cl = wordfill_mklinepart(&w,cbuf,cbl)) > 0) {
	                rs = procoutline(pip,cbuf,cl) ;
	                wlen += rs ;
	                line += 1 ;
	            } else if (cl != SR_NOTFOUND)
	                rs = cl ;
	        } /* end if (partial lines) */

	        rs1 = wordfill_finish(&w) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (word-fill) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procoutline(PROGINFO *pip,cchar *lp,int ll)
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


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->count = -1 ;
	lip->max = -1 ;
	lip->f.separate = TRUE ;
	lip->indent = OPT_INDENT ;
	lip->f.bookname = OPT_BOOKNAME ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.para) {
	    lip->open.para = FALSE ;
	    rs1 = biblepara_close(&lip->pdb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->have.ndb && lip->f.ndb) {
	    lip->have.ndb = FALSE ;
	    lip->f.ndb = FALSE ;
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


static int locinfo_nlookup(LOCINFO *lip,int bi,char nbuf[],int nlen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bibleq/locinfo_nlookup: bi=%u\n",bi) ;
#endif

	nbuf[0] = '\0' ;
	if (! lip->have.ndb) {
	    lip->have.ndb = TRUE ;
	    rs = biblebook_open(&lip->ndb,pip->pr,lip->ndbname) ;
	    lip->f.ndb = (rs >= 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bibleq/locinfo_nlookup: "
	            "biblebook_open() rs=%d\n", rs) ;
#endif

	} /* end if */

	if (rs >= 0) {
	    if (lip->f.ndb) {
	        rs = biblebook_get(&lip->ndb,bi,nbuf,nlen) ;
	        len = rs ;
	    } else
	        rs = SR_NOTFOUND ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bibleq/locinfo_nlookup: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_nlookup) */


static int locinfo_ispara(LOCINFO *lip,BIBLEQ_Q *qp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (qp == NULL) return SR_FAULT ;

	if (lip->f.para) {

	    if (! lip->open.para) {
	        rs = biblepara_open(&lip->pdb,pip->pr,lip->pdbname) ;
	        lip->open.para = (rs >= 0) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_bibleverse/locinfo_ispara: "
	                "biblepara_open() rs=%d\n",rs) ;
#endif
	        if (isNotPresent(rs)) rs = SR_OK ;
	    }

	    if (rs >= 0) {
	        BIBLEPARA_QUERY	pq ;
	        pq.b = qp->b ;
	        pq.c = qp->c ;
	        pq.v = qp->v ;
	        rs = biblepara_ispara(&lip->pdb,&pq) ;
	        f = (rs > 0) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_bibleverse/locinfo_ispara: "
	                "biblepara_ispara() rs=%d\n",rs) ;
#endif
	    }

	} /* end if (ispara) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_ispara) */


