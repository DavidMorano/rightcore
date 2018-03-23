/* b_dhtext */

/* this is a SHELL built-in command to print text to a terminal */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_BUFLINEIN	1		/* line-buffering for STDIN */
#define	CF_LINERCLEAR	0		/* using |liner_clear()| */
#define	CF_LOCSETENT	0		/* using |locinfo_setentry()| */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written as a KSH built-in command.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ doubletext [-af <afile>] [-of <ofile>] <word(s)> [-V]

	Arguments:

	<word(s)>	words to print
	-af <afile>	file containing file names to read as input
	-of <ofile>	write to this file instead of to standard-output
	-V		print command version to standard-error and then exit


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
#include	<vecstr.h>
#include	<termout.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_dhtext.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifndef	ABUFLEN
#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif
#endif

#ifndef	VBUFLEN
#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	LINER		struct liner


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	termescseq(char *,int,int,int,int,int,int) ;
extern int	shio_printdline(SHIO *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

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


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		cvtcase:1 ;
	uint		cvtuc:1 ;
	uint		cvtlc:1 ;
	uint		termout:1 ;
	uint		outer:1 ;
	uint		dbl:1 ;
	uint		linelen:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	TERMOUT		outer ;
	vecstr		stores ;
	PROGINFO	*pip ;
	const char	*termtype ;
	int		to ;
	int		linelen ;
} ;

struct liner {
	LOCINFO		*lip ;
	void		*ofp ;
	char		*lbuf ;
	int		llen ;
	int		ll ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procsetcase(PROGINFO *,const char *,int) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procline(PROGINFO *,void *,cchar *,int) ;
static int	procword(PROGINFO *,void *,const char *) ;
static int	procdata(PROGINFO *,char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setlinelen(LOCINFO *) ;
static int	locinfo_termoutbegin(LOCINFO *,void *) ;
static int	locinfo_termoutend(LOCINFO *) ;
static int	locinfo_termoutprint(LOCINFO *,void *,const char *,int) ;
static int	locinfo_printd(LOCINFO *,void *,char *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,const char **,cchar *,int) ;
#endif

static int	liner_start(LINER *,LOCINFO *,void *) ;
static int	liner_finish(LINER *) ;
static int	liner_addword(LINER *,const char *,int) ;

#if	CF_LINERCLEAR
static int	liner_clear(LINER *) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"to",
	"tr",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_to,
	argopt_tr,
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
	"cvtcase",
	"cc",
	"casecvt",
	"bufwhole",
	"bufline",
	"bufnone",
	"whole",
	"line",
	"none",
	"un",
	"termout",
	"double",
	"linelen",
	"",
	NULL
} ;

enum akonames {
	akoname_cvtcase,
	akoname_cc,
	akoname_casecvt,
	akoname_bufwhole,
	akoname_bufline,
	akoname_bufnone,
	akoname_whole,
	akoname_line,
	akoname_none,
	akoname_un,
	akoname_termout,
	akoname_double,
	akoname_linelen,
	akoname_empty,
	akoname_overlast
} ;

static const char	*cases[] = {
	"upper",
	"lower",
	NULL
} ;


/* exported subroutines */


int b_dhtext(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_dhtext) */


int p_dhtext(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_dhtext) */


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
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		wlen = 0 ;
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
	const char	*tos_open = NULL ;
	const char	*tos_read = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_dhtext: starting DFD=%u\n",rs) ;
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
	pip->to_open = -1 ;
	pip->to_read = -1 ;

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

/* open time-out */
	                case argopt_to:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            tos_open = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tos_open = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* read time-out */
	                case argopt_tr:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            tos_read = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tos_read = argp ;
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

/* output file name */
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

/* terminal-type */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
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

	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                tos_open = argp ;
	                                tos_read = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* line-buffered */
	                    case 'u':
	                        pip->have.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_dhtext: debuglevel=%u\n",pip->debuglevel) ;
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
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

	if ((rs >= 0) && (tos_open != NULL)) {
	    rs = cfdecti(tos_open,-1,&v) ;
	    pip->to_open = v ;
	}

	if ((rs >= 0) && (tos_read != NULL)) {
	    rs = cfdecti(tos_read,-1,&v) ;
	    pip->to_read = v ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((pip->to_open >= 0) || (pip->to_read >= 0)) {
	        fmt = "%s: to_open=%d\n" ;
	        shio_printf(pip->efp,fmt,pn,pip->to_open) ;
	        fmt = "%s: to_read=%d\n" ;
	        shio_printf(pip->efp,fmt,pn,pip->to_read) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_dhtext: to_open=%d\n",pip->to_open) ;
	    debugprintf("b_dhtext: to_read=%d\n",pip->to_read) ;
	    debugprintf("b_dhtext: f_bufline=%u\n",pip->f.bufline) ;
	    debugprintf("b_dhtext: f_bufnone=%u\n",pip->f.bufnone) ;
	}
#endif /* CF_DEBUG */

	if (pip->to_open == 0)
	    pip->to_open = 1 ;

	if (pip->to_read == 0)
	    pip->to_read = 1 ;

	if (rs >= 0) {
	    rs = locinfo_setlinelen(lip) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    rs = process(pip,aip,bop,ofn,afn) ;
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
	        shio_printf(pip->efp,"%s: could not process file (%d)\n",
	            pip->progname,rs) ;
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
	    debugprintf("b_dhtext: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_dhtext: final mallout=%u\n",mo-mo_start) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",pip->progname,rs) ;
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

	fmt = "%s: USAGE> %s [<word(s)> ...] [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
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
	                case akoname_cvtcase:
	                case akoname_casecvt:
	                case akoname_cc:
	                    if (! lip->final.cvtcase) {
	                        lip->have.cvtcase = TRUE ;
	                        lip->final.cvtcase = TRUE ;
	                        lip->f.cvtcase = TRUE ;
	                        if (vl > 0)
	                            rs = procsetcase(pip,vp,vl) ;
	                    }
	                    break ;
	                case akoname_bufwhole:
	                case akoname_whole:
	                    if (! pip->final.bufwhole) {
	                        pip->have.bufwhole = TRUE ;
	                        pip->final.bufwhole = TRUE ;
	                        pip->f.bufwhole = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufwhole = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufline:
	                case akoname_line:
	                    if (! pip->final.bufline) {
	                        pip->have.bufline = TRUE ;
	                        pip->final.bufline = TRUE ;
	                        pip->f.bufline = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufline = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufnone:
	                case akoname_none:
	                case akoname_un:
	                    if (! pip->final.bufnone) {
	                        pip->have.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufnone = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_termout:
	                    if (! lip->final.termout) {
	                        lip->have.termout = TRUE ;
	                        lip->final.termout = TRUE ;
	                        lip->f.termout = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.termout = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_double:
	                    if (! lip->final.dbl) {
	                        lip->have.dbl = TRUE ;
	                        lip->final.dbl = TRUE ;
	                        lip->f.dbl = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.dbl = (rs > 0) ;
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
	                case akoname_empty:
	                    break ;
	                default:
	                    rs = SR_INVALID ;
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


static int procsetcase(PROGINFO *pip,cchar *vp,int vl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		ci ;

	if ((ci = matostr(cases,1,vp,vl)) >= 0) {
	    const int	ch = CHAR_TOLC(vp[0]) ;
	    switch (ch) {
	    case 'l':
	        lip->f.cvtlc = TRUE ;
	        break ;
	    case 'u':
	        lip->f.cvtuc = TRUE ;
	        break ;
	    default:
	        rs = ci ; /* lint */
	        break ;
	    } /* end switch */
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (procsetcase) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	const int	to = pip->to_open ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_opene(ofp,ofn,"wct",0666,to)) >= 0) {
	    LOCINFO	*lip = pip->lip ;

	    if (pip->have.bufnone)
	        shio_control(ofp,SHIO_CSETBUFNONE,TRUE) ;

	    if (pip->have.bufline)
	        shio_control(ofp,SHIO_CSETBUFLINE,pip->f.bufline) ;

	    if (pip->have.bufwhole)
	        shio_control(ofp,SHIO_CSETBUFWHOLE,pip->f.bufwhole) ;

/* go through the loops */

	    if ((rs = locinfo_termoutbegin(lip,ofp)) >= 0) {
	        LINER	l ;
	        cchar	*cp ;

	        if ((rs = liner_start(&l,lip,ofp)) >= 0) {
	            int		ai ;
	            int		f ;
		    cchar	**argv = aip->argv ;

	            for (ai = 1 ; ai < aip->argc ; ai += 1) {

	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = aip->argv[ai] ;
	                    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = liner_addword(&l,cp,-1) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end for */

	            rs1 = liner_finish(&l) ;
	            if (rs >= 0) rs = rs1 ;
	            wlen += rs1 ;
	        } /* end if (liner) */

	        if ((rs >= 0) && (afn != NULL)) {
	            SHIO	afile, *afp = &afile ;

	            if ((afn[0] == '\0') || (strcmp(afn,"-") == 0))
	                afn = STDINFNAME ;

	            if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if (len > 0) {
	                        pan += 1 ;
	                        rs = procline(pip,ofp,lbuf,len) ;
	                        wlen += rs ;
	                    }

	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                fmt = "%s: afile=%s\n" ;
	                shio_printf(pip->efp,fmt,pn,afn) ;
	            } /* end if */

	        } /* end if (procesing file argument file list) */

	        rs1 = locinfo_termoutend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (termout) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

	if ((pip->debuglevel > 0) && (rs >= 0)) {
	    fmt = "%s: written=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,wlen) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procline(PROGINFO *pip,void *ofp,cchar *lp,int ll)
{
	LOCINFO		*lip = pip->lip ;
	LINER		l ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = liner_start(&l,lip,ofp)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

	    while ((cl = nextfield(lp,ll,&cp)) > 0) {

	        rs = liner_addword(&l,cp,cl) ;
	        wlen += rs ;

	        ll -= ((cp+1)-lp) ;
	        lp = (cp+1) ;

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = liner_finish(&l) ;
	    if (rs >= 0) rs = rs1 ;
	    wlen += rs1 ;
	} /* end if (liner) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


/* process a file */
static int procword(PROGINFO *pip,void *ofp,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		infile, *ifp = &infile ;
	const int	to_open = pip->to_open ;
	const int	to_read = pip->to_read ;
	const int	llen = LINEBUFLEN ;
	volatile int	*intarr[3] ;
	int		rs = SR_OK ;
	int		len ;
	int		wlen = 0 ;
	int		f_fifo = FALSE ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_dhtext/procword: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	{
	    int	i = 0 ;
	    intarr[i] = NULL ;
	}

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0)) {
	    fname = STDINFNAME ;
	}

	if (rs >= 0) {
	    int	i = 0 ;
	    lbuf[i++] = 'r' ;
	    if (f_fifo) lbuf[i++] = 'n' ;
	    lbuf[i] = '\0' ;
	}

	if (rs >= 0)
	    rs = shio_opene(ifp,fname,lbuf,0666,to_open) ;

	if (rs < 0) goto ret0 ;

	if ((rs >= 0) && f_fifo)
	    rs = shio_control(ifp,SHIO_CNONBLOCK,0) ;

	if ((rs >= 0) && pip->f.bufnone)
	    rs = shio_control(ifp,SHIO_CSETBUFNONE,0) ;

#if	CF_BUFLINEIN
	if (pip->f.bufline)
	    rs = shio_control(ifp,SHIO_CSETBUFLINE,TRUE) ;
#endif

	if (lip->open.outer) {

	    while (rs >= 0) {
	        rs = shio_readlines(ifp,lbuf,llen,NULL) ;
	        len = rs ;
	        if (rs <= 0) break ;

	        rs = locinfo_termoutprint(lip,ofp,lbuf,len) ;
	        wlen += rs ;

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	    } /* end while */

	} else {

	    while (rs >= 0) {

	        if (pip->f.bufline || pip->f.bufnone) {
	            rs = shio_readlinetimed(ifp,lbuf,llen,to_read) ;
	            len = rs ;
	        } else {
	            rs = shio_readintr(ifp,lbuf,llen,to_read,intarr) ;
	            len = rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_dhtext/procword: "
	                "shio_readintr() rs=%d\n",
	                rs) ;
	            if (rs >= 0)
	                debugprintf("b_dhtext/procword: d=>%t<\n",
	                    lbuf,strlinelen(lbuf,len,40)) ;
	        }
#endif

	        if ((rs >= 0) && (len == 0)) break ; /* EOF */

	        if (rs == SR_AGAIN) {
	            rs = SR_OK ;
	            len = 0 ;
	        }

	        if ((rs >= 0) && (len > 0) && lip->f.cvtcase) {
	            rs = procdata(pip,lbuf,len) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_dhtext/procword: c=>%t<\n",
	                    lbuf,strlinelen(lbuf,len,40)) ;
#endif

	        } /* end if (in-place data conversion) */

	        if ((rs >= 0) && (len > 0)) {

	            rs = shio_write(ofp,lbuf,len) ;
	            wlen += rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_dhtext/procword: "
	                    "shio_write() rs=%d\n",
	                    rs) ;
#endif

	        } /* end if (write) */

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	    } /* end while (reading lines) */

	} /* end if (outer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_dhtext/procword: after rs=%d\n",rs) ;
#endif

	shio_close(ifp) ;

ret0:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: file=%s (%d)\n",
	        pip->progname,fname,((rs >= 0) ? wlen : rs)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_dhtext/procword: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procword) */


static int procdata(PROGINFO *pip,char *bufline,int len)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((len > 0) && lip->f.cvtcase) {
	    int		i ;
	    int		ch, nch ;

	    if (lip->f.cvtlc) {
	        for (i = 0 ; i < len ; i += 1) {
	            ch = (bufline[i] & 0xff) ;
	            nch = CHAR_TOLC(ch) ;
	            if (ch != nch)
	                bufline[i] = nch ;
	        }
	    } else if (lip->f.cvtuc) {
	        for (i = 0 ; i < len ; i += 1) {
	            ch = (bufline[i] & 0xff) ;
	            nch = CHAR_TOUC(ch) ;
	            if (ch != nch)
	                bufline[i] = nch ;
	        } /* end for */
	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (procdata) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;
	const char	*varterm = VARTERM ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->termtype = getourenv(pip->envv,varterm) ;

	lip->f.dbl = TRUE ;

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


#if	CF_LOCSETENT
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
#endif /* CF_LOCSETENT */


static int locinfo_setlinelen(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->linelen == 0)) {
	    cchar	*vp = getourenv(pip->envv,VARCOLUMNS) ;
	    if (vp != NULL) {
	        rs = optvalue(vp,-1) ;
	        lip->linelen = rs ;
	    }
	} /* end if */

	if ((rs >= 0) && (lip->linelen == 0)) {
	    lip->linelen = COLUMNS ;
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_setlinelen) */


static int locinfo_termoutbegin(LOCINFO *lip,void *ofp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*tstr = lip->termtype ;

	if (lip->f.termout || ((rs = shio_isterm(ofp)) > 0)) {
	    int		ncols = COLUMNS ;
	    cchar	*vp ;
	    if ((vp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	        int	v ;
	        rs1 = cfdeci(vp,-1,&v) ;
	        if (rs1 >= 0) ncols = v ;
	    }
	    if (rs >= 0) {
	        rs = termout_start(&lip->outer,tstr,-1,ncols) ;
	        lip->open.outer = (rs >= 0) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_termoutbegin) */


static int locinfo_termoutend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.outer) {
	    lip->open.outer = FALSE ;
	    rs1 = termout_finish(&lip->outer) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_termoutend) */


static int locinfo_termoutprint(LOCINFO *lip,void *ofp,cchar lbuf[],int llen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (llen > 0) {
	    TERMOUT	*top = &lip->outer ;
	    if ((rs = termout_load(top,lbuf,llen)) >= 0) {
	        int	ln = rs ;
	        int	i ;
	        int	ll ;
	        cchar	*lp ;
	        for (i = 0 ; i < ln ; i += 1) {
	            ll = termout_getline(top,i,&lp) ;
	            if (ll < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                const int	maxcols = COLUMNS ;
	                debugprintf("b_dhtext/locinfo_termoutprint: "
	                    "maxcols=%u ll=%u\n",
	                    maxcols,ll) ;
	                debugprintf("b_dhtext/locinfo_termoutprint: "
	                    "l=>%t<\n",
	                    lp,strlinelen(lp,ll,40)) ;
	            }
#endif /* CF_DEBUG */

	            if (lip->f.dbl) {
	                rs = shio_printdline(ofp,lp,ll) ;
	                wlen += rs ;
	            } else {
	                rs = shio_print(ofp,lp,ll) ;
	                wlen += rs ;
	            }
	            if (rs < 0) break ;

	        } /* end for */
	        if ((rs >= 0) && (ll != SR_NOTFOUND)) rs = ll ;
	    } /* end if (termout_load) */
	} else {
	    rs = shio_print(ofp,lbuf,llen) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_termoutprint) */


static int locinfo_printd(LOCINFO *lip,void *ofp,char lbuf[],int llen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_printd: open_outer=%u\n",lip->open.outer) ;
#endif

	if (lip->open.outer) {
	    rs = locinfo_termoutprint(lip,ofp,lbuf,llen) ;
	    wlen += rs ;
	} else {
	    rs = shio_print(ofp,lbuf,llen) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_printd: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_printd) */


static int liner_start(LINER *lrp,LOCINFO *lip,void *ofp)
{
	int		rs = SR_OK ;
	int		llen ;
	int		size ;
	char		*p ;

	memset(lrp,0,sizeof(LINER)) ;
	lrp->lip = lip ;
	lrp->ofp = ofp ;

	llen = (2 * lip->linelen) ;
	size = (llen+20) ;		/* extra space for escape sequence */
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    lrp->lbuf = p ;
	    lrp->llen = llen ;
	    lrp->lbuf[0] = '\0' ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (liner_start) */


static int liner_finish(LINER *lrp)
{
	LOCINFO		*lip = lrp->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	void		*ofp = lrp->ofp ;

	if (lrp->ll > 0) {
	    rs = locinfo_printd(lip,ofp,lrp->lbuf,lrp->ll) ;
	    lrp->ll = 0 ;
	    lrp->lbuf[0] = '\0' ;
	}

	if (lrp->lbuf != NULL) {
	    rs1 = uc_free(lrp->lbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    lrp->lbuf = NULL ;
	}

	return rs ;
}
/* end subroutine (liner_finish) */


#if	CF_LINERCLEAR
static int liner_clear(LINER *lrp)
{
	int		rs = SR_OK ;

	if (lrp->lbuf != NULL) {
	    lrp->ll = 0 ;
	    lrp->lbuf[0] = '\0' ;
	}

	return rs ;
}
/* end subroutine (liner_clear) */
#endif /* CF_LINERCLEAR */


static int liner_addword(LINER *lrp,cchar *wp,int wl)
{
	LOCINFO		*lip = lrp->lip ;
	void		*ofp = lrp->ofp ;
	int		rs = SR_OK ;
	int		linelen ;
	int		dwl ;
	int		f = FALSE ;

	linelen = lip->linelen ;
	if (wl < 0) wl = strlen(wp) ;

	dwl = (2*(1+wl)) ;

	f = f || (dwl >= linelen) ;
	f = f || ((lrp->ll + dwl + 1) > linelen) ;
	if (f && (wl > 0)) {
	    rs = locinfo_printd(lip,ofp,lrp->lbuf,lrp->ll) ;
	    lrp->ll = 0 ;
	    lrp->lbuf[0] = '\0' ;
	}

	if ((rs >= 0) && (dwl >= linelen)) {
	    int		size = (wl+20) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        strwcpy(bp,wp,wl) ;
	        rs = locinfo_printd(lip,ofp,bp,wl) ;
	        wl = 0 ;
	        uc_free(bp) ;
	    } /* end if (memory-allocation) */
	}

	if ((rs >= 0) && (wl > 0)) {
	    int		i = 0 ;
	    char	*bp = (lrp->lbuf + lrp->ll) ;
	    if (lrp->ll > 0) {
	        *bp++ = ' ' ;
	        i += 1 ;
	    }
	    bp = strwcpy(bp,wp,wl) ;
	    lrp->ll += (wl + i) ;
	}

	return rs ;
}
/* end subroutine (liner_addword) */


