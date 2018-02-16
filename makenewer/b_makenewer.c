/* b_makenewer */

/* SHELL built-in to return load averages */
/* generic short program front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_ALWAYS	1		/* always update the target? */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the main subroutine for MAKENEWER.
	This was a fairly generic subroutine adpapted for this program.

	Usage notes:

	+ suffix-mapping (uses the '-m' option)
	This maps specified names (as given as invocation arguments
	of with the argument-list input) to source files.

	+ suffix-substitution (uses the '-s' option)
	This maps specified source files to destination files


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
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<field.h>
#include	<fsdirtree.h>
#include	<sigblock.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_makenewer.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#define	CLEANBUFLEN	40

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	removes(const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	isdigitlatin(int) ;
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
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		nochange:1 ;
	uint		print:1 ;
	uint		zero:1 ;
	uint		rmfile:1 ;	/* remove destination files first */
	uint		rmsuf:1 ;
	uint		follow:1 ;
	uint		iacc:1 ;
	uint		nostop:1 ;
	uint		younger:1 ;
	uint		im:1 ;
} ;

struct locinfo {
	VECSTR		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	PROGINFO	*pip ;
	vecstr		sufmaps ;
	vecstr		sufsubs ;
	const char	*tardname ;
	uid_t		euid ;
	int		younger ;
	int		c_processed ;
	int		c_updated ;
} ;


/* forward references */

static int	mainsub(int,const char **,const char **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_tardname(LOCINFO *,const char *) ;
static int	locinfo_sufbegin(LOCINFO *) ;
static int	locinfo_sufend(LOCINFO *) ;
static int	locinfo_sufmap(LOCINFO *,cchar *,int) ;
static int	locinfo_finish(LOCINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procsufsub(PROGINFO *,const char *,int) ;
static int	procsufxxx(PROGINFO *,vecstr *,const char *,int) ;
static int	procsufadd(PROGINFO *,vecstr *,const char *,int) ;
static int	proctouchfile(PROGINFO *,const char *) ;
static int	procname(PROGINFO *,void *,const char *) ;
static int	procdir(PROGINFO *,void *,cchar *,FSDIRTREESTAT *) ;
static int	procfile(PROGINFO *,void *,cchar *,FSDIRTREESTAT *) ;
static int	procfiler(PROGINFO *,void *,struct ustat *,cchar *) ;
static int	procdisposition(PROGINFO *,const char *,int) ;
static int	procfilesuf(PROGINFO *,vecstr *,char *,cchar *,int) ;
static int	openaccess(cchar *,int,mode_t,int) ;

static int	procout_begin(PROGINFO *,void *,const char *) ;
static int	procout_end(PROGINFO *,void *) ;

static int	mknewfname(char *,const char *,const char *,cchar *,cchar *) ;

static int	sufclean(char *,int,const char *,int) ;

#ifdef	COMMENT
static int	mkpdirs(const char *,mode_t) ;
#endif


/* local variables */

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
	"im",
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
	argopt_im,
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
	"rmfile",
	"rmsuf",
	"follow",
	"nostop",
	"younger",
	"im",
	NULL
} ;

enum akonames {
	akoname_rmfile,
	akoname_rmsuf,
	akoname_follow,
	akoname_nostop,
	akoname_younger,
	akoname_im,
	akoname_overlast
} ;

static const char	*suffers[] = {
	"sufsub",
	"sufmap",
	NULL
} ;

enum suffers {
	suffer_sub,
	suffer_map,
	suffer_overlast
} ;


/* exported subroutines */


int b_makenewer(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_makenewer) */


int p_makenewer(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_makenewer) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	struct ustat	sb ;
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
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*tardname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*touchfname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
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

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

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

	            argval = NULL ;

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
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program searchname */
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

	                case argopt_im:
	                    if (! lip->final.im) {
	                        lip->final.im = TRUE ;
	                        lip->f.im = TRUE ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* continue on error */
	                    case 'c':
	                        lip->final.nostop = TRUE ;
	                        lip->have.nostop = TRUE ;
	                        lip->f.nostop = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.nostop = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* directory name */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                tardname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* option: follow (symlinks) */
	                    case 'f':
	                        lip->have.follow = TRUE ;
	                        lip->final.follow = TRUE ;
	                        lip->f.follow = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.follow = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* no-change */
	                    case 'n':
	                        lip->f.nochange = TRUE ;
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

/* print something! */
	                    case 'p':
	                        lip->f.print = TRUE ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* remove destination files before copying over */
	                    case 'r':
	                        lip->final.rmfile = TRUE ;
	                        lip->have.rmfile = TRUE ;
	                        lip->f.rmfile = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.rmfile = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* suffix mapping */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_sufmap(lip,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* suffix substitution */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = procsufsub(pip,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* touch file */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                touchfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->final.younger = TRUE ;
	                                lip->have.younger = TRUE ;
	                                rs = cfdecti(argp,argl,&v) ;
	                                lip->younger = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* zero files (are OK) */
	                    case 'z':
	                        lip->f.zero = TRUE ;
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

	        } /* end if (digits or options) */

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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* program root */

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

/* help */

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

/* check a few more things */

	if ((argval != NULL) && (! lip->have.younger)) {
	    rs = cfdecti(argval,-1,&v) ;
	    lip->final.younger = TRUE ;
	    lip->have.younger = TRUE ;
	    lip->younger = v ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	    if (rs < 0) ex = EX_USAGE ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get a destination directory if we do not already have one */

	if ((rs >= 0) && (tardname == NULL)) {
	    if ((rs = bits_count(&pargs)) > 1) {
	        for (ai = (argc - 1) ; ai > 1 ; ai -= 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: argv[%u]=%s\n",ai,argv[ai]) ;
#endif

	            f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	            f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	            if (f) {
	                cp = argv[ai] ;
	                if (cp[0] != '\0') {
	                    tardname = cp ;
	                    bits_clear(&pargs,ai) ;
	                    break ;
		        }
		    }

	        } /* end for */
	    } /* end if (more than one positional argument) */
	} /* end if (getting a directory) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: tardname=%s\n",tardname) ;
	    debugprintf("main: rmfile=%u\n",lip->f.rmfile) ;
	    debugprintf("main: rmsuf=%u\n",lip->f.rmsuf) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: tardname=%s\n",pn,tardname) ;
	    shio_printf(pip->efp,"%s: rmfile=%u\n",pn,lip->f.rmfile) ;
	    shio_printf(pip->efp,"%s: rmsuf=%u\n",pn,lip->f.rmsuf) ;
	}

/* does the target directory exist? */

	if ((rs >= 0) && (tardname == NULL)) {
	    rs = SR_INVALID ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: no directory was specified\n",
	        pip->progname) ;
	}

	if (rs >= 0) {
	    rs = uc_stat(tardname,&sb) ;
	    if (isNotPresent(rs) || (! S_ISDIR(sb.st_mode))) {
	        if (rs >= 0) rs = SR_NOTDIR ;
	        ex = EX_USAGE ;
	        shio_printf(pip->efp,
	            "%s: target directory inaccessible (%d)\n",
	            pip->progname,rs) ;
	        shio_printf(pip->efp,"%s: tardir=%s\n",
	            pip->progname,tardname) ;
	    }
	} /* end if */

	if (rs >= 0) {
	    rs = locinfo_tardname(lip,tardname) ;
	}

/* debug information */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    vecstr	*lp ;
	    int		type, i ;
	    cchar	*pn = pip->progname ;
	    cchar	*name ;
	    for (type = 0 ; suffers[type] != NULL ; type += 1) {
	        name  = suffers[type] ;
	        switch (type) {
	        case suffer_sub:
	            lp = &lip->sufsubs ;
	            break ;
	        case suffer_map:
	            lp = &lip->sufmaps ;
	            break ;
	        }
	        for (i = 0 ; vecstr_get(lp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                shio_printf(pip->efp,"%s: %s %s\n",pn,name,cp) ;
		    }
	        } /* end for */
	    } /* end for */
	} /* end if */

/* do the invocation arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: ai_max=%u\n",ai_max) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procargs(pip,&ainfo,&pargs,ofname,afname)) >= 0) {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;

	        if (lip->f.print || (pip->verboselevel > 1)) {
		    fmt = "files processed=%u updated=%u\n" ;
	            shio_printf(ofp,fmt,lip->c_processed,lip->c_updated) ;
		}

	        if (pip->debuglevel > 0) {
		    const int	c_processed = lip->c_processed ;
		    const int	c_updated = lip->c_updated ;
		    fmt = "%s: files processed=%u updated=%u\n" ;
	            shio_printf(pip->efp,fmt,pn,c_processed,c_updated) ;
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("main: debuglevel=%u quiet=%u\n",
	                pip->debuglevel,pip->f.quiet) ;
	            debugprintf("main: past main loop rs=%d ex=%u\n",rs,ex) ;
	            debugprintf("main: processed=%d updated=%d\n",
	                lip->c_processed,lip->c_updated) ;
	        }
#endif

	        if ((touchfname != NULL) && (touchfname[0] != '\0')) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("main: touchfname=%s\n",touchfname) ;
#endif
	            rs = proctouchfile(pip,touchfname) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("main: proctouchfile() rs=%d\n",rs) ;
#endif
	        }

	    } /* end if (procargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (! pip->f.quiet)) {
	    const char	*pn = pip->progname ;
	    const char	*fmt = "%s: could not perform function (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done rs=%d ex=%u\n",rs,ex) ;
#endif

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
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

/* good return from program */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u rs=%d\n",ex,rs) ;
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
	    debugprintf("b_makenewer: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	if (pip->efp != NULL) {

	    fmt = "%s: USAGE> %s <file(s)> [-af <afile>] "
	        "{<dstdir>|{-d <dstdir>}}\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-im] [-z] [-r] [-t <touchfile>] [-m <o>=<n>] "
	        "[-s <o>=<n>]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->euid = geteuid() ;

	rs = locinfo_sufbegin(lip) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	rs1 = locinfo_sufend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_sufbegin(LOCINFO *lip)
{
	vecstr		*smp = &lip->sufmaps ;
	int		rs ;

	if ((rs = vecstr_start(smp,10,0)) >= 0) {
	    rs = vecstr_start(&lip->sufsubs,10,0) ;
	    if (rs < 0)
	        vecstr_finish(smp) ;
	}

	return rs ;
}
/* end subroutine (locinfo_sufbegin) */


static int locinfo_sufend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&lip->sufsubs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&lip->sufmaps) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_sufend) */


static int locinfo_tardname(LOCINFO *lip,const char *tardname)
{

	if (lip == NULL)
	    return SR_FAULT ;

	if (tardname == NULL) return SR_FAULT ;
	if (tardname[0] == '\0') return SR_INVALID ;

	lip->tardname = tardname ;

	return SR_OK ;
}
/* end subroutine (locinfo_tardname) */


static int locinfo_sufmap(LOCINFO *lip,cchar *sp,int sl)
{
	PROGINFO	*pip = lip->pip ;
	vecstr		*lp = &lip->sufmaps ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("locinfo_sufmap: suf> %t <\n",sp,sl) ;
#endif

	if ((rs = procsufxxx(pip,lp,sp,sl)) >= 0) {
	    if (pip->debuglevel > 0) {
		int	i ;
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: %s %s\n" ;
		cchar	*name = "sufmap" ;
		cchar	*cp ;
	        for (i = 0 ; vecstr_get(lp,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) {
	                shio_printf(pip->efp,fmt,pn,name,cp) ;
		    }
	        } /* end for */
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_sufmap) */


static int procsufsub(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*lp ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("procsufsub: suf> %t <\n",sp,sl) ;
#endif

	lp = &lip->sufsubs ;
	rs = procsufxxx(pip,lp,sp,sl) ;

	return rs ;
}
/* end subroutine (procsufsub) */


static int procsufxxx(PROGINFO *pip,vecstr *slp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp ;

	if (sl < 0) sl = strlen(sp) ;

	while ((rs >= 0) && ((tp = strnchr(sp,sl,',')) != NULL)) {
	    c += 1 ;
	    rs = procsufadd(pip,slp,sp,(tp - sp)) ;
	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    c += 1 ;
	    rs = procsufadd(pip,slp,sp,sl) ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsufxxx) */


static int procsufadd(PROGINFO *pip,vecstr *slp,cchar *sp,int sl)
{
	const int	clen = CLEANBUFLEN ;
	int		rs ;
	char		cbuf[CLEANBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = sufclean(cbuf,clen,sp,sl)) >= 0) {
	    if (rs > 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    if (rs >= 0) {
	        rs = vecstr_add(slp,sp,sl) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("procsufadd: add=>%t<\n",sp,sl) ;
	    debugprintf("procsufadd: procsufadd() rs=%d\n",rs) ;
	}
#endif

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procsufadd) */


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

	                case akoname_rmsuf:
	                    if (! lip->final.rmsuf) {
	                        lip->have.rmsuf = TRUE ;
	                        lip->final.rmsuf = TRUE ;
	                        lip->f.rmsuf = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.rmsuf = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_rmfile:
	                    if (! lip->final.rmfile) {
	                        lip->have.rmfile = TRUE ;
	                        lip->final.rmfile = TRUE ;
	                        lip->f.rmfile = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.rmfile = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_follow:
	                    if (! lip->final.follow) {
	                        lip->have.follow = TRUE ;
	                        lip->final.follow = TRUE ;
	                        lip->f.follow = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.follow = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_nostop:
	                    if (! lip->final.nostop) {
	                        lip->have.nostop = TRUE ;
	                        lip->final.nostop = TRUE ;
	                        lip->f.nostop = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.nostop = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_younger:
	                    if (! lip->final.younger) {
	                        lip->have.younger = TRUE ;
	                        lip->final.younger = TRUE ;
	                        lip->f.younger = TRUE ;
	                        if (vl > 0) {
	                            int	v ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->younger = v ;
	                        }
	                    }
	                    break ;

	                case akoname_im:
	                    if (! lip->final.im) {
	                        lip->final.im = TRUE ;
	                        lip->f.im = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.im = (rs > 0) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_makenewer/procopts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = procout_begin(pip,ofp,ofn)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
		cchar	**argv = aip->argv ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procname(pip,ofp,cp) ;
	                }
	            }

	            if (rs < 0) {
	                shio_printf(pip->efp,
	                    "%s: failure in processing name=%s (%d)\n",
	                    pip->progname,cp,rs) ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (looping through command arguments) */
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

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    lbuf[(cp-lbuf)+cl] = '\0' ;
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procname(pip,ofp,cp) ;
	                    }
	                }

	                if (rs < 0) {
			    fmt = "%s: procesing failure in name=%s (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,cp,rs) ;
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while */

	            rs1 = shio_close(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: argument file inaccessible (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if (opened) */

	    } /* end if (argument file) */

	    if ((rs >= 0) && (pan == 0) && (! lip->f.zero)) {
	        rs = SR_INVALID ;
	        if (! pip->f.quiet) {
		    fmt = "%s: no files were specified\n" ;
	            shio_printf(pip->efp,fmt,pn) ;
		}
	    }

	    rs1 = procout_end(pip,ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procout) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procout_begin(PROGINFO *pip,void *ofp,const char *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (lip->f.print || (pip->verboselevel > 1)) {
	    if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-')) {
	        ofn = STDOUTFNAME ;
	    }
	    rs = shio_open(ofp,ofn,"wct",0644) ;
	} /* end if (printing enabled) */

	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,void *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.print || (pip->verboselevel > 1)) {
	    const char	*pn = pip->progname ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs1 < 0) {
		cchar	*fmt = "%s: inaccessible output (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs1) ;
	    }
	}

	return rs ;
}
/* end subroutine (procout_end) */


static int proctouchfile(PROGINFO *pip,cchar *touchfname)
{
	SHIO		touchfile, *tfp = &touchfile ;
	int		rs ;
	int		rs1 ;
	int		f_write = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

/* sanity check */

	if (touchfname == NULL) return SR_FAULT ;

	if (touchfname[0] == '\0') return SR_INVALID ;

/* go */

	if ((rs = shio_open(tfp,touchfname,"w",0666)) >= 0) {

#if	CF_ALWAYS
	    shio_printf(tfp,"%s\n",
	        timestr_logz(pip->daytime,timebuf)) ;
#else
	    if (f_write || (c_updated > 0)) {
	        shio_printf(tfp,"%s\n",
	            timestr_logz(pip->daytime,timebuf)) ;
	    }
#endif /* CF_ALWAYS */

	    rs1 = shio_close(tfp) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (rs == SR_NOENT) {
	    f_write = TRUE ;
	    rs = shio_open(tfp,touchfname,"wct",0666) ;
	} /* end if (file opened) */

	return (rs >= 0) ? f_write : rs ;
}
/* end subroutine (proctouchfile) */


static int procname(PROGINFO *pip,void *ofp,cchar *name)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	char		mfname[MAXPATHLEN+1] ;

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: ent name=%s\n",name) ;
#endif

/* form the actual file name to be processed after suffix mapping */

	if ((rs = procfilesuf(pip,&lip->sufmaps,mfname,name,-1)) >= 0) {
	    FSDIRTREE_STAT	sb, ssb, *sbp = &sb ;
	    int			f_islink = FALSE ;
	    int			f_isdir = FALSE ;

	    if (rs > 0) name = mfname ;

/* continue */

	    if ((rs = fsdirtreestat(name,1,&sb)) >= 0) { /* this is LSTAT */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            if (rs < 0) sb.st_mode = 0 ;
	            debugprintf("main/procname: fsdirtreestat() rs=%d\n", 
	                rs) ;
	            debugprintf("main/procname: mode=%0o\n",sb.st_mode) ;
	        }
#endif

	        f_isdir = S_ISDIR(sb.st_mode) ;
	        f_islink = S_ISLNK(sb.st_mode) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("main/procname: f_isdir=%u\n",f_isdir) ;
	            debugprintf("main/procname: f_islink=%u\n",f_islink) ;
	        }
#endif

	        if (f_islink && lip->f.follow) {

	            rs1 = fsdirtreestat(name,0,&ssb) ; /* this is STAT */
	            if (rs1 >= 0) {
	                f_isdir = S_ISDIR(ssb.st_mode) ;
	                sbp = &ssb ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procname: symlink dangle=%u\n",
	                    ((rs1 < 0)?1:0)) ;
#endif

	        } /* end if */

	        if (rs >= 0) {
	            if (f_isdir) {
	                rs = procdir(pip,ofp,name,sbp) ;
	            } else {
	                rs = procfile(pip,ofp,name,sbp) ;
		    }
	        }

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procname: fdirtreestat() rs=%d\n",rs) ;
#endif

	        if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	            if (lip->f.im) rs = SR_OK ;
	        }
	    } /* end if (fsdirtreestat) */

	    if (rs < 0) {
		rs = procdisposition(pip,name,rs) ;
	    }

	} /* end if (procfilesuf) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procname) */


static int procdir(PROGINFO *pip,void *ofp,cchar *name,FSDIRTREE_STAT *sbp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		opts ;
	int		size ;
	int		nl ;
	int		c = 0 ;
	char		mfname[MAXPATHLEN+1] ;
	char		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procdir: name=%s\n",name) ;
#endif

	nl = strlen(name) ;
	while ((nl > 0) && (name[nl-1] == '/')) nl -= 1 ;

/* do all of our children */

	size = (nl + 1 + MAXPATHLEN + 1) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    FSDIRTREE		d ;
	    FSDIRTREE_STAT	fsb, *fsp = &fsb ;
	    char		*fname = p ;
	    char		*bp ;

	    bp = strwcpy(fname,name,nl) ;
	    *bp++ = '/' ;

	    opts = 0 ;
	    if (lip->f.follow) opts |= FSDIRTREE_MFOLLOW ;

	    if ((rs = fsdirtree_open(&d,name,opts)) >= 0) {
	        vecstr		*slp = &lip->sufmaps ;
	        const int	mpl = MAXPATHLEN ;

	        while ((rs = fsdirtree_read(&d,fsp,bp,mpl)) > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procdir: direntry=%s\n",bp) ;
#endif

	            if (pip->debuglevel >= 2) {
	                shio_printf(pip->efp,"%s: looking fn=%s\n",
	                    pip->progname,fname) ;
	            }

	            if ((rs = procfilesuf(pip,slp,mfname,fname,-1)) >= 0) {
	                cchar	*pfname = fname ;
	                if (rs > 0) {
	                    pfname = mfname ;
	                    rs = fsdirtreestat(pfname,1,fsp) ; /* LSTAT */
	                }
	                if (rs >= 0) {
	                    rs = procfile(pip,ofp,pfname,fsp) ;
	                    if (rs > 0) c += 1 ;
	                }
	                if (rs < 0) rs = procdisposition(pip,pfname,rs) ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading entries) */

	        rs1 = fsdirtree_close(&d) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (reading directory entries) */

	    if (p != NULL) uc_free(p) ;
	} /* end if (memory allocation) */

/* do ourself */

	if (rs >= 0) {
	    rs = procfile(pip,ofp,name,sbp) ;
	    if (rs >= 0) c += 1 ;
	}

	if ((rs < 0) && (c == 0)) rs = procdisposition(pip,name,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdir) */


static int procfile(PROGINFO *pip,void *ofp,cchar *fname,FSDIRTREESTAT *sbp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f_updated = FALSE ;
	int		f = TRUE ;

	if ((fname[0] == '\0') || (fname[0] == '-'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfile: fname=%s\n",fname) ;
#endif

	lip->c_processed += 1 ;
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: fname=%s\n",pip->progname,fname) ;
	}

	if (lip->f.follow && S_ISLNK(sbp->st_mode)) {
	    rs = fsdirtreestat(fname,0,sbp) ; /* STAT */
	}

	if ((rs >= 0) && f) {
	    if (lip->younger > 0) {
	        f = ((pip->daytime - sbp->st_mtime) < lip->younger) ;
	    }
	}

	if ((rs >= 0) && f) {
	    f = S_ISREG(sbp->st_mode) ;
	}

/* continue with the processing operation */

	if ((rs >= 0) && f) {
	    SIGBLOCK	blocker ;
	    if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	        rs = procfiler(pip,ofp,sbp,fname) ;
	        f_updated = (rs > 0) ;

	        sigblock_finish(&blocker) ;
	    } /* end if (blocking signals) */
	} /* end if (processing issued) */

	if (rs < 0) rs = procdisposition(pip,fname,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procfile: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procfile) */


static int procfiler(PROGINFO *pip,void *ofp,struct ustat *ssbp,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	struct ustat	dsb ;
	const mode_t	nm = (ssbp->st_mode & (~ S_IFMT)) | 0600 ;
	uid_t		duid = -1 ;
	size_t		fsize = 0 ;
	int		rs = SR_OK ;
	int		bfl ;
	int		of ;
	int		f_create = FALSE ;
	int		f_update = FALSE ;
	int		f_updated = FALSE ;
	const char	*tardname ;
	const char	*bfp ;
	char		altfname[MAXPATHLEN + 2] ;
	char		dstfname[MAXPATHLEN + 2] ;
	char		tmpfname[MAXPATHLEN + 1] ;

	tardname = lip->tardname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procfiler: tardname=%s\n",tardname) ;
	    debugprintf("main/procfiler: fname=%s\n",fname) ;
	}
#endif

/* form the filename of the one in the directory */

	if ((bfl = sfbasename(fname,-1,&bfp)) > 0) {
	    if (lip->f.rmsuf) {
	        cchar	*tp = strnrchr(bfp,bfl,'.') ;
	        rs = mknewfname(dstfname,tardname,fname,tp,NULL) ;
	    } else {
		vecstr	*lp = &lip->sufsubs ;
	        if ((rs = procfilesuf(pip,lp,tmpfname,fname,-1)) >= 0) {
	            cchar	*fn = (rs > 0) ? tmpfname : fname ;
	            rs = mkpath2(dstfname,tardname,fn) ;
		}
	    } /* end if */
	} else {
	    rs = SR_NOENT ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procfiler: mid rs=%d\n",rs) ;
	    debugprintf("main/procfiler: dstfname=%s\n",dstfname) ;
	}
#endif

	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    if (S_ISREG(dsb.st_mode)) {
	        fsize = (size_t) dsb.st_size ;
	        if (lip->f.rmfile) {
	            f_create = TRUE ;
	        } else {
	            int	f = FALSE ;
	            f = f || (ssbp->st_mtime > dsb.st_mtime) ;
	            f = f || (ssbp->st_size != dsb.st_size) ;
	            if (f) f_update = TRUE ;
	            duid = dsb.st_uid ;
	        }
	    } else {
	        f_create = TRUE ;
	        if (S_ISDIR(dsb.st_mode)) {
	            rs = removes(dstfname) ;
	        } else {
	            rs = uc_unlink(dstfname) ;
		}
	    }

	} else {

	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procfiler: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int	dnl ;
	        cchar	*dnp ;
	        rs = SR_OK ;
	        if ((dnl = sfdirname(dstfname,-1,&dnp)) > 0) {
	            if ((rs = mkpath1w(tmpfname,dnp,dnl)) >= 0) {
	                rs = uc_unlink(tmpfname) ;
		    }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procfiler: no dst rs=%d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    cchar	*dnp ;
	    if ((dnl = sfdirname(dstfname,-1,&dnp)) > 0) {
	        if ((rs = mkpath1w(tmpfname,dnp,dnl)) >= 0) {
	            struct ustat	sb ;
	            const mode_t	dm = 0775 ;
	            int			rs1 = u_lstat(tmpfname,&sb) ;
	            int			f = FALSE ;
	            if ((rs1 = u_lstat(tmpfname,&sb)) >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = uc_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode)) {
	                    f = TRUE ;
			}
	                if (f) {
	                    rs = uc_unlink(tmpfname) ;
			}
	            } else {
	                f = TRUE ;
		    }
	            if (f) {
	                rs = mkdirs(tmpfname,dm) ;
		    }
	        }
	    }
	} /* end if (create) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: dst-dir rs=%d\n",rs) ;
#endif

/* update (or create) the target file */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: dstfname=%s\n",dstfname) ;
#endif

	        if (lip->f.print || (pip->verboselevel > 1)) {
	            shio_printf(ofp,"%s\n",dstfname) ;
		}

	if ((rs >= 0) && lip->f.rmfile) {
	    if ((rs = mkpath1(altfname,dstfname)) >= 0) {
		cchar	*aname = "mnXXXXXXXXXXXX" ;
		char	tbuf[MAXPATHLEN+1] ;
		if ((rs = mkpath2(tbuf,tardname,aname)) >= 0) {
		    rs = mktmpfile(dstfname,nm,tbuf) ;
		}
	    }
	}

	of = O_WRONLY ;
	if (f_create) of |= O_CREAT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    char	ofbuf[TIMEBUFLEN+1] ;
	    snopenflags(ofbuf,TIMEBUFLEN,of) ;
	    debugprintf("main/procfiler: mid rs=%d f_update=%u\n",rs,f_update) ;
	    debugprintf("main/procfiler: of=%s\n",ofbuf) ;
	}
#endif

	if (rs >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfiler: open dfn=%s\n",dstfname) ;
#endif
	    if ((rs = openaccess(dstfname,of,nm,f_create)) >= 0) {
	        int	dfd = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfiler: u_open() rs=%d\n",rs) ;
#endif

/* do we need to UPDATE the destination? */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfiler: update? rs=%d f_update=%d\n",
	                rs,f_update) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfiler: updating dstfname=%s\n",
	                dstfname) ;
#endif

		lip->c_updated += 1 ;

	        if (! lip->f.nochange) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procfiler: open fn=%s\n",fname) ;
#endif
	            if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	                const int	sfd = rs ;
	                int		len ;
	                if ((rs = uc_writedesc(dfd,sfd,-1)) >= 0) {
	                    f_updated = TRUE ;
	                    len = rs ;
	                    if (len < fsize) {
	                        offset_t	uoff = len ;
	                        rs = uc_ftruncate(dfd,uoff) ;
			    }
	                }
	                u_close(sfd) ;
	            } /* end if (open source) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: mid3 rs=%d\n",rs) ;
#endif
	            if (rs >= 0) {
	                int	f_utime = FALSE ;
	                f_utime = f_utime || (duid < 0) ;
	                f_utime = f_utime || (duid == lip->euid) ;
	                f_utime = f_utime || (lip->euid == 0) ;
	                if (f_utime) {
			    struct utimbuf	ut ;
			    u_close(dfd) ;
			    dfd = -1 ;
			    ut.actime = ssbp->st_atime ;
			    ut.modtime = ssbp->st_mtime ;
	                    rs = uc_utime(dstfname,&ut) ;
			}
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procfiler: mid4 rs=%d\n",rs) ;
	    debugprintf("main/procfiler: afn=%s\n",altfname) ;
	    debugprintf("main/procfiler: dfn=%s\n",dstfname) ;
	}
#endif
			if ((rs >= 0) && lip->f.rmfile) {
			    rs = uc_rename(dstfname,altfname) ;
			}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: mid5 rs=%d\n",rs) ;
#endif
	            } /* end if (ok) */
		    if ((rs < 0) && lip->f.rmfile) {
			uc_unlink(dstfname) ;
		    }
	        } /* end if (allowable actual update) */

	        if (dfd >= 0) u_close(dfd) ;
	    } /* end if (destination file opened) */
	} /* end if (ok) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfiler: ret rs=%d f_up=%u\n",rs,f_updated) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procfiler) */


static int procdisposition(PROGINFO *pip,cchar *name,int rs)
{
	LOCINFO		*lip = pip->lip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procdisposition: ent name=%s rs=%d\n",name,rs) ;
#endif


	if (rs < 0) {
	    if ((! pip->f.quiet) && (rs < 0)) {
	        shio_printf(pip->efp,"%s: name=%s (%d)\n",
	            pip->progname,name,rs) ;
	    }

	    if (rs == SR_ACCESS) {
	        if (lip->f.iacc) rs = SR_OK ;
	    } else if (rs == SR_NOENT) {
	        if (lip->f.zero) rs = SR_OK ;
	    } else if (isNotPresent(rs)) {
	        if (lip->f.nostop) rs = SR_OK ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procdisposition: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdisposition) */


static int procfilesuf(PROGINFO *pip,vecstr *slp,char *nfname,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		bnl ;
	int		fl = 0 ;
	const char	*tp ;
	const char	*bnp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procfilesuf: fname=%t\n",np,nl) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	nfname[0] = '\0' ;
	if ((bnl = sfbasename(np,nl,&bnp)) > 0) {
	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {
	        int	sl ;
	        cchar	*sp ;
	        cchar	*cp ;
	        char	sufbuf[SUFLEN + 1] ;

	        sl = ((bnp + bnl) - (tp + 1)) ;
	        sp = (tp + 1) ;

	        if ((rs1 = snwcpy(sufbuf,SUFLEN,sp,sl)) >= 0) {
	            rs1 = vecstr_search(slp,sufbuf,vstrkeycmp,&cp) ;
	        }

	        if (rs1 >= 0) {
	            rs = mknewfname(nfname,NULL,np,tp,cp) ;
	            fl = rs ;
	        }

	    } /* end if (had a suffix) */
	} /* end if (had a basename) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    if (rs >= 0) {
	        debugprintf("main/procfilesuf: nfname=%s\n",nfname) ;
	    }
	    debugprintf("main/procfilesuf: ret rs=%d fl=%u\n",rs,fl) ;
	}
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (procfilesuf) */


static int openaccess(cchar *dstfname,int of,mode_t nm,int f_create)
{
	int		rs ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("main/openaccess: ent fn=%s\n",dstfname) ;
	debugprintf("main/openaccess: ent f_create=%u\n",f_create) ;
#endif
	rs = uc_open(dstfname,of,nm) ;
	fd = rs ;
#if	CF_DEBUGS
	    debugprintf("main/openaccess: 1 u_open() rs=%d\n",rs) ;
#endif
	if ((rs == SR_ACCESS) && (! f_create)) {
	    int		dnl ;
	    const char	*dnp ;
	    if ((dnl = sfdirname(dstfname,-1,&dnp)) > 0) {
		char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath1w(tbuf,dnp,dnl)) >= 0) {
	        if ((rs = uc_access(tbuf,W_OK)) >= 0) {
	            f_create = TRUE ;
	            if ((rs = uc_unlink(dstfname)) >= 0) {
	                of |= O_CREAT ;
	                rs = uc_open(dstfname,of,nm) ;
			fd = rs ;
#if	CF_DEBUGS
	                    debugprintf("main/openaccess: 2 u_open() rs=%d\n",
	                        rs) ;
#endif
	            }
		    }
	        }
	    }
	} /* end if (alternate open) */
	if ((rs >= 0) && f_create) {
	        rs = uc_fminmod(fd,nm) ;
	} /* end if (needed to be created) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;
#if	CF_DEBUGS
	debugprintf("main/openaccess: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openaccess) */


static int mknewfname(char *rbuf,cchar *dname,cchar *fname,cchar *sp,cchar *cp)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	cchar		*tp ;

	if (dname != NULL) {
	    rs = storebuf_strw(rbuf,rlen,i,dname,-1) ;
	    i += rs ;
	    if (rs >= 0) {
	        rs = storebuf_char(rbuf,rlen,i,'/') ;
	        i += rs ;
	    }
	}

	if (rs >= 0) {
	    int		fl = (sp != NULL) ? (sp - fname) : -1 ;
	    rs = storebuf_strw(rbuf,rlen,i,fname,fl) ;
	    i += rs ;
	}

	if ((rs >= 0) && (cp != NULL) && ((tp = strchr(cp,'=')) != NULL)) {

	    if ((tp+1)[0] != '\0') {
	        rs = storebuf_char(rbuf,rlen,i,'.') ;
	        i += rs ;
	        if (rs >= 0) {
	            rs = storebuf_strw(rbuf,rlen,i,(tp + 1),-1) ;
	            i += rs ;
	        }
	    }

	} /* end if (adding new suffix) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mknewfname) */


static int sufclean(char *rbuf,int rlen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	cchar		*tp ;

	rbuf[0] = '\0' ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    int		cl ;
	    cchar	*cp ;

	    if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	        rs = storebuf_strw(rbuf,rlen,i,cp,cl) ;
	        i += rs ;
	    }

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	    if (rs >= 0) {
	        rs = storebuf_char(rbuf,rlen,i,'=') ;
	        i += rs ;
	    }

	    if ((rs >= 0) && (sl > 0)) {
	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = storebuf_strw(rbuf,rlen,i,cp,cl) ;
	            i += rs ;
	        }
	    } /* end if */

	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sufclean) */


#ifdef	COMMENT
static int mkpdirs(const char *tarfname,mode_t dm)
{
	int		rs = SR_OK ;
	int		dl ;
	cchar		*dp ;

	if ((dl = sfdirname(tarfname,-1,&dp)) > 0) {
	    char	dname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath1w(dname,dp,dl)) >= 0) {
	        uc_unlink(dname) ; /* just a little added help */
	        rs = mkdirs(dname,dm) ;
	    }
	} else {
	    rs = SR_NOENT ;
	}

	return rs ;
}
/* end subroutine (mkpdirs) */
#endif /* COMMENT */


