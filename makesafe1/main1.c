/* main */

/* generic short program front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_ALWAYS	1		/* always update the target? */
#define	CF_DELETER	1		/* include the deleter */


/* revision history:

	= 2004-05-14, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did. It used pieces from other (similar in some ways)
        programs. It is linted out and should be very clean -- we depend on this
        everyday to do what we need. Pieces not used in their full like where
        they originally were, are sort of hacked up to minimal code. Try not to
        get your knickers in a bunch over that.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the main subroutine for MAKESAFE. This was a fairly generic
        subroutine adpapted for this program. Note that parallel processing is
        enabled by default. If you do not want parallel processing for some
        reason use the '-o' invocation option to set the maximum parallelism to
        '1'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<psem.h>
#include	<fsdir.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dirlist.h"
#include	"cachetime.h"
#include	"fsi.h"
#include	"upt.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#define	DISP		struct disp_head
#define	DISP_ARGS	struct disp_args
#define	DISP_THR	struct disp_thr

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	getnprocessors(cchar **,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progfile(PROGINFO *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct subinfo {
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	IDS		id ;
	int		pan ;
} ;

struct disp_args {
	PROGINFO	*pip ;
	PTM		*omp ;
	bfile		*ofp ;
} ;

struct disp_thr {
	pthread_t	tid ;
	uint		f_active ;
} ;

struct disp_head {
	PROGINFO	*pip ;
	DISP_THR	*threads ;
	DISP_ARGS	a ;
	FSI		wq ;
	PSEM		wq_sem ;
	PTM		om ;		/* output mutex */
	volatile int	f_exit ;
	volatile int	f_done ;
	int		n ;
} ;

struct delargs {
	const char	*udname ;
	int		to_delete ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procincdirs(PROGINFO *) ;
static int	procsubprog(PROGINFO *,const char *) ;
static int	proctouchfile(PROGINFO *,const char *) ;
static int	procdelete(PROGINFO *) ;

static int	procout_begin(PROGINFO *,void *,cchar *) ;
static int	procout_end(PROGINFO *) ;

static int	procalready_begin(PROGINFO *) ;
static int	procalready_stat(PROGINFO *) ;
static int	procalready_end(PROGINFO *) ;

static int	arginfo_start(ARGINFO *,int,const char **) ;
static int	arginfo_finish(ARGINFO *) ;

#ifdef	COMMENT
static int	arginfo_arg(ARGINFO *,const char *) ;
#endif

static int	subinfo_start(SUBINFO *,PROGINFO *, ARGINFO *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_args(SUBINFO *,BITS *,DISP *) ;
static int	subinfo_argfile(SUBINFO *,DISP *) ;
static int	subinfo_procfile(SUBINFO *,DISP *,cchar *) ;

#ifdef	COMMENT
static int	subinfo_stdin(SUBINFO *,DISP *) ;
static int	ereport(PROGINFO *,cchar *,int) ;
#endif

static int	disp_start(DISP *,DISP_ARGS *) ;
static int	disp_starter(DISP *) ;
static int	disp_addwork(DISP *,cchar *,int) ;
static int	disp_finish(DISP *,int) ;
static int	disp_worker(DISP *) ;

static int	deleter(struct delargs *) ;
static int	deleter_all(struct delargs *) ;

static int	loadncpus(PROGINFO *) ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"cpp",
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
	argopt_cpp,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
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

static const char	*aknames[] = {
	"cache",
	"cpp",
	"parallel",
	"debug",
	NULL
} ;

enum aknames {
	akname_cache,
	akname_cpp,
	akname_par,
	akname_debug,
	akname_overlast
} ;

static const char	*progcpps[] = {
	"/usr/ccs/lib/cpp",
	"/usr/lib/cpp",
	"/usr/add-on/ncmp/bin/cpp",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo, *aip = &ainfo ;
	SUBINFO		si, *sip = &si ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;
	bfile		ofile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai ;
	int		rs, rs1 ;
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
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*touchfname = NULL ;
	const char	*progcpp = NULL ;
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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->f.cache = TRUE ;

	if (rs >= 0)
	    rs = ptm_create(&pip->efm,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret1 ;
	}

	rs = arginfo_start(aip,argc,argv) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badarginfo ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = dirlist_start(&pip->incs) ;
	    pip->open.incs = (rs >= 0) ;
	}

	aip->ai_max = 0 ;
	aip->ai_pos = 0 ;
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

	            aip->ai_pos = ai ;
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

/* CPP program */
	                case argopt_cpp:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            progcpp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                progcpp = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'I':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					DIRLIST	*dlp = &pip->incs ;
	                                rs = dirlist_adds(dlp,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

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
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* print something !! */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

	                    case 'r':
	                        pip->f.remove = TRUE ;
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

/* zero files (are OK) */
	                    case 'z':
	                        pip->f.zero = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.zero = (rs > 0) ;
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

	        } /* end if (digits or options) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        aip->ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    aip->ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialize */

	if ((rs >= 0) && (pip->npar == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->npar = rs ;
	}

	if ((rs >= 0) && (pip->npar == 0)) {
	    if ((cp = getourenv(envv,VARNPAR)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        pip->npar = rs ;
	    }
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* procopts */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    KEYOPT_CUR	cur ;
	    keyopt_curbegin(&akopts,&cur) ;
	    while (TRUE) {
	        rs1 = keyopt_enumkeys(&akopts,&cur,&cp) ;
	        debugprintf("main: akopt rs=%d key=%s\n",rs1,
	            ((rs1 >= 0) ? cp : "")) ;
	        if (rs1 < 0)
	            break ;
	    }
	    keyopt_curend(&akopts,&cur) ;
	}
#endif /* CF_DEBUG */

	if ((rs >= 0) && (pip->npar == 0)) {
	    rs = loadncpus(pip) ;
	    pip->npar = (rs+1) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: cache mode=%u\n" ;
	    bprintf(pip->efp,fmt,pn,pip->f.cache) ;
	    fmt = "%s: allowed parallelism=%u\n" ;
	    bprintf(pip->efp,fmt,pn,pip->npar) ;
	} /* end if */

/* check a few more things */

	aip->afname = afname ;

	if (pip->to_read <= 0)
	    pip->to_read = TO_READ ;

	if (pip->to_delete <= 0)
	    pip->to_delete = TO_DELETE ;

/* create a TMPUSER-directory (as necessary) */

	if (rs >= 0) {
	    cchar	*pn = pip->progname ;
	    char	udname[MAXPATHLEN + 1] ;
	    if ((rs = mktmpuserdir(udname,"-",pn,0775)) >= 0) {
		cchar	**vpp = &pip->udname ;
	        rs = proginfo_setentry(pip,vpp,udname,rs) ;
	    }
	}

/* find the CPP program (if we don't already have one) */

	if (rs >= 0) {
	    rs = procsubprog(pip,progcpp) ;
	    if (rs < 0) {
	        ex = EX_UNAVAILABLE ;
	        bprintf(pip->efp,"%s: CPP program unavailable\n",
	            pip->progname) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: CPP=%s\n",pip->prog_cpp) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: CPP=%s\n",
	        pip->progname,pip->prog_cpp) ;
	}

/* check for extra include-directories */

	if (rs >= 0) {
	    rs = procincdirs(pip) ;
	}

/* continue */

	if (rs >= 0) {
	if ((rs = procalready_begin(pip)) >= 0) {
	    if ((rs = procout_begin(pip,&ofile,ofname)) >= 0) {
	        if ((rs = subinfo_start(sip,pip,aip)) >= 0) {
		    DISP	disp ;
		    DISP_ARGS	wa ;

	            memset(&wa,0,sizeof(DISP_ARGS)) ;
	            wa.pip = pip ;
	            wa.ofp = pip->ofp ;

	            if ((rs = disp_start(&disp,&wa)) >= 0) {
			BITS	*bop = &pargs ;
	                if ((rs = subinfo_args(sip,bop,&disp)) >= 0) {
	                    rs = subinfo_argfile(sip,&disp) ;
			}

#if	CF_DEBUG
		    if (DEBUGLEVEL(2))
	    		debugprintf("main: mid2 rs=%d\n",rs) ;
#endif

	                if ((sip->pan == 0) && (! pip->f.zero)) {
	                    rs = SR_INVALID ;
	                    ex = EX_USAGE ;
	                    if (! pip->f.quiet) {
	                        cchar	*fmt ;
	                        fmt = "%s: no files were specified\n" ;
	                        bprintf(pip->efp,fmt,pip->progname) ;
	                    }
	                } /* end if */

	                f = (rs < 0) ;
	                rs1 = disp_finish(&disp,f) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (disp) */

#if	CF_DEBUG
		    if (DEBUGLEVEL(2))
	    		debugprintf("main: mid3 rs=%d\n",rs) ;
#endif

/* finish */

	            if (rs >= 0) {
	                cchar	*fmt ;
	                if (pip->debuglevel > 0) {
	                    fmt = "%s: safefiles processed=%u updated=%u\n" ;
	                    bprintf(pip->efp,fmt,
	                        pip->progname,
	                        pip->c_processed,pip->c_updated) ;
	                }
	                if (pip->verboselevel > 0) {
	                    fmt = "safefiles processed=%u updated=%u\n" ;
	                    bprintf(pip->ofp,fmt,
	                        pip->c_processed,pip->c_updated) ;
	                }
	            } /* end if */

/* cache statistics (mostly for fun) */

	            procalready_stat(pip) ;

/* finish up */

#if	CF_DELETER
	            if (rs >= 0)
	                rs = procdelete(pip) ;
#endif

	            rs1 = subinfo_finish(sip) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	        rs1 = procout_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: output unavailable (%d)\n" ;
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if */
	    rs1 = procalready_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procalready) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: done rs=%d\n",rs) ;
	    debugprintf("main: processed=%d updated=%d\n",
	        pip->c_processed,pip->c_updated) ;
	}
#endif /* CF_DEBUG */

/* done */
	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: good rs=%d processed=%d updated=%d\n",
	            rs,pip->c_processed,pip->c_updated) ;
#endif

	    if ((touchfname != NULL) && (touchfname[0] != '\0')) {
	        rs = proctouchfile(pip,touchfname) ;
	        if (rs == SR_NOENT) ex = EX_CANTCREAT ;
	    } /* end if (touch-file) */

	} /* end if */

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
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
	} /* end if */

/* we are out of here */
retearly:
	if ((pip->debuglevel > 0) || pip->f.optdebug) {
	    cchar	*pn = pip->progname ;
	    if (pip->f.optdebug) {
	        proginfo_pwd(pip) ;
	        bprintf(pip->efp,"%s: dir=%s\n",pn,pip->pwd) ;
	    }
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",pn,ex,rs) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(1))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.incs) {
	    pip->open.incs = FALSE ;
	    dirlist_finish(&pip->incs) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	arginfo_finish(aip) ;

badarginfo:
	ptm_destroy(&pip->efm) ;

ret1:
	proginfo_finish(pip) ;

badprogstart:

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

/* bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


int progeprintf(PROGINFO *pip,cchar *fmt,...)
{
	int		rs = SR_OK ;

	if (pip->debuglevel > 0) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = ptm_lock(&pip->efm)) >= 0) {
	        rs = bvprintf(pip->efp,fmt,ap) ;
	        ptm_unlock(&pip->efm) ;
	    } /* end if (mutex) */
	    va_end(ap) ;
	} /* end if */

	return rs ;
}
/* end subroutine (progeprintf) */


int progout_printf(PROGINFO *pip,cchar *fmt,...)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if ((pip->ofp != NULL) && (pip->verboselevel > 0)) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = ptm_lock(&pip->ofm)) >= 0) {
	        rs = bvprintf(pip->ofp,fmt,ap) ;
	        len = rs ;
	        ptm_unlock(&pip->ofm) ;
	    } /* end if (mutex) */
	    va_end(ap) ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progout_printf) */


int progalready_lookup(PROGINFO *pip,cchar *cp,int cl,time_t *rtp)
{
	int		rs = SR_OK ;
	if (pip->open.cache) {
	    rs = cachetime_lookup(&pip->mtdb,cp,cl,rtp) ;
	} /* end if (cache) */
	return rs ;
}
/* end subroutine (progalready_lookup) */


/* local subroutines */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_NOTOPEN ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	if (pip->efp != NULL) {

	    fmt = "%s: USAGE> %s [<objfile(s)> ...] [[-I <incdir(s)>] ...]\n" ;
	    if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-t <target>] [-z]\n" ;
	    if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-V]\n" ;
	    if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {
	        int	ki ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&cur,&kp)) >= 0) {

	            if ((ki = matostr(aknames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (ki) {
	                case akname_cache:
	                    c += 1 ;
	                    if (! pip->final.cache) {
	                        pip->have.cache = TRUE ;
	                        pip->f.cache = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.cache = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akname_cpp:
	                    c += 1 ;
	                    if ((vl > 0) && (pip->prog_cpp == NULL)) {
	                        const char	**vpp = &pip->prog_cpp ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case akname_par:
	                    c += 1 ;
	                    if (! pip->final.optpar) {
	                        pip->have.optpar = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->npar = rs ;
	                        }
	                    }
	                    break ;
	                case akname_debug:
	                    c += 1 ;
	                    if (! pip->final.optdebug) {
	                        pip->have.optdebug = TRUE ;
	                        pip->f.optdebug = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optdebug = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while (procopts) */

	        keyopt_curend(kop,&cur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procopts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procincdirs(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;
	if ((cp = getenv(VARINCDIRS)) != NULL) {
	    rs = dirlist_adds(&pip->incs,cp,-1) ;
	    c = rs ;
	} /* end if */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procincdirs) */


static int procsubprog(PROGINFO *pip,cchar *progcpp)
{
	int		rs = SR_OK ;
	const char	*cp ;
	const char	**vpp = &pip->prog_cpp ;

	if (progcpp != NULL) {
	    rs = proginfo_setentry(pip,vpp,progcpp,-1) ;
	}

	if ((rs >= 0) && (pip->prog_cpp == NULL)) {
	    if ((cp = getenv(VARCPP)) != NULL) {
	        rs = proginfo_setentry(pip,vpp,cp,-1) ;
	    }
	}

	if ((rs >= 0) && (pip->prog_cpp == NULL)) {
	    int		sl = -1 ;
	    const char	*sp = CPPFNAME ;
	    char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: need to find CPP\n") ;
#endif

	    if ((rs = findfilepath(NULL,tmpfname,sp,X_OK)) >= 0) {
	        if (rs > 0) {
	            sl = rs ;
	            sp = tmpfname ;
	        } else {
	            sl = -1 ;
		}
	    } else if (isNotPresent(rs)) {
	        int	i ;

	        for (i = 0 ; progcpps[i] != NULL ; i += 1) {
	            sp = progcpps[i] ;
	            rs = perm(sp,-1,-1,NULL,X_OK) ;
	            if (rs >= 0) break ;
	        } /* end for */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: perm() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = proginfo_setentry(pip,vpp,sp,sl) ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (procsubprog) */


static int procout_begin(PROGINFO *pip,void *ofp,const char *ofn)
{
	int		rs = SR_OK ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if (pip->f.print || (pip->verboselevel > 0)) {
	    if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	        if ((rs = ptm_create(&pip->ofm,NULL)) >= 0) {
	            pip->ofp = ofp ;
	        }
	        if (rs < 0) {
	            bclose(pip->ofp) ;
	        }
	    } /* end if (bopen) */
	} /* end if */

	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.print || (pip->verboselevel > 0)) {
	    rs1 = ptm_destroy(&pip->ofm) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = bclose(pip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->ofp = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procout_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procout_end) */


static int procalready_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->f.cache) {
	    if ((rs = cachetime_start(&pip->mtdb)) >= 0) {
	        pip->open.cache = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (procalready_begin) */


static int procalready_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.cache) {
	    pip->open.cache = FALSE ;
	    rs1 = cachetime_finish(&pip->mtdb) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procalready_end) */


static int procalready_stat(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if ((pip->verboselevel > 0) || (pip->debuglevel > 0)) {
	    if (pip->open.cache) {
	        CACHETIME_STATS	stat ;

	        if ((rs = cachetime_stats(&pip->mtdb,&stat)) >= 0) {
	 	    cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            if (pip->verboselevel >= 3) {
	                fmt = "cache requests=%u hits=%u misses=%u\n" ;
	                bprintf(pip->ofp,fmt,stat.req,stat.hit,stat.miss) ;
	            }
	            if (pip->debuglevel > 0) {
			fmt = "%s: cache requests=%u hits=%u misses=%u\n" ;
	                bprintf(pip->efp,fmt,pn,stat.req,stat.hit,stat.miss) ;
	            }
	        } /* end if (successful) */

	    } /* end if (cache) */
	} /* end if (verbose or debug) */
	return rs ;
}
/* end subroutine (procalready_stat) */


static int proctouchfile(PROGINFO *pip,cchar touchfname[])
{
	bfile		touchfile ;
	int		rs = SR_OK ;
	int		f_write = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;
	if (touchfname == NULL) return SR_FAULT ;

	if (touchfname[0] == '\0') return SR_INVALID ;

	pip->daytime = time(NULL) ;

	rs = bopen(&touchfile,touchfname,"w",0666) ;

	if (isNotPresent(rs)) {
	    f_write = TRUE ;
	    rs = bopen(&touchfile,touchfname,"wct",0666) ;
	}

	if (rs >= 0) {

#if	CF_ALWAYS
	    bprintf(&touchfile,"%s\n",
	        timestr_logz(pip->daytime,timebuf)) ;
#else
	    if (f_write || (pip->c_updated > 0))
	        bprintf(&touchfile,"%s\n",
	            timestr_logz(pip->daytime,timebuf)) ;
#endif /* CF_ALWAYS */

	    bclose(&touchfile) ;
	} /* end if (file) */

	return (rs >= 0) ? f_write : rs ;
}
/* end subroutine (proctouchfile) */


static int arginfo_start(ARGINFO *aip,int argc,cchar *argv[])
{
	int		rs = SR_OK ;

	memset(aip,0,sizeof(ARGINFO)) ;
	aip->argc = argc ;
	aip->argv = argv ;

	return rs ;
}
/* end subroutine (arginfo_start) */


#ifdef	COMMENT
static int arginfo_arg(aip,ap)
ARGINFO		*aip ;
const char	*ap ;
{
	int	rs = SR_OK ;
	if (aip == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (arginfo_arg) */
#endif /* COMMENT */


static int arginfo_finish(ARGINFO *aip)
{

	if (aip == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (arginfo_finish) */


static int subinfo_start(SUBINFO *sip,PROGINFO *pip,ARGINFO *aip)
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pip = pip ;
	sip->aip = aip ;

	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = sip->pan ;

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_args(SUBINFO *sip,BITS *bop,DISP *dop)
{
	PROGINFO	*pip = sip->pip ;
	ARGINFO		*aip = sip->aip ;
	int		rs = SR_OK ;
	int		ai ;
	int		f ;
	const char	*cp ;

	if (pip == NULL) return SR_FAULT ;

	for (ai = 1 ; ai < aip->argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (f) {
	        cp = aip->argv[ai] ;
		if (cp[0] != '\0') {
	            sip->pan += 1 ;
	            rs = subinfo_procfile(sip,dop,cp) ;
		}
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/subinfo_args: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_args) */


static int subinfo_argfile(SUBINFO *sip,DISP *dop)
{
	PROGINFO	*pip = sip->pip ;
	ARGINFO		*aip = sip->aip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((aip->afname != NULL) && (aip->afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;
	    const char	*afname = aip->afname ;

	    if (afname[0] == '-') afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        int		cl ;
	        const char	*cp ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
			    lbuf[(cp+cl)-lbuf] = '\0' ;
	            	    sip->pan += 1 ;
	            	    rs = subinfo_procfile(sip,dop,cp) ;
			}
		    }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else if (! pip->f.quiet) {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: unaccessible (%d) afile=%s\n" ;
	        bprintf(pip->efp,fmt,pn,rs,aip->afname) ;
	    } /* end if */

	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/subinfo_argfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_argfile) */


#ifdef	COMMENT
static int subinfo_stdin(SUBINFO *sip,DISP *dop)
{
	int		rs = SR_OK ;

	if (sip->pan == 0) {
	    const char	*cp = "-" ;
	    sip->pan += 1 ;
	    rs = subinfo_procfile(sip,dop,cp) ;
	}

	return rs ;
}
/* end subroutine (subinfo_stdin) */
#endif /* COMMENT */


static int subinfo_procfile(SUBINFO *sip,DISP *dop,cchar *fname)
{
	PROGINFO	*pip = sip->pip ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;

	pip->c_processed += 1 ;
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: processing file=%s\n",
	        pip->progname,fname) ;
	}

	if (fname[0] != '-') {
	    struct ustat	sb ;

	    rs1 = u_stat(fname,&sb) ;
	    if (rs1 >= 0)
	        rs1 = sperm(&sip->id,&sb,R_OK) ;

#ifdef	COMMENT
	    if (rs1 < 0)
	        ereport(pip,fname,rs1) ;
#endif

	    if (rs1 >= 0) {
	        rs = disp_addwork(dop,fname,-1) ;
	    }

	} /* end if (flie check) */

	return rs ;
}
/* end subroutine (subinfo_procfile) */


static int disp_start(DISP *dop,DISP_ARGS *wap)
{
	PROGINFO	*pip ;
	int		rs ;

	if (dop == NULL) return SR_FAULT ;
	if (wap == NULL) return SR_FAULT ;

	pip = wap->pip ;

	memset(dop,0,sizeof(DISP)) ;
	dop->pip = pip ;
	dop->a = *wap ;
	dop->n = pip->npar ;

	if ((rs = fsi_start(&dop->wq)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: mid2 rs=%d\n",rs) ;
#endif
	    if ((rs = psem_create(&dop->wq_sem,FALSE,0)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: mid3 rs=%d\n",rs) ;
#endif
		if ((rs = ptm_create(&dop->om,NULL)) >= 0) {
		    const int	size = (pip->npar * sizeof(DISP_THR)) ;
		    void	*p ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: mid4 rs=%d sz=%u\n",rs,size) ;
#endif
		    if ((rs = uc_malloc(size,&p)) >= 0) {
		        dop->threads = p ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: mid5 rs=%d\n",rs) ;
#endif
			rs = disp_starter(dop) ;
		        if (rs < 0) {
			    uc_free(dop->threads) ;
			    dop->threads = NULL ;
		        }
		    } /* end if (m-a) */
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: malloc-out rs=%d\n",rs) ;
#endif
		    if (rs < 0)
		        ptm_destroy(&dop->om) ;
	        }
		if (rs < 0)
		    psem_destroy(&dop->wq_sem) ;
	    } /* end if (psem_create) */
	    if (rs < 0)
		fsi_finish(&dop->wq) ;
	} /* end if (fsi) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_start) */


static int disp_starter(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		i ;

	if (pip == NULL) return SR_FAULT ;

	for (i = 0 ; (rs >= 0) && (i < dop->n) ; i += 1) {
	    uptsub_t	fn = (uptsub_t) disp_worker ;
	    rs = uptcreate(&tid,NULL,fn,dop) ;
	    dop->threads[i].tid = tid ;
	    dop->threads[i].f_active = TRUE ;
	}

	if (rs < 0) {
	    int		n = i ;
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < n ; i += 1) {
	        psem_post(&dop->wq_sem) ;
	    }
	    for (i = 0 ; i < n ; i += 1) {
	        tid = dop->threads[i].tid ;
	        uptjoin(tid,NULL) ;
		dop->threads[i].f_active = FALSE ;
	    }
	} /* end if (failure) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_starter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_starter) */


static int disp_finish(DISP *dop,int f_abort)
{
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dop == NULL) return SR_FAULT ;

	dop->f_done = TRUE ;
	if (f_abort)
	    dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->n ; i += 1) {
	    psem_post(&dop->wq_sem) ;
	}

	for (i = 0 ; i < dop->n ; i += 1) {
	    if (dop->threads[i].f_active) {
	        dop->threads[i].f_active = FALSE ;
	        tid = dop->threads[i].tid ;
	        rs1 = uptjoin(tid,NULL) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	if (dop->threads != NULL) {
	    rs1 = uc_free(dop->threads) ;
	    if (rs >= 0) rs = rs1 ;
	    dop->threads = NULL ;
	}

	rs1 = ptm_destroy(&dop->om) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->wq_sem) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fsi_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(DISP *dop,cchar *tagbuf,int taglen)
{
	int		rs ;

	if ((rs = fsi_add(&dop->wq,tagbuf,taglen)) >= 0) {
	    rs = psem_post(&dop->wq_sem) ;
	}

	return rs ;
}
/* end subroutine (disp_addwork) */


static int disp_worker(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	pthread_t	tid ;
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		c = 0 ;
	char		rbuf[MAXPATHLEN + 1] ;

	uptself(&tid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkkey/worker: ent tid=%u\n",tid) ;
#endif

	while (rs >= 0) {

	    while ((rs = psem_wait(&dop->wq_sem)) < 0) {
	        if ((rs != SR_AGAIN) && (rs != SR_INTR))
	            break ;
	    } /* end while */

	    if (rs < 0) break ;
	    if (dop->f_exit) break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("mkkey/worker: tid=%u wakeup\n",tid) ;
#endif

	    if ((rs = fsi_remove(&dop->wq,rbuf,rlen)) >= 0) {
	        rs = progfile(pip,rbuf) ;
	        if (rs > 0) c += 1 ;
	    } else if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
		if (dop->f_done) break ;
	    }

	} /* end while (server loop) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkkey/worker: tid=%u ret rs=%d c=%u\n",tid,rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_worker) */


#ifdef	COMMENT

static int ereport(PROGINFO *pip,cchar *fname,int frs)
{
	int		rs = SR_OK ;

	if (! pip->f.quiet) {
	    bprintf(pip->efp,"%s: file-processing error (%d)\n",
	        pip->progname,frs) ;
	    rs = bprintf(pip->efp,"%s: file=%s\n",
	        pip->progname,fname) ;
	}

	return rs ;
}
/* end subroutine (ereport) */

#endif /* COMMENT */


#if	CF_DELETER

static int procdelete(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->udname != NULL) {
	struct delargs	da ;

	memset(&da,0,sizeof(struct delargs)) ;
	da.udname = pip->udname ;
	da.to_delete = pip->to_delete ;

	if ((rs = uc_fork()) == 0) { /* child */
	    int	ex ;
	    int	i ;

#if	CF_DEBUGS
#else
	    for (i = 0 ; i < NOFILE ; i += 1)
	        u_close(i) ;
#endif

	    u_setsid() ;

	    rs = deleter(&da) ;

	    ex = (rs >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex) ;

	} /* end if (child) */

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (procdelete) */


static int deleter(struct delargs *dap)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("main/deleter: entered\n") ;
#endif

	rs = deleter_all(dap) ;

	return rs ;
}
/* end subroutine (deleter) */


static int deleter_all(struct delargs *dap)
{
	struct ustat	sb ;
	FSDIR		dir ;
	FSDIR_ENT	de ;
	vecstr		files ;
	time_t		daytime = time(NULL) ;
	const int	to = dap->to_delete ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl ;
	int		fl ;
	int		i ;
	const char	*udname = dap->udname ;
	const char	*fp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (udname == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	if (udname[0] == '\0') {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	if ((rs = vecstr_start(&files,0,0)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main/deleter_all: udname=%s\n",udname) ;
#endif

	    if ((rs = fsdir_open(&dir,udname)) >= 0) {

	        while ((nl = fsdir_read(&dir,&de)) > 0) {
	            if (de.name[0] == '.') continue ;

	            rs1 = mkpath2w(tmpfname,udname,de.name,nl) ;
	            fl = rs1 ;
	            if (rs1 >= 0)
	                rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	            debugprintf("main/deleter_all: name=%s rs1=%d\n",
	                de.name,rs1) ;
#endif

	            if (rs1 >= 0) {
	                if ((daytime - sb.st_mtime) >= to) {

#if	CF_DEBUGS
	                    debugprintf("main/deleter_all: sched_del name=%s\n",
	                        de.name) ;
#endif

	                    rs = vecstr_add(&files,tmpfname,fl) ;

	                }
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while */

	        fsdir_close(&dir) ;
	    } /* end if */

	    if (rs >= 0) {
	        for (i = 0 ; vecstr_get(&files,i,&fp) >= 0 ; i += 1) {
	            if (fp != NULL) {
	            if (fp[0] != '\0') {

#if	CF_DEBUGS
	                debugprintf("main/deleter_all: unlink name=%s\n",fp) ;
#endif

	                u_unlink(fp) ;
	            }
		    }
	        } /* end for */
	    } /* end if */

	    vecstr_finish(&files) ;
	} /* end if (files) */

ret0:
	return rs ;
}
/* end subroutine (deleter_all) */

#endif /* CF_DELETER */


static int loadncpus(PROGINFO *pip)
{
	int		rs = getnprocessors(pip->envv,0) ;
	if (pip == NULL) return SR_FAULT ;
	pip->ncpu = rs ;
	return rs ;
}
/* end subroutine (loadncpus) */


