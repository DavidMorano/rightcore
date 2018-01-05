/* b_commandment */

/* translate a commandment number to its corresponding entry (string) */
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

	$ commandment <num(s)>


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
#include	<tmtime.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_commandment.h"
#include	"defs.h"
#include	"commandment.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	(4*LINEBUFLEN)	/* maximum commandment length (?) */
#endif

#define	SPECLEN		100

#define	COLBUFLEN	(COLUMNS + 10)

#define	NBLANKS		20

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sicasesub(const char *,int,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	ndigits(int,int) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
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
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		all:1 ;
	uint		n:1 ;
	uint		audit:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		separate:1 ;
	uint		interactive:1 ;
	uint		defall:1 ;
	uint		defnull:1 ;
	uint		rotate:1 ;
	uint		tmtime:1 ;
	uint		nitems:1 ;
	uint		gmt:1 ;
} ;

struct locinfo {
	COMMANDMENT	cdb ;
	void		*ofp ;
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	TMTIME		tm ;
	int		linelen ;
	int		indent ;
	int		nitems ;
	int		count, max, precision ;
	int		cout ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,int) ;
static int	procsome(PROGINFO *,ARGINFO *,BITS *,cchar *,int) ;
static int	procspecs(PROGINFO *,const char *,int) ;
static int	procspec(PROGINFO *,const char *,int) ;
static int	procall(PROGINFO *) ;
static int	procstrings(PROGINFO *,const char *,int) ;
static int	procout(PROGINFO *,uint,const char *,int) ;
static int	procoutline(PROGINFO *,int *,uint,const char *,int) ;

static int	loadprecision(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_deflinelen(LOCINFO *) ;
static int	locinfo_tmtime(LOCINFO *) ;
static int	locinfo_combegin(LOCINFO *,cchar *) ;
static int	locinfo_comend(LOCINFO *) ;

static int	vecstr_have(vecstr *,cchar *,int) ;

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
	"separate",
	"interactive",
	"defnull",
	"default",
	"defall",
	"rotate",
	"gmt",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_linelen,
	akoname_indent,
	akoname_separate,
	akoname_interactive,
	akoname_defnull,
	akoname_default,
	akoname_defall,
	akoname_rotate,
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


/* exported subroutines */


int b_commandment(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_commandment) */


int p_commandment(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_commandment) */


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
	int		f_apm = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_commandment: starting DFD=%d\n",rs) ;
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
	if (rs < 0)
	    goto badlocstart ;

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

	            if (f_optplus) f_apm = TRUE ;
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

/* BibleBook DB file */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbname = argp ;
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

	                    case 'a':
	                        lip->f.all = TRUE ;
	                        break ;

	                    case 'n':
	                        lip->f.n = TRUE ;
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

/* use GMT */
	                    case 'z':
	                        lip->final.gmt = TRUE ;
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
	    debugprintf("b_commandment: debuglevel=%u\n",pip->debuglevel) ;
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

	if ((lip->nitems <= 0) && (argval != NULL)) {
	    if ((rs = optvalue(argval,-1)) >= 0) {
	        lip->nitems = (f_apm) ? (rs+1) : rs ;
	    }
	} else
	    lip->nitems = 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_commandment: nitems=%u\n",lip->nitems) ;
#endif

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (dbname == NULL) dbname = getourenv(envv,VARDB) ;
	if (dbname == NULL) dbname = DBNAME ;

	if (rs >= 0) {
	    rs = locinfo_deflinelen(lip) ;
	}

/* process */

	if (pip->debuglevel > 0) {
	    cp = (dbname != NULL) ? dbname : "" ;
	    shio_printf(pip->efp,"%s: db=%s\n",pip->progname,cp) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	if ((rs = locinfo_combegin(lip,dbname)) >= 0) {

	    if (lip->f.audit) {
	        rs = commandment_audit(&lip->cdb) ;
	        if ((rs < 0) || (pip->debuglevel > 0))
	            shio_printf(pip->efp,"%s: DB audit (%d)\n",
	                pip->progname,rs) ;
	    }

	    if ((rs = loadprecision(pip)) >= 0) {
	        rs = process(pip,&ainfo,&pargs,ofname,afname,f_apm) ;
	    }

	    rs1 = locinfo_comend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    shio_printf(pip->efp,
	        "%s: could not load commandment-DB (%d)\n",
	        pip->progname,rs) ;
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
	    debugprintf("b_commandment: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_commandment: final mallout=%u\n",mo-mo_start) ;
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
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<number(s)>|<string(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-a] [-db <dbname>] [-q] [-w <width>] [-o <opt(s)>]\n" ;
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
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->indent = rs ;
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
	                case akoname_defall:
	                    if (! lip->final.defall) {
	                        lip->have.defall = TRUE ;
	                        lip->final.defall = TRUE ;
	                        lip->f.defall = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.defall = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_defnull:
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
	                case akoname_rotate:
	                    if (! lip->final.rotate) {
	                        lip->have.rotate = TRUE ;
	                        lip->final.rotate = TRUE ;
	                        lip->f.rotate = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.rotate = (rs > 0) ;
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


static int process(pip,aip,bop,ofn,afn,f_apm)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
const char	*ofn;
const char	*afn;
int		f_apm ;
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
	        rs = procsome(pip,aip,bop,afn,f_apm) ;
		wlen += rs ;
	    }

	    lip->ofp = NULL ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procsome(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,int f_apm)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	int		wlen = 0 ;
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

	if (rs >= 0) {
	    if (((pan == 0) && lip->f.defnull) || f_apm) {
		pan += 1 ;
	        rs = procspec(pip,"+",1) ;
	        wlen += rs ;
	    }
	} /* end if */

	if (rs >= 0) {
	    if ((pan == 0) && lip->f.defall) {
	        pan += 1 ;
	        rs = procall(pip) ;
	        wlen += rs ;
	    }
	} /* end if */

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

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    const char	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,cchar sp[],int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		ch ;
	int		wlen = 0 ;
	cchar		*fmt ;

	if (sp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_commandment/procspec: s=>%t<\n",
	        sp,strlinelen(sp,sl,40)) ;
	    debugprintf("b_commandment/procspec: nitems=%d\n",
	        lip->nitems) ;
	}
#endif

	ch = (sp[0] & 0xff) ;
	if (isdigitlatin(ch) || (ch == '+')) {
	    const uint	max = lip->max ;
	    uint	i ;
	    int		cbl ;

	    if (ch == '+') {
	        if ((rs = locinfo_tmtime(lip)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_commandment/procspec: max=%d day=%d\n",
	                    lip->max,lip->tm.mday) ;
#endif
	            i = (lip->tm.mday % max) ;
	            if (i == 0) i = max ;
	        }
	    } else {
	        rs = cfdecui(sp,sl,&i) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_commandment/procspec: _get() i=%d\n",i) ;
#endif

	    if (rs >= 0) {
	        const int	ni = lip->nitems ;
	        const int	clen = COMBUFLEN ;
	        int		j ;
	        char		cbuf[COMBUFLEN + 1] ;
	        for (j = 0 ; (rs >= 0) && (j < ni) ; j += 1) {
	            cbl = commandment_get(&lip->cdb,i,cbuf,clen) ;
	            if (cbl == SR_NOTFOUND) break ;
	            rs = cbl ;
	            if (rs > 0) {
	                rs = procout(pip,i,cbuf,cbl) ;
			wlen += rs ;
	            }
	            if (lip->f.rotate) {
	                i = ((i+1) % max) ;
	                if (i == 0) i = max ;
	            } else {
	                i += 1 ;
	            }
	        } /* end for */
	    } /* end if */

	} else {

	    rs = procstrings(pip,sp,sl) ;
	    wlen += rs ;

	} /* end if (type of query) */

	    if ((rs < 0) && isNotGoodCite(rs) && lip->f.interactive) {
		fmt = "invalid citation=>%t< (%d)\n" ;
	        rs = shio_printf(lip->ofp,fmt,sp,sl,rs) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_commandment/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procstrings(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		ps ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_commandment/procstrings: sl=%u\n",sl) ;
	    debugprintf("b_commandment/procstrings: s=>%t<\n",sp,sl) ;
	}
#endif

	if ((rs = vecstr_start(&ps,10,0)) >= 0) {
	    if ((rs = vecstr_adds(&ps,sp,sl)) > 0) {
		COMMANDMENT	*cmdp = &lip->cdb ;
		COMMANDMENT_CUR	cur ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_commandment/procstrings: cur\n") ;
#endif

		if ((rs = commandment_curbegin(cmdp,&cur)) >= 0) {
	            uint	cn ;
	            const int	clen = COMBUFLEN ;
		    const int	n = (lip->max > 0) ? (lip->max+1) : NDBENTS ;
		    int		c = 0 ;
	            int		cbl ;
	            char	cbuf[COMBUFLEN + 1] ;
		    while (rs >= 0) {

	                cbl = commandment_enum(cmdp,&cur,&cn,cbuf,clen) ;
	                if (cbl == SR_NOTFOUND) break ;
	                rs = cbl ;
	                if (rs < 0) break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_commandment/procstrings: "
			        "c=>%t<\n",cbuf,strlinelen(cbuf,cbl,40)) ;
#endif

		        if ((rs = vecstr_have(&ps,cbuf,cbl)) > 0) {
	                    rs = procout(pip,cn,cbuf,cbl) ;
			    wlen += rs ;
	                } /* end if (match) */

		        if (++c >= n) break ;
	                if (rs < 0) break ;
	            } /* end while */
		    rs1 = commandment_curend(cmdp,&cur) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (commandment-cur) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_commandment/procstrings: "
			"cur-out rs=%d\n",rs) ;
#endif

	    } /* end if (non-zero items to process) */
	    rs1 = vecstr_finish(&ps) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procstrings) */


static int procall(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	COMMANDMENT	*cmdp = &lip->cdb ;
	COMMANDMENT_CUR	cur ;
	const int	clen = COMBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cbl ;
	int		wlen = 0 ;
	char		cbuf[COMBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_commandment/procall: ent\n") ;
	    debugprintf("b_commandment/procall: max=%d\n",lip->max) ;
	}
#endif

	if ((rs = commandment_curbegin(cmdp,&cur)) >= 0) {
	    uint	cn ;
	    const int	n = (lip->max > 0) ? (lip->max+1) : NDBENTS ;
	    int		c = 0 ;
	    while (rs >= 0) {

	        cbl = commandment_enum(&lip->cdb,&cur,&cn,cbuf,clen) ;
	        if (cbl == SR_NOTFOUND) break ;
	        rs = cbl ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_commandment/procall: cn=%u\n",cn) ;
#endif

	        if ((rs >= 0) && (cn != 0)) {
	            rs = procout(pip,cn,cbuf,cbl) ;
		    wlen += rs ;
	        }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (++c >= n) break ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = commandment_curend(cmdp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (commandment-cur) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_commandment/procall: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end if (procall) */


static int procout(PROGINFO *pip,uint n,cchar *vp,int vl)
{
	LOCINFO		*lip = pip->lip ;
	const int	clen = COLBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cbl ;
	int		line = 0 ;
	int		wlen = 0 ;
	const char	*fmt ;

	if (vl <= 0)
	    goto ret0 ;

	cbl = MIN(lip->linelen,clen) - (lip->precision + 1) ;

	if (cbl < (lip->precision + 2)) {
	    rs = SR_OVERFLOW ;
	    goto ret0 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_commandment/procout: n=%u vl=%d\n",n,vl) ;
#endif

#if	CF_COOKIE
	fmt = "%%\n" ;
#else
	fmt = "\n" ;
#endif

#ifdef	COMMENT
	rs = shio_printf(lip->ofp,"%2u >%t<\n",
	    n,vp,MIN(vl,20)) ;
#endif

/* print out any necessary separator */

	if ((lip->cout++ > 0) && lip->f.separate) {
	    rs = shio_printf(lip->ofp,fmt) ;
	    wlen += rs ;
	} /* end if (separator) */

/* print out the text-data itself */

	if (rs >= 0) {
	    WORDFILL	w ;
	    if ((rs = wordfill_start(&w,vp,vl)) >= 0) {
	        int		cl ;
	        char		cbuf[COLBUFLEN + 1] ;

	        while ((cl = wordfill_mklinefull(&w,cbuf,cbl)) > 0) {
	            rs = procoutline(pip,&line,n,cbuf,cl) ;
	            wlen += rs ;
	            if (rs < 0) break ;
	        } /* end while (full lines) */

	        if (rs >= 0) {
	            if ((cl = wordfill_mklinepart(&w,cbuf,cbl)) > 0) {
	                rs = procoutline(pip,&line,n,cbuf,cl) ;
	                wlen += rs ;
	            }
	        } /* end if (partial lines) */

	        rs1 = wordfill_finish(&w) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (wordfill) */
	} /* end if (ok) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procoutline(PROGINFO *pip,int *linep,uint n,cchar *lp,int ll)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		prec ;
	int		wlen = 0 ;

	prec = MIN(lip->precision,NBLANKS) ;

	if (*linep == 0) {
	    rs = shio_printf(lip->ofp,"%*u %t\n",prec,n,lp,ll) ;
	    wlen += rs ;
	} else {
	    rs = shio_printf(lip->ofp,"%t %t\n",blanks,prec,lp,ll) ;
	    wlen += rs ;
	}

	*linep = (*linep + 1) ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


static int loadprecision(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		prec = DEFPRECISION ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_commandment/loadprecision: ent\n") ;
#endif

	if (lip->max < 0) {
	    const int	nrs = SR_NOSYS ;
	    lip->max = 0 ;
	    if ((rs = commandment_max(&lip->cdb)) == nrs) {
	        if ((rs = commandment_count(&lip->cdb)) >= 0) {
	            lip->count = rs ;
	            if (rs > 0) lip->max = (rs-1) ;
		} else if (rs == nrs) {
		    rs = SR_OK ;
		}
	    } else {
	        lip->max = rs ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_commandment/loadprecision: max=%d\n",lip->max) ;
#endif

	if (rs >= 0) {
	    prec = ndigits(lip->max,10) ;
	    lip->precision = prec ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_commandment/loadprecision: ret rs=%d prec=%u\n",
	        rs,prec) ;
#endif

	return (rs >= 0) ? prec : rs ;
}
/* end subroutine (loadprecision) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->linelen = 0 ;
	lip->count = -1 ;
	lip->max = -1 ;
	lip->f.separate = DEFOPT_SEPARATE ;
	lip->f.rotate = DEFOPT_ROTATE ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{

	lip->pip = NULL ;
	return SR_OK ;
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
	    if (! isStrEmpty(cp,-1)) {
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


static int locinfo_combegin(LOCINFO *lip,cchar *dbname)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	rs = commandment_open(&lip->cdb,pip->pr,dbname) ;
	return rs ;
}
/* end subroutine (locinfo_combegin) */


static int locinfo_comend(LOCINFO *lip)
{
	int		rs ;
	rs = commandment_close(&lip->cdb) ;
	return rs ;
}
/* end subroutine (locinfo_comend) */


static int locinfo_tmtime(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.tmtime) {
	    lip->f.tmtime = TRUE ;
	    if (pip->daytime == 0) pip->daytime = time(NULL) ;
	    if (lip->f.gmt) {
	        rs = tmtime_gmtime(&lip->tm,pip->daytime) ;
	    } else {
	        rs = tmtime_localtime(&lip->tm,pip->daytime) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_tmtime) */


static int vecstr_have(vecstr *vlp,cchar *cbuf,int clen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		j ;
	int		f = TRUE ;
	cchar		*ep ;
	for (j = 0 ; (rs1 = vecstr_get(vlp,j,&ep)) >= 0 ; j += 1) {
	                if (ep != NULL) {
	                    if (sicasesub(cbuf,clen,ep) < 0) {
	                        f = FALSE ;
	                        break ;
	                    }
			}
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (vecstr_have) */


static int isNotGoodCite(int rs)
{
	int		f = FALSE ;
	f = f || (rs == SR_NOTFOUND) ;
	f = f || isNotValid(rs) ;
	return f ;
}
/* end subroutine (isNotGoodCite) */


