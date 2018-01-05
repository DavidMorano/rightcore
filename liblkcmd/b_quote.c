/* b_quote */

/* query the bible database using words as the query keys */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_COOKIE	0		/* use cookie as separator */


/* revision history:

	= 2004-06-02, David A­D­ Morano
	This subroutine was originally written and is the QUOTE built-in
	command for KSH.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ quote <word(s)>


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
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<field.h>
#include	<char.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<wordfill.h>
#include	<prsetfname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_quote.h"
#include	"defs.h"
#include	"biblebook.h"
#include	"quote.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum quote length (?) */
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(6 * MAXPATHLEN)
#endif

#define	COLBUFLEN	(COLUMNS + 10)
#define	BOOKBUFLEN	60		/* length of book-name */
#define	WPBUFLEN	(20 * 100)

#define	NBLANKS		20

#define	PO_NAME		"name"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfcasesub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	ndigits(int,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
extern int	field_wordphrase(FIELD *,const uchar *,char *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUG || CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


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
	uint		quotedirs:1 ;
	uint		quotenames:1 ;
} ;

struct locinfo {
	void		*ofp ;
	const char	*homedname ;
	const char	*org ;
	const char	*name ;
	const char	*fullname ;
	char		*ndbname ;
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	BIBLEBOOK	ndb ;		/* bible-book-name DB */
	QUOTE		qdb ;
	vecstr		quotedirs ;
	vecstr		quotenames ;
	int		linelen ;
	int		indent ;
	int		nverses ;
	int		count, max, precision ;
	int		cout ;
	int		ncites ;
} ;

struct config_flags {
	uint		p:1 ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMFILE	params ;
	EXPCOOK		cooks ;
	struct config_flags	f ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	config_init(struct config *,PROGINFO *,const char *) ;
static int	config_check(struct config *) ;
static int	config_read(struct config *) ;
static int	config_free(struct config *) ;
static int	config_files(struct config *,vecstr *) ;
static int	config_loadcooks(struct config *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procspecs(PROGINFO *,const char *,int) ;
static int	procspec(PROGINFO *,vecstr *) ;
static int	procnames(PROGINFO *,PARAMOPT *) ;

static int	procout(PROGINFO *,const char *,int) ;
static int	procoutline(PROGINFO *,int,const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_nlookup(LOCINFO *,int,char *,int) ;


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
	"vdb",
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
	argopt_vdb,
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
	"interactive",
	"prefix",
	"separate",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_interactive,
	akoname_prefix,
	akoname_separate,
	akoname_overlast
} ;

static const char	*params[] = {
	"logsize",
	"logfile",
	"quotedirs",
	"quotenames",
	NULL
} ;

enum params {
	param_logsize,
	param_logfile,
	param_quotedirs,
	param_quotenames,
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


int b_quote(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_quote) */


int p_quote(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_quote) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO	li, *lip = &li ;
	struct config	co ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	USERINFO	u ;
	SHIO		errfile ;
	SHIO		outfile, *ofp = &outfile ;
	VECSTR		qstr ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		n, i, j ;
	int		size, len ;
	int		opts ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*configfname = NULL ;
	const char	*ndbname = NULL ;
	const char	*vdbname = NULL ;
	const char	*cp ;
	char		userbuf[USERINFO_LEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_quote: starting DFD=%d\n",rs) ;
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

	                    case 'c':
	                    case 'n':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    lip->have.quotenames = TRUE ;
				    lip->final.quotenames = TRUE ;
	                            rs = paramopt_loads(&aparams,
	                                PO_NAME,argp,argl) ;
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

	if (rs < 0) {
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_quote: debuglevel=%u\n",pip->debuglevel) ;
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

	if ((lip->nverses <= 0) && (argvalue >= 0)) {
	    lip->have.nverses = TRUE ;
	    lip->final.nverses = TRUE ;
	    lip->nverses = argvalue ;
	}

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ndbname == NULL) ndbname = getourenv(envv,VARNDB) ;

#ifdef	OPTIONAL
	if (ndbname == NULL)
	    ndbname = NDBNAME ;
#endif

	lip->ndbname = ndbname ;

	if (vdbname == NULL)
	    vdbname = getourenv(envv,VARVDB) ;

	if (vdbname == NULL)
	    vdbname = VDBNAME ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: ndb=%s\n",
		pip->progname,((ndbname != NULL) ? ndbname : "NULL")) ;
	    shio_printf(pip->efp,"%s: vdb=%s\n",
		pip->progname,((vdbname != NULL) ? vdbname : "NULL")) ;
	}

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

	if ((lip->nverses < 1) && (! lip->have.nverses)) lip->nverses = 1 ;

	if (lip->nverses < 0) lip->nverses = 1 ;

	if (rs < 0) goto badarg ;

/* identification */

	rs = userinfo_start(&u,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduserinfo ;
	}

	pip->username = u.username ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->name = u.name ;
	pip->logid = u.logid ;
	pip->pid = u.pid ;

	lip->homedname = u.homedname ;
	lip->org = u.organization ;
	lip->name = u.name ;
	lip->fullname = u.fullname ;

/* calendar directories */

	if ((rs >= 0) && (! lip->final.quotedirs)) {
	if ((cp = getourenv(envv,VARCALDNAMES)) != NULL) {
	    lip->final.quotedirs = TRUE ;
	    lip->have.quotedirs = TRUE ;
	    rs = locinfo_loaddirs(lip,cp,-1) ;
	} /* end if */
	}

/* calendar names */

	if ((rs >= 0) && lip->have.quotenames) {
	    rs = procnames(pip,&aparams) ;
	}

	if ((rs >= 0) && (! lip->final.quotenames)) {
	    if ((cp = getourenv(envv,VARQUOTENAMES)) != NULL) {
	        lip->final.quotenames = TRUE ;
	        lip->have.quotenames = TRUE ;
	        rs = locinfo_loadnames(lip,cp,-1) ;
	    } /* end if */
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinitload ;
	}

/* configuration */

	if (configfname == NULL)
	    configfname = getourenv(envv,VARCONFIG) ;

	if ((pip->debuglevel > 0) && (configfname != NULL))
	    shio_printf(pip->efp,"%s: conf=%s\n",
		pip->progname,configfname) ;

	pip->config = &co ;
	rs = config_init(pip->config,pip,configfname) ;
	pip->open.config = (rs > 0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinitconfig ;
	}

/* more initialization */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_calyear: process\n") ;
	    shio_flush(pip->efp) ;
	}
#endif

	{
	    const char	**quotedirs = NULL ;
	    const char	**quotenames = NULL ;

	    if (rs >= 0) {
		rs = locinfo_getstuff(lip,&quotedirs,&quotenames) ;
	    }

	    if ((rs >= 0) && (pip->debuglevel > 0)) {
		if (quotedirs != NULL) {
		    for (i = 0 ; quotedirs[i] != NULL ; i += 1)
	                shio_printf(pip->efp,"%s: caldir=%s\n",
			    pip->progname,quotedirs[i]) ;
		}
		if (quotenames != NULL) {
		    for (i = 0 ; quotenames[i] != NULL ; i += 1)
	                shio_printf(pip->efp,"%s: calname=%s\n",
			    pip->progname,quotenames[i]) ;
		}
	    } /* end if (debug printout) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_calyear: quotedirs:\n") ;
	    if (quotedirs != NULL) {
		for (i = 0 ; quotedirs[i] != NULL ; i += 1)
	            debugprintf("b_calyear: caldir%02u=%s\n",i,quotedirs[i]) ;
	    }
	    debugprintf("b_calyear: quotenames:\n") ;
	    if (quotenames != NULL) {
		for (i = 0 ; quotenames[i] != NULL ; i += 1)
	            debugprintf("b_calyear: calname%02u=%s\n",i,quotenames[i]) ;
	    }
	}
#endif /* CF_DEBUG */

/* process */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_quote: quote_open() vdbname=%s\n",
		vdbname) ;
#endif

	rs = quote_open(&lip->qdb,pip->pr,quotedirs,quotenames) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_quote: quote_open() rs=%d\n",rs) ;
#endif

	} /* end block */

	if (rs < 0) {
	    ex = EX_CONFIG ;
	    shio_printf(pip->efp,
	        "%s: could not load quote DB (%d)\n",
	        pip->progname,rs) ;
	    goto badquoteopen ;
	}

	if (lip->f.audit) {
	    rs = quote_audit(&lip->qdb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_quote: quote_audit() rs=%d\n",rs) ;
#endif

	    if ((rs < 0) || (pip->debuglevel > 0))
	        shio_printf(pip->efp,
	            "%s: quote DB audit (%d)\n",
	            pip->progname,rs) ;
	    if (rs < 0)
		goto badaudit ;
	}

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_quote: done w/ opening output rs=%d\n",rs) ;
#endif

/* give the answers */

	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&qstr,5,opts)) >= 0) {

	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            pan += 1 ;
	            rs = vecstr_adduniq(&qstr,cp,-1) ;
		}

		    if (rs >= 0) rs = lib_sigterm() ;
		    if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end for (looping through positional arguments) */

	    if ((rs >= 0) && (pan > 0)) {

		rs1 = vecstr_count(&qstr) ;
		if (rs1 > 0)
	    	    rs = procspec(pip,&qstr) ;

	    } /* end if */

	    vecstr_finish(&qstr) ;
	} /* end if (positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
			if (cp[0] != '#') {
	            	    pan += 1 ;
		    	    rs = procspecs(pip,cp,cl) ;
			}
		    }

		    if (rs >= 0) rs = lib_sigterm() ;
		    if (rs >= 0) rs = lib_sigintr() ;
		    if (rs < 0) break ;
	        } /* end while */

	        rs1 = shio_close(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if */

	    if (rs < 0) {
		ex = EX_NOINPUT ;
	        shio_printf(pip->efp,
		    "%s: argument-list file input unavailable (%d)\n",
			pip->progname,rs) ;
	    }

	} /* end if (afile arguments) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,"%s: inaccessible output (%d)\n",
	        pip->progname,rs) ;
	}

/* done */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_quote: done rs=%d ex=%u\n",rs,ex) ;
#endif

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

badaudit:
	quote_close(&lip->qdb) ;

badquoteopen:
	if (pip->open.config) {
	    pip->open.config = FALSE ;
	    config_free(pip->config) ;
	}

badinitconfig:
badinitload:
	userinfo_finish(&u) ;

baduserinfo:
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
	    debugprintf("b_quote: mallout=%u\n",mo-mo_start) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (mainsub) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->count = -1 ;
	lip->max = -1 ;
	lip->f.separate = TRUE ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

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


static int locinfo_nlookup(LOCINFO *lip,int bi,char *buf,int buflen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_quote/locinfo_nlookup: bi=%u\n",bi) ;
#endif

	buf[0] = '\0' ;
	if (! lip->have.ndb) {
	    lip->have.ndb = TRUE ;
	    rs = biblebook_open(&lip->ndb,pip->pr,lip->ndbname) ;
	    lip->f.ndb = (rs >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_quote/locinfo_nlookup: biblebook_open() rs=%d\n",
		rs) ;
#endif

	} /* end if */

	if (rs >= 0) {
	    if (lip->f.ndb) {
	        rs = biblebook_get(&lip->ndb,bi,buf,buflen) ;
	        len = rs ;
	    } else
		rs = SR_NOTFOUND ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_quote/locinfo_nlookup: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_nlookup) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<word(s) ...] [-w <width>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-ndb <booknamedb>] [-vdb <versedb>] \n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* configuration maintenance */
static int config_init(cfp,pip,configfname)
struct config	*cfp ;
PROGINFO	*pip ;
const char	*configfname ;
{
	EXPCOOK		*ckp ;
	PARAMFILE	*pp ;
	vecstr		files ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	char		*fp ;

	if (cfp == NULL)
	    return SR_FAULT ;

	memset(cfp,0,sizeof(struct config)) ;

	cfp->pip = pip ;
	pp = &cfp->params ;
	ckp = &cfp->cooks ;
	rs = paramfile_open(pp,pip->envv,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: paramfile_open() rs=%d \n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: configfname=%s\n",configfname) ;
#endif

	if ((configfname != NULL) && (configfname[0] != '\0')) {

	    rs1 = perm(configfname,-1,-1,NULL,R_OK) ;
	    if (rs1 >= 0) {
		c += 1 ;
		rs = paramfile_fileadd(pp,configfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: 1 paramfile_fileadd() rs=%d \n",rs) ;
#endif

	    }

	} else {

	    if ((rs = vecstr_start(&files,2,0)) >= 0) {

	        rs = config_files(cfp,&files) ;
		if (rs >= 0) {

		    for (i = 0 ; vecstr_get(&files,i,&fp) >= 0 ; i += 1) {
			if (fp == NULL) continue ;
			c += 1 ;
	                rs = paramfile_fileadd(pp,fp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: 2 paramfile_fileadd() rs=%d \n",rs) ;
#endif

			if (rs < 0)
			    break ;
		    } /* end for */

		} /* end if */

	        vecstr_finish(&files) ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: mid rs=%d c=%u\n",rs,c) ;
#endif

	if ((rs < 0) || (c == 0))
	    goto bad1 ;

	rs = expcook_start(ckp) ;
	if (rs < 0)
	    goto bad1 ;

	rs = config_loadcooks(cfp) ;
	if (rs < 0)
	    goto bad2 ;

	cfp->f.p = TRUE ;
	rs = config_read(cfp) ;
	if (rs < 0)
	    goto bad2 ;

ret1:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;

/* bad stuff */
bad2:
	expcook_finish(ckp) ;

bad1:
	paramfile_close(&cfp->params) ;

bad0:
	goto ret1 ;
}
/* end subroutine (config_init) */


static int config_files(cfp,flp)
struct config	*cfp ;
vecstr		*flp ;
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO	*lip ;
	vecstr		sv ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	*cfn = CONFIGFNAME ;
	char		tmpfname[MAXPATHLEN + 1] ;

	lip = pip->lip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("config_files: ent\n") ;
	    debugprintf("config_files: pr=%s\n",pip->pr) ;
	    debugprintf("config_files: userhome=%s\n",lip->homedname) ;
	}
#endif

	tmpfname[0] = '\0' ;
	rs = vecstr_start(&sv,4,0) ;
	if (rs < 0)
	    goto ret0 ;

	vecstr_envadd(&sv,"r",pip->pr,-1) ;

	vecstr_envadd(&sv,"e","etc",-1) ;

	vecstr_envadd(&sv,"n",pip->searchname,-1) ;

	if (rs >= 0) {
	    rs1 = permsched(schedconf,&sv, tmpfname,MAXPATHLEN, cfn,R_OK) ;
	    if (rs1 >= 0) {
	        c += 1 ;
	        rs = vecstr_add(flp,tmpfname,rs1) ;
	    }
	}

	if (rs >= 0)
	    vecstr_envset(&sv,"r",lip->homedname,-1) ;

	if (rs >= 0) {
	    rs1 = permsched(schedconf,&sv, tmpfname,MAXPATHLEN, cfn,R_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("config_files: user permsched() rs=%d\n",rs1) ;
	    debugprintf("config_files: tmpfname=%s\n",tmpfname) ;
	}
#endif

	    if (rs1 >= 0) {
	        c += 1 ;
	        rs = vecstr_add(flp,tmpfname,rs1) ;
	    }
	}

	if ((rs >= 0) && (pip->debuglevel > 0) && (c > 0)) {
	    int		i ;
	    char	*cp ;
	    for (i = 0 ; vecstr_get(flp,i,&cp) >= 0 ; i += 1) {
		if (cp == NULL) continue ;
	        shio_printf(pip->efp,"%s: conf=%s\n",pip->progname,cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_files: conf=%s\n",cp) ;
#endif

	    }
	} /* end if */

ret1:
	vecstr_finish(&sv) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_files: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_files) */


static int config_loadcooks(cfp)
struct config	*cfp ;
{
	PROGINFO	*pip = cfp->pip ;
	EXPCOOK		*ckp ;
	int		rs = SR_OK ;

	ckp = &cfp->cooks ;

	expcook_add(ckp,"P",pip->progname ,-1) ;

	expcook_add(ckp,"S",pip->searchname,-1) ;

	expcook_add(ckp,"N",pip->nodename,-1) ;

	expcook_add(ckp,"D",pip->domainname,-1) ;

	{
	    int		hnl ;
	    char	hostname[MAXHOSTNAMELEN + 1] ;
	    hnl = snsds(hostname,MAXHOSTNAMELEN,pip->nodename,pip->domainname) ;
	    expcook_add(ckp,"H",hostname,hnl) ;
	}

	expcook_add(ckp,"R",pip->pr,-1) ;

	rs = expcook_add(ckp,"U",pip->username,-1) ;

	return rs ;
}
/* end subroutine (config_loadcooks) */


static int config_check(cfp)
struct config	*cfp ;
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_NOTOPEN ;

	if (cfp == NULL)
	    return SR_FAULT ;

	if (cfp->f.p) {
	    if ((rs = paramfile_check(&cfp->params,pip->daytime)) > 0)
	        rs = config_read(cfp) ;
	}

	return rs ;
}
/* end subroutine (config_check) */


static int config_free(cfp)
struct config	*cfp ;
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_NOTOPEN ;

	if (cfp == NULL)
	    return SR_FAULT ;

	if (cfp->f.p) {

	    expcook_finish(&cfp->cooks) ;

	    rs = paramfile_close(&cfp->params) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_free: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_free) */


static int config_read(cfp)
struct config	*cfp ;
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO	*lip ;
	PARAMFILE	*pfp ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	EXPCOOK		*ckp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		kl, vl ;
	int		el ;
	int		v ;
	char		pbuf[PBUFLEN + 1] ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;
	const char	*ccp ;
	const char	*kp, *vp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	pfp = &cfp->params ;
	ckp = &cfp->cooks ;
	if (cfp == NULL)
	    return SR_FAULT ;

	lip = pip->lip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",cfp->f.p) ;
#endif

	if (! cfp->f.p) {
	    rs = SR_NOTOPEN ;
	    goto ret0 ;
	}

	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    char	*pr = pip->pr ;

	while (rs >= 0) {
	    kl = paramfile_enum(pfp,&cur,&pe,pbuf,PBUFLEN) ;
	    if (kl <= 0) break ;

	    kp = pe.key ;
	    vp = pe.value ;
	    vl = pe.vlen ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_read: enum k=%t\n",kp,kl) ;
#endif

	    i = matpstr(params,2,kp,kl) ;

	    if (i < 0) continue ;

	    ebuf[0] = '\0' ;
	    el = 0 ;
	    if (vl > 0) {

	        el = expcook_exp(ckp,0,ebuf,EBUFLEN,vp,vl) ;
	        if (el >= 0)
	            ebuf[el] = '\0' ;

	    } /* end if */

	    if (el < 0)
	        continue ;

	    switch (i) {
	            case param_logsize:
	                if ((! pip->final.logsize) && (el > 0)) {
	                    pip->have.logsize = TRUE ;
	                    rs1 = cfdecmfi(ebuf,el,&v) ;
	                    if (rs1 >= 0)
	                        pip->logsize = v ;
	                }
	                break ;
	            case param_logfile:
	                if (! pip->final.logfname) {
	                    pip->have.logfname = TRUE ;
	                    rs1 = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        LOGDNAME,pip->searchname,"") ;
	                    ccp = pip->lfname ;
	                    if ((ccp == NULL) ||
	                        (strcmp(ccp,tmpfname) != 0)) {
	                        pip->changed.logfname = TRUE ;
	                        rs = proginfo_setentry(pip,&pip->lfname,
	                            tmpfname,rs1) ;
	                    }
	                }
	                break ;
	            case param_quotedirs:
	                if ((! lip->final.quotedirs) && (el > 0)) {
#if	CF_DEBUG
			if (DEBUGLEVEL(4))
	    		    debugprintf("config_read: caldir=%t\n",
				ebuf,el) ;
#endif
	                    rs = locinfo_loaddirs(lip,ebuf,el) ;
			}
	                break ;
	            case param_quotenames:
	                if ((! lip->final.quotenames) && (el > 0)) {
	                    rs = locinfo_loadnames(lip,ebuf,el) ;
			}
	                break ;
	            } /* end switch */

	    if (rs < 0) break ;
	} /* end while (enumerating) */

	    paramfile_curend(pfp,&cur) ;
	} /* end if (cursor) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_read) */


static int procnames(pip,app)
PROGINFO	*pip ;
PARAMOPT	*app ;
{
	LOCINFO	*lip = pip->lip ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl ;
	int		c = 0 ;
	const char	*po_name = PO_NAME ;
	char		*np ;

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {

	    while (rs >= 0) {

	        rs1 = paramopt_fetch(app,po_name,&cur,&np) ;
		nl = rs ;
	        if (rs1 == SR_NOTFOUND) break ;
		rs = rs1 ;
		if (rs < 0) break ;

	        if (nl == 0) continue ;

	        rs = locinfo_loadname(lip,np,nl) ;
	        c += rs ;

	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (cursor) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


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


static int procspecs(pip,sp,sl)
PROGINFO	*pip ;
const char	*sp ;
int		sl ;
{
	LOCINFO	*lip = pip->lip ;
	VECSTR		qstr ;
	int		rs ;
	int		c = 0 ;

	if (sp == NULL)
	    return SR_FAULT ;

	if (lip->f.interactive)
	    lip->cout = 0 ;

	if ((rs = vecstr_start(&qstr,5,0)) >= 0) {
	    FIELD	fsb ;
	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	        const int	flen = WPBUFLEN ;
		int		fl ;
	        char		fbuf[WPBUFLEN + 1] ;
	        const char	*fp = fbuf ;

	        while ((fl = field_wordphrase(&fsb,aterms,fbuf,flen)) >= 0) {
		    if (fl > 0) {
	    	        if ((rs = vecstr_adduniq(&qstr,fp,fl)) >= 0) {
	    		    c += ((rs < INT_MAX) ? 1 : 0) ;
			}
		    }
	    	    if (fsb.term == '#') break ;
	        } /* end while */

	        field_finish(&fsb) ;
	    } /* end if (field) */
	    if ((rs >= 0) && (c > 0)) {
	         rs = procspec(pip,&qstr) ;
	    }
	    vecstr_finish(&qstr) ;
	} /* end if (vecstr) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procspec(pip,qsp)
PROGINFO	*pip ;
vecstr		*qsp ;
{
	LOCINFO	*lip = pip->lip ;
	QUOTE_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cbl ;
	int		ntags ;
	int		qopts = 0 ;
	int		c = 0 ;
	int		f_cur = FALSE ;
	const char	**qkeya ;
	char		combuf[COMBUFLEN + 1] ;

	if (qsp == NULL)
	    return SR_FAULT ;

	if (lip->f.prefix)
	    qopts |= QUOTE_OPREFIX ;

	rs = quote_curbegin(&lip->qdb,&cur) ;
	f_cur = (rs >= 0) ;

	if (rs >= 0)
	    vecstr_getvec(qsp,&qkeya) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
		int	i ;
		for (i = 0 ; qkeya[i] != NULL ; i += 1)
		debugprintf("quote/procspec: sk=>%s<\n",qkeya[i]) ;
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {
	    rs = quote_lookup(&lip->qdb,&cur,qopts,qkeya) ;
	    ntags = rs ;
	}

	while ((rs >= 0) && (ntags > 0)) {
	    rs1 = quote_read(&lip->qdb,&cur,combuf,COMBUFLEN) ;
	    cbl = rs1 ;
	    if (rs1 == SR_NOTFOUND) break ;
	    rs = rs1 ;

	    if (rs >= 0) {
		c += 1 ;
		rs = procout(pip,combuf,cbl) ;
	    }

	} /* end while */

	if (f_cur)
	    quote_curend(&lip->qdb,&cur) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspec) */


static int procout(pip,buf,buflen)
PROGINFO	*pip ;
const char	buf[] ;
int		buflen ;
{
	LOCINFO	*lip = pip->lip ;
	WORDFILL	w ;
	int		rs = SR_OK ;
	int		cl ;
	int		cbl ;
	int		line ;
	int		wlen = 0 ;
	int		f_havebook = FALSE ;
	const char	*fmt ;
	char		colbuf[COLBUFLEN + 1] ;

	if (buflen <= 0)
	    goto ret0 ;

	cbl = MIN((lip->linelen - lip->indent),COLBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_quote/procout: cbl=%d\n",cbl) ;
#endif

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

/* print out citation specification */

	line = 0 ;
	rs = wordfill_start(&w,NULL,0) ;
	if (rs < 0)
	    goto ret1 ;

	if ((rs = wordfill_addlines(&w,buf,buflen)) >= 0) {

	    while ((cl = wordfill_mklinefull(&w,colbuf,cbl)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_quote/procout: m line=>%t<¬\n",
	                colbuf,strnlen(colbuf,MIN(cl,40))) ;
#endif

	        rs = procoutline(pip,line,colbuf,cl) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

		line += 1 ;

	    } /* end while (full lines) */

	    if ((rs >= 0) && ((cl = wordfill_mklinepart(&w,colbuf,cbl)) > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_quote/procout: e line=>%t<¬ cl=%u\n",
	                colbuf,strnlen(colbuf,MIN(cl,40))) ;
#endif

	        rs = procoutline(pip,line,colbuf,cl) ;
	        wlen += rs ;

		line += 1 ;

	    } /* end if (partial lines) */

	} /* end if */

ret2:
	wordfill_finish(&w) ;

/* done */
ret1:
ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procoutline(pip,line,lp,ll)
PROGINFO	*pip ;
int		line ;
const char	*lp ;
int		ll ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		indent ;
	int		wlen = 0 ;

	indent = MIN(lip->indent,NBLANKS) ;
	rs = shio_printf(lip->ofp,"%t%t\n",
	        blanks,indent,
	        lp,ll) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


