/* b_sanity */

/* this is a SHELL built-in version of 'cat(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LINEBUFIN	1		/* line-buffering for STDIN */
#define	CF_LOCSETENT	0		/* allow |locinfo_setentry()| */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine is originally written as a KSH built-in command.  The
	idea for this program comes from a previous one that I wrote that did
	the same thing, but in a stand-alone program.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ sanity [<file(s)> ...] [<options>]


*******************************************************************************/


#include	<envstandards.h>

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
#include	<linefold.h>
#include	<char.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_sanity.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#define	NBLANKS		8

#ifndef	TO_OPEN
#define	TO_OPEN		4
#endif
#ifndef	TO_READ
#define	TO_READ		4
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isEOL(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

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
extern char	*strnchr(cchar *,int,int) ;


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
	uint		geekout:1 ;
	uint		pass:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	int		to ;
	int		linelen ;
	int		indent ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procfile(PROGINFO *,void *,cchar *) ;
static int	mkgeekout(PROGINFO *,char *,int,const char *,int) ;
static int	procoutline(PROGINFO *,void *,const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	isOther(int) ;


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
	"indent",
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
	argopt_indent,
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
	"toopen",
	"toread",
	"width",
	"sanity",
	"geekout",
	"pass",
	NULL
} ;

enum progopts {
	progopt_toopen,
	progopt_toread,
	progopt_width,
	progopt_sanity,
	progopt_geekout,
	progopt_pass,
	progopt_overlast
} ;

static const char	blanks[NBLANKS+1] = "        " ;


/* exported subroutines */


int b_sanity(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_sanity) */


int p_sanity(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_sanity) */


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
	const char	*ifname = NULL ;
	const char	*tos_open = NULL ;
	const char	*tos_read = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_sanity: starting DFD=%d\n",rs) ;
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

	pip->lip = lip ;
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

/* line-indent */
	                case argopt_indent:
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

/* input file name */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
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

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tos_read = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* line-buffered */
	                    case 'u':
	                        pip->f.bufline = TRUE ;
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

	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
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
	    debugprintf("b_sanity: debuglevel=%u\n",pip->debuglevel) ;
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

/* some initialization */

	pip->to_open = -1 ;
	pip->to_read = -1 ;
	if ((rs >= 0) && (tos_open != NULL)) {
	    rs = cfdecti(tos_open,-1,&v) ;
	    pip->to_open = v ;
	}

	if ((rs >= 0) && (tos_read != NULL)) {
	    rs = cfdecti(tos_read,-1,&v) ;
	    pip->to_read = v ;
	}

/* program options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* check arguments */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    if ((pip->to_open >= 0) || (pip->to_read >= 0)) {
	    shio_printf(pip->efp,"%s: to_open=%d\n",pn,pip->to_open) ;
	    shio_printf(pip->efp,"%s: to_read=%d\n",pn,pip->to_read) ;
	    }
	    shio_printf(pip->efp,"%s: pass=%u\n",pn,lip->f.pass) ;
	}

	if (pip->to_open == 0)
	    pip->to_open = 1 ;

	if (pip->to_read == 0)
	    pip->to_read = 1 ;

	if (lip->f.pass && (! lip->have.geekout)) lip->f.geekout = FALSE ;

/* linewidth (for geek-out mode -- the default) */

	if ((rs >= 0) && (lip->linelen <= 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    lip->linelen = rs ;
	}

	if ((rs >= 0) && (lip->linelen <= 0)) {
	    if ((cp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        lip->linelen = rs ;
	    }
	}

	if ((rs >= 0) && (lip->linelen <= 0)) {
	    lip->linelen = COLUMNS ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_sanity: linelen=%u\n",lip->linelen) ;
	    debugprintf("b_sanity: to_open=%d\n",pip->to_open) ;
	    debugprintf("b_sanity: to_read=%d\n",pip->to_read) ;
	    debugprintf("b_sanity: f_lbufline=%u\n",pip->f.bufline) ;
	    debugprintf("b_sanity: geekout=%u\n",lip->f.geekout) ;
	    debugprintf("b_sanity: pass=%u\n",lip->f.pass) ;
	}
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: geekout=%u\n",pn,lip->f.geekout) ;
	    shio_printf(pip->efp,"%s: linelen=%d\n",pn,lip->linelen) ;
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*ofn = ofname ;
	    const char	*ifn = ifname ;
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,ifn,afn) ;
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
	    shio_printf(pip->efp,"%s: could not process file (%d)\n",
	        pip->progname,rs) ;
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
	if (DEBUGLEVEL(4))
	    debugprintf("b_sanity: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_sanity: final mallout=%u\n",(mo-mo_start)) ;
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
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->indent = DEFINDENT ;
	lip->to = -1 ;

	lip->f.geekout = TRUE ;

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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...] [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-to <to_open>] [-tr <to_read>] [-<width>] [-w <width>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
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
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	v ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_toopen:
	                    if (vl > 0) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_open = v ;
	                    }
	                    break ;
	                case progopt_toread:
	                    if (vl > 0) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_read = v ;
	                    }
	                    break ;
	                case progopt_width:
	                    if (vl > 0) {
	                        rs = optvalue(vp,vl) ;
	                        lip->linelen = rs ;
	                    }
	                    break ;
	                case progopt_sanity:
	                case progopt_geekout:
	                    lip->have.geekout = TRUE ;
	                    lip->f.geekout = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.geekout = (rs > 0) ;
	                    }
	                    break ;
	                case progopt_pass:
	                    lip->have.pass = TRUE ;
	                    lip->f.pass = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.pass = (rs > 0) ;
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


static int procargs(pip,aip,bop,ofn,ifn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
const char	*ofn ;
const char	*ifn ;
const char	*afn ;
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_opene(ofp,ofn,"wct",0666,pip->to_open)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (pip->f.bufline)
	        shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;

/* go through the loops */

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
	                    rs = procfile(pip,ofp,cp) ;
	                    wlen += rs ;
			}
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len-1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
				lbuf[((cp+cl)-lbuf)] = '\0' ;
	                        pan += 1 ;
	                        rs = procfile(pip,ofp,cp) ;
	                        wlen += rs ;
	                    }
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
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (procesing file argument file list) */

	    if (rs >= 0) {
	        if (((ifn != NULL) && (ifn[0] != '\0')) || (pan == 0)) {

	            if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	                ifn = STDINFNAME ;

	            pan += 1 ;
	            rs = procfile(pip,ofp,ifn) ;
	            wlen += rs ;

	        } /* end if (whatever) */
	    } /* end if (ok) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


/* process a file */
static int procfile(PROGINFO *pip,void *ofp,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	const int	to_open = pip->to_open ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len ;
	int		cl ;
	int		wlen = 0 ;
	int		f_stdin = FALSE ;
	int		f_fifo = FALSE ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		geekbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sanity/procfile: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0)) {
	    fname = STDINFNAME ;
	    f_stdin = TRUE ;
	}

	if (! f_stdin) {
	    struct ustat	sb ;
	    int	rs1 = u_stat(fname,&sb) ;
	    if (rs1 >= 0) f_fifo = S_ISFIFO(sb.st_mode) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sanity/procfile: f_fifo=%u\n",f_fifo) ;
#endif
	if (rs >= 0) {
	    int		i = 0 ;
	    lbuf[i++] = 'r' ;
	    if (f_fifo) lbuf[i++] = 'n' ;
	    lbuf[i] = '\0' ;
	}

	if (rs >= 0) {
	    SHIO	ifile, *ifp = &ifile ;
	    if ((rs = shio_opene(ifp,fname,lbuf,0666,to_open)) >= 0) {
		int	f_zero ;

#if	CF_LINEBUFIN
	        if (pip->f.bufline)
	            shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;
#endif

	        while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("b_sanity/procfile: %u d=>%t<\n",len,
			lbuf,strlinelen(lbuf,len,50)) ;
	            }
#endif /* CF_DEBUG */

		    f_zero = (lbuf[0] == 0) ;
	            while (len && isEOL(lbuf[len-1])) len -= 1 ;

	            if (len > 0) {
	                cp = lbuf ;
	                cl = len ;
	                if ((rs >= 0) && lip->f.geekout) {
	                    cp = geekbuf ;
	                    cl = mkgeekout(pip,geekbuf,llen,lbuf,len) ;
	                }
	                if ((rs >= 0) && (cl > 0)) {
			    int		ll = cl ;
			    cchar	*lp = cp ;
			    cchar	*tp ;
			    while ((tp = strnchr(lp,ll,'\n')) != NULL) {
				if ((tp-lp) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfile: a%u l=>%t<\n",ll,
		lp, strlinelen(lp,(tp-lp),50)) ;
#endif
	                            rs = procoutline(pip,ofp,lp,(tp-lp)) ;
	                            wlen += rs ;
				}
			        ll -= ((tp+1)-lp) ;
			        lp = (tp+1) ;
				if (rs < 0) break ;
			    } /* end while */
			    if ((rs >= 0) && (ll > 0)) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfile: b%u l=>%t<\n",ll,
		lp, strlinelen(lp,ll,50)) ;
#endif
	                        rs = procoutline(pip,ofp,lp,ll) ;
	                        wlen += rs ;
			    }
	                } /* end if */
	            } else if (! f_zero) {
	                rs = shio_print(ofp,lbuf,0) ;
	                wlen += rs ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while */

	        rs1 = shio_close(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (shio) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sanity/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procoutline(PROGINFO *pip,void *ofp,cchar *lbuf,int llen)
{
	LOCINFO		*lip = pip->lip ;
	LINEFOLD	liner ;
	int		rs ;
	int		rs1 ;
	int		n ;
	int		ni ;
	int		ll ;
	int		wlen = 0 ;
	const char	*fmt = "%t%t\n" ;
	const char	*lp ;

	n = lip->linelen ;
	ni = MIN(NBLANKS,lip->indent) ;
	if ((rs = linefold_start(&liner,n,ni,lbuf,llen)) >= 0) {
	    int	i ;
	    for (i = 0 ; (ll = linefold_getline(&liner,i,&lp)) > 0 ; i += 1) {
	        if (i > 0) {
	            rs = shio_printf(ofp,fmt,blanks,ni,lp,ll) ;
	        } else {
	            rs = shio_print(ofp,lp,ll) ;
	        }
	        wlen += rs ;
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = linefold_finish(&liner) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */

	return (rs >= 0) ? wlen : rs ;
}
/* end if (procoutline) */


static int mkgeekout(PROGINFO *pip,char *gbuf,int glen,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		linelen ;
	int		i ;
	int		j = 0 ;
	int		n = 0 ;
	int		nw = 0 ;
	int		ch ;

	linelen = lip->linelen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	debugprintf("mkgeekout: ent linelen=%u glen=%u\n",linelen,glen) ;
	debugprintf("mkgeekout: sl=%u\n",sl) ;
	}
#endif

	for (i = 0 ; (i < sl) && (j < glen) ; i += 1) {
	    ch = (*sp++ & 0xff) ;
	    if (ch == '\n') {
		nw = 0 ;
		n = 0 ;
	        if (j < glen) gbuf[j++] = '\n' ;
	    } else if (isOther(ch)) {
		nw = 0 ;
		n = 0 ;
	        if (j < glen) gbuf[j++] = ch ;
	    } else if (isprintlatin(ch)) {
		nw = 0 ;
		if (n++ >= linelen) {
		    n = 0 ;
	            if (j < glen) gbuf[j++] = CH_NL ;
		}
	        if (j < glen) gbuf[j++] = ch ;
	    } else {
		if (nw++ == 0) {
		    n = 0 ;
		    if (j < glen) gbuf[j++] = '\n' ;
		}
	    } /* end if */
	} /* end for (reading characters) */

	return j ;
}
/* end subroutine (mkgeekout) */


static int isOther(int ch)
{
	int	f = FALSE ;
	ch &= UCHAR_MAX ;
	f = f || (ch == CH_SP) ;
	f = f || (ch == CH_TAB) ;
	f = f || (ch == CH_VT) ;
	f = f || (ch == CH_CR) ;
	return f ;
}
/* end if (isOther) */


