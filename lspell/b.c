/* b_lspell */

/* this is a generic front-end for the LSPELL command */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	The program was written from scratch.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a simple program (of some sort!).


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
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<field.h>
#include	<ascii.h>
#include	<strpack.h>
#include	<hdb.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"config.h"
#include	"defs.h"
#include	"xwords.h"


/* local defines */

#ifndef	PO_OPTION
#define	PO_OPTION	"option"
#endif

#ifndef	DEFTABSIZE
#define	DEFTABSIZE	1000
#endif

#ifndef	BUFLEN
#define	BUFLEN		4096
#endif


/* external subroutines */

extern int	sfword(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;
extern int	isalphalatin(int) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		foldcase:1 ;
	uint		uniq:1 ;
	uint		counts:1 ;
} ;

struct locinfo {
	struct proginfo	*pip ;
	vecstr		stores ;
	STRPACK		wstore ;
	HDB		wdb ;
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	open ;
} ;

struct wordent {
	const char	*word ;
	int		count ;
} ;


/* forward references */

int		p_lspell(int,const char **,const char **,void *) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
static int	locinfo_finish(struct locinfo *) ;

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procargs(struct proginfo *,struct arginfo *,BITS *,PARAMOPT *,
			void *,uchar *,const char *,const char *) ;
static int	procfile(struct proginfo *,void *,uchar *,const char *) ;
static int	procword(struct proginfo *,const char *,int) ;
static int	procoutcounts(struct proginfo *,void *) ;
static int	printuniq(struct proginfo *,SHIO *,const char *,int) ;

static int	mkourterms(uchar *) ;

#ifdef	COMMENT
static int	isallalpha(const char *,int) ;
#endif

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"alpha",
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
	argopt_if,
	argopt_alpha,
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

static const char	*akonames[] = {
	"foldcase",
	"uniq",
	"counts",
	NULL
} ;

enum akonames {
	akoname_foldcase,
	akoname_uniq,
	akoname_counts,
	akoname_overlast
} ;


/* exported subroutines */


int b_lspell(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	int	rs = SR_OK ;
	int	ex = EX_SOFTWARE ;

	if (contextp != NULL) rs = lib_initenviron() ;

	if (rs >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = p_lspell(argc,argv,envv,contextp) ;
	}

	return ex ;
}
/* end subroutine (b_lspell) */


int p_lspell(argc,argv,envv,contextp)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	struct arginfo	ainfo ;

	SIGMAN		sm ;

	BITS		pargs ;

	KEYOPT		akopts ;

	PARAMOPT	aparams ;

	SHIO		errfile ;
	SHIO		outfile, *ofp = &outfile ;

	unsigned char	wterms[32] ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	rs, rs1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cp ;


	if_exit = 0 ;
	if_int = 0 ;

	rs = sigman_start(&sm, sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto badsigman ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("b_lspell: starting\n") ;
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
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

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

/* do we have a keyword or only key letters? */

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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->tmpdname = argp ;
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

/* argument-list file */
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                    }
	                    break ;

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ifname = argp ;
	                    }
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'c':
	                        lip->final.counts = TRUE ;
	                        lip->have.counts = TRUE ;
	                        lip->f.counts = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->f.counts = (rs > 0) ;
	                            }
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'u':
	                        lip->f.uniq = TRUE ;
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
	    debugprintf("b_lspell: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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

/* check a few more things */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

/* set the options for processing */

/* field terminators */

	mkourterms(wterms) ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4)) {
	    for (i = 0 ; i < 256 ; i += 1) {
	        if (BATST(wterms,i)) {
	            debugprintf("b_lspell: terms> %3u %c\n",
	                i,
	                ((isprint(i)) ? i : ' ')) ;
	        }
	    }
	}
#endif /* CF_DEBUG */

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* open the output file (if we are not processing in place that it) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_lspell: opening output file=%s\n",ofname) ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {
	    struct arginfo	*aip = &ainfo ;
	    PARAMOPT	*app = &aparams ;
	    const char	*afn = afname ;
	    const char	*ifn = ifname ;
	    uchar	*wt = wterms ;
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	        if ((rs = procargs(pip,aip,&pargs,app,ofp,wt,ifn,afn)) >= 0) {
	            if (lip->f.counts) {
	                rs = procoutcounts(pip,ofp) ;
	            }
	        } /* end if (procargs) */
	    } /* end if (procopts) */
	    shio_close(ofp) ;
	} else {
	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,"%s: inaccesssible output file (%d)\n",
	        pip->progname,rs) ;
	}

/* finished */
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet)
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        shio_printf(pip->efp,"%s: file unavailable (%d)\n",
	            pip->progname,rs) ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

retearly:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_lspell: exiting ex=%u rs=%d\n",ex,rs) ;
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
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_lspell: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

badsigman:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (b_lspell) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{

	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */

}
/* end subroutine (sighand_int) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	const int	n = (20*1024) ;
	const int	ssize = (10*1024) ;

	int	rs ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;

	if ((rs = strpack_start(&lip->wstore,ssize)) >= 0) {
	    rs = hdb_start(&lip->wdb,n,1,NULL,NULL) ;
	    if (rs < 0)
	        strpack_finish(&lip->wstore) ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (lip == NULL)
	    return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = strpack_finish(&lip->wstore) ;
	if (rs >= 0) rs = rs1 ;

	{
	    HDB		*dbp = &lip->wdb ;
	    HDB_CUR	cur ;
	    HDB_DATUM	k, v ;
	    if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {
	        while (hdb_enum(dbp,&cur,&k,&v) >= 0) {
	            struct wordent	*ep = (struct wordent *) v.buf ;
	            rs1 = uc_free(ep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end while */
	        hdb_curend(dbp,&cur) ;
	    } /* end if (cursor) */
	    if (rs >= 0) rs = rs1 ;
	} /* end block */

	rs1 = hdb_finish(&lip->wdb) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int	rs = SR_OK ;
	int	vnlen = 0 ;


	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        vnlen = strnlen(vp,vl) ;
	        if ((rs = vecstr_add(&lip->stores,vp,vnlen)) >= 0) {
	            rs = vecstr_get(&lip->stores,rs,epp) ;
	        } /* end if (added new entry) */
	    } /* end if (had a new entry) */

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (locinfo_setentry) */


static int locinfo_storeword(lip,wp,wl)
struct locinfo	*lip ;
const char	*wp ;
int		wl ;
{
	struct proginfo	*pip = lip->pip ;

	HDB		*dbp = &lip->wdb ;
	HDB_DATUM	k, v ;

	const int	esize = sizeof(struct wordent) ;

	int	rs ;
	int	f = TRUE ;

	const char	*sp ;

	if (wl < 0) wl = strlen(wp) ;

	k.buf = wp ;
	k.len = wl ;
	v.buf = NULL ;
	v.len = esize ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    struct wordent	*ep = (struct wordent *) v.buf ;
	    ep->count += 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_lspell/locinfo_storeword: found\n") ;
#endif

	} else if (rs == SR_NOTFOUND) {
	    struct wordent	*ep ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_lspell/locinfo_storeword: not_found\n") ;
#endif

	    f = FALSE ;
	    if ((rs = uc_malloc(esize,&ep)) >= 0) {
	        if ((rs = strpack_store(&lip->wstore,wp,wl,&sp)) >= 0) {
	            ep->word = sp ;
	            ep->count = 1 ;
	            k.buf = sp ;
	            k.len = wl ;
	            v.buf = ep ;
	            v.len = esize ;
	            rs = hdb_store(dbp,k,v) ;
	        }
	        if (rs < 0)
	            uc_free(ep) ;
	    } /* end if (entry-allocation) */

	} /* end if (not-found) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_lspell/locinfo_storeword: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_storeword) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [<file(s)> ...] [-af <afile>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	c = 0 ;

	const char	*cp ;


	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL)
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

	                case akoname_foldcase:
	                    if (! lip->final.foldcase) {
	                        lip->have.foldcase = TRUE ;
	                        lip->final.foldcase = TRUE ;
	                        lip->f.foldcase = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.foldcase = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_uniq:
	                    if (! lip->final.uniq) {
	                        lip->have.uniq = TRUE ;
	                        lip->final.uniq = TRUE ;
	                        lip->f.uniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.uniq = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_counts:
	                    if (! lip->final.counts) {
	                        lip->have.counts = TRUE ;
	                        lip->final.counts = TRUE ;
	                        lip->f.counts = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.counts = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                default:
	                    rs = SR_INVALID ;
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


static int procargs(pip,aip,bop,app,ofp,wterms,ifname,afname)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
PARAMOPT	*app ;
void		*ofp ;
uchar		wterms[] ;
const char	*ifname ;
const char	*afname ;
{
	int	rs = SR_OK ;
	int	pan = 0 ;
	const char	*cp ;

	if (rs >= 0) {
	    int	ai ;
	    int	f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (! f) continue ;

	        cp = aip->argv[ai] ;
	        pan += 1 ;
	        rs = procfile(pip,ofp,wterms,cp) ;

	        if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	        if ((rs >= 0) && if_int) rs = SR_INTR ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_lspell: afname=%s\n",afname) ;
#endif

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = procfile(pip,ofp,wterms,cp) ;

	            if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	            if ((rs >= 0) && if_int) rs = SR_INTR ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        shio_close(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: argument file inaccessible (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_lspell: afname rs=%d\n",rs) ;
#endif

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (ifname != NULL) && (ifname[0] != '\0')) {

	    cp = ifname ;
	    pan += 1 ;
	    rs = procfile(pip,ofp,wterms,cp) ;

	} /* end if */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procfile(pip,ofp,wterms,cp) ;

	} /* end if */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procfile(pip,ofp,wterms,fname)
struct proginfo	*pip ;
void		*ofp ;
unsigned char	wterms[] ;
const char	fname[] ;
{
	struct locinfo	*lip = pip->lip ;

	FIELD	fsb ;

	XWORDS	w ;

	SHIO	infile, *ifp = &infile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	sl, cl ;
	int	wlen = 0 ;

	const char	*sp ;
	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_lspell/procfile: fname=%s\n",fname) ;
#endif

	if (ofp == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0))
	    fname = STDINFNAME ;

	if ((rs = shio_open(&infile,fname,"r",0666)) >= 0) {
	    int	len ;

	    while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	        len = rs ;

	        if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	            int		fl ;
	            const char	*fp ;

	            while ((fl = field_word(&fsb,wterms,&fp)) >= 0) {
	                if (fl == 0) continue ;

	                if ((sl = sfword(fp,fl,&sp)) > 0) {
	                    if ((rs = xwords_start(&w,sp,sl)) >= 0) {
	                        int	i = 0 ;

	                        while ((cl = xwords_get(&w,i++,&cp)) > 0) {

	                            if (lip->f.counts) {
	                                rs = procword(pip,cp,cl) ;
	                            } else if (lip->f.uniq) {
	                                rs = printuniq(pip,ofp,cp,cl) ;
	                                wlen += rs ;
	                            } else {
	                                rs = shio_printf(ofp,"%t\n",cp,cl) ;
	                                wlen += rs ;
	                            }
	                            if (rs < 0) break ;
	                        } /* end while */

	                        xwords_finish(&w) ;
	                    } /* end if (xwords) */
	                } /* end if (have word) */

	                if (rs < 0) break ;
	            } /* end while (fielding words) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	        if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	        if ((rs >= 0) && if_int) rs = SR_INTR ;
	        if (rs < 0) break ;
	    } /* end while */

	    shio_close(ifp) ;
	} /* end if (opened file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_lspell/procfile: ret rs=%d c=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procword(struct proginfo *pip,const char *wp,int wl)
{
	struct locinfo	*lip = pip->lip ;

	int	rs ;

	rs = locinfo_storeword(lip,wp,wl) ;

	return rs ;
}
/* end subroutine (procword) */


static int procoutcounts(struct proginfo *pip,void *ofp)
{
	struct locinfo	*lip = pip->lip ;

	HDB		*dbp ;
	HDB_CUR		cur ;
	HDB_DATUM	k, v ;

	int	rs ;
	int	wlen = 0 ;

	dbp = &lip->wdb ;
	if ((rs = hdb_curbegin(dbp,&cur)) >= 0) {
	    struct wordent	*ep ;
	    const char	*fmt = "%-6u %t\n" ;
	    const char	*wp ;
	    int		wl ;
	    int		count ;
	    while ((rs = hdb_enum(dbp,&cur,&k,&v)) >= 0) {
	        ep = (struct wordent *) v.buf ;
	        wp = (const char *) k.buf ;
	        wl = k.len ;
	        count = ep->count ;
	        rs = shio_printf(ofp,fmt,count,wp,wl) ;
	        wlen += rs ;
	        if (rs < 0) break ;
	    } /* end while */
	    if (rs == SR_NOTFOUND) rs = SR_OK ;
	    hdb_curend(dbp,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutcounts) */


static int printuniq(pip,ofp,wp,wl)
struct proginfo	*pip ;
SHIO		*ofp ;
const char	*wp ;
int		wl ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs ;
	int	wlen = 0 ;

	if ((rs = locinfo_storeword(lip,wp,wl)) == 0) {
	    rs = shio_printline(ofp,wp,wl) ;
	    wlen += rs ;
	} /* end if (is unique) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printuniq) */


static int mkourterms(uchar *wterms)
{
	int	i ;

	for (i = 0 ; i < 32 ; i += 1)
	    wterms[i] = 0xFF ;

	BACLR(wterms,'_') ;
	BACLR(wterms,'-') ;

	BACLR(wterms,CH_SQUOTE) ;

	for (i = 0x20 ; i < 256 ; i += 1) {
	    if (isalphalatin(i))
	        BACLR(wterms,i) ;
	} /* end for */

	return 0 ;
}
/* end subroutine (mkourterms) */


#ifdef	COMMENT

static int isallalpha(s,slen)
const char	s[] ;
int		slen ;
{
	int	fl, i ;
	int	f_alpha = TRUE ;


	if (slen < 0)
	    slen = INT_MAX ;

	for (i = 0 ; (i < slen) && s[i] ; i += 1) {
	    if (! isalpha(s[i])) {
	        f_alpha = FALSE ;
	        break ;
	    }
	} /* end for */

	fl = (f_alpha) ? i : -1 ;

#if	CF_DEBUGS && 0
	debugprintf("isallalpha: f_alpha=%d fl=%d\n",f_alpha,fl) ;
#endif

	return fl ;
}
/* end subroutine (isallalpha) */

#endif /* COMMENT */


