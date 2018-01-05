/* b_pathclean */

/* front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_VECSTRDELALL	1		/* use 'vecstr_delall(3dam)' */
#define	CF_DYNAMICALLOC	0		/* dynamic stack allocation */
#define	CF_LOCLOADPATHS	0		/* "locinfo_loadpaths()| */


/* revision history:

	= 2004-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ pathclean path(s) [-j]

	Notes:

	This is not entirely pretty everywhere.  But it works, and is certainly
	well enough for the present purposes.


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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<storebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_pathclean.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO_MAGIC	0x99224571
#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	MAXPATHSLEN	(40 * MAXPATHLEN)


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

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
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		join:1 ;	/* join paths */
	uint		exists:1 ;	/* directory must exist */
} ;

struct locinfo {
	uint		magic ;
	PROGINFO	*pip ;
	VECSTR		paths ;
	LOCINFO_FL	f ;
	int		nmax ;
	int		c ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procname(PROGINFO *,SHIO *,const char *,int) ;
static int	procpath(PROGINFO *,const char *,int) ;
static int	procjoin(PROGINFO *,SHIO *) ;
static int	printit(PROGINFO *,SHIO *,const char *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_nmax(LOCINFO *,int) ;
static int	locinfo_loadpath(LOCINFO *,const char *,int) ;
static int	locinfo_mkjoin(LOCINFO *,char *,int) ;
static int	locinfo_finishpaths(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCLOADPATHS
static int	locinfo_loadpaths(LOCINFO *,const char *,int) ;
#endif /* CF_LOCLOADPATHS */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"DEBUG",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"xu",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_debug,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_xu,
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
	{ SR_NOMEM, EX_OSERR },
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_ALREADY, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"join",
	"exists",
	NULL
} ;

enum progopts {
	progopt_join,
	progopt_exists,
	progopt_overlast
} ;


/* exported subroutines */


int b_pathclean(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_pathclean) */


int p_pathclean(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_pathclean) */


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
	int		argvalue = -1 ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optplus, f_optminus, f_optequal ;
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
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_pathclean: starting DFD=%d\n",rs) ;
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

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* gather the arguments */

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
	    if ((argl > 1) && (f_optplus || f_optminus)) {
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

/* do we have a keyword or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
				    pip->debuglevel = rs ;
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

/* argument files */
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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'j':
	                        lip->f.join = TRUE ;
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

/* verbosity level */
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
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathclean: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: verboselevel=%d\n",
	        pip->progname,pip->verboselevel) ;
	}

	if (f_version && (pip->efp != NULL)) {
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

/* option parsing */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: unrecognized option\n",
	        pip->progname) ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs >= 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&argvalue) ;
	}

/* load any maximum print-count */

	if (rs >= 0) {
	    rs = locinfo_nmax(&li,argvalue) ; 
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
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
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
	locinfo_finish(&li) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_pathclean: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	    ex = EX_USAGE ;
	    if (pip->efp != NULL)
	        shio_printf(pip->efp,
	            "%s: invalid argument specified (%d)\n",
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

	fmt = "%s: USAGE> %s [<path(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-j] [-o <opts>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	} /* end if (error-output enabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;
	int		size ;
	int		opts ;

	size = sizeof(LOCINFO) ;
	memset(lip,0,size) ;
	lip->pip = pip ;
	lip->nmax = -1 ;

	opts = VECSTR_OORDERED | VECSTR_OSTSIZE | VECSTR_OREUSE ;
	if ((rs = vecstr_start(&lip->paths,10,opts)) >= 0) {
	    lip->magic = LOCINFO_MAGIC ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&lip->paths) ;
	if (rs >= 0) rs = rs1 ;

	lip->magic = 0 ;
	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_nmax(LOCINFO *lip,int nmax)
{

	lip->nmax = nmax ;
	return SR_OK ;
}
/* end subroutine (locinfo_nmax) */


#if	CF_LOCLOADPATHS
static int locinfo_loadpaths(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*tp, *cp ;

	while ((tp = strnpbrk(sp,sl," \r\n\t,")) != NULL) {

	    cp = sp ;
	    cl = (tp - sp) ;
	    if (cl > 0) {
	        rs = locinfo_loadpath(lip,cp,sl) ;
	    }

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	        if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {

	    rs = locinfo_loadpath(lip,sp,sl) ;

	}

	return rs ;
}
/* end subroutine (locinfo_loadpaths) */
#endif /* CF_LOCLOADPATHS */


static int locinfo_loadpath(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs ;
	int		c = 0 ;

	rs = vecstr_adduniq(&lip->paths,sp,sl) ;
	if (rs < 0) c = 1 ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadpath) */


static int locinfo_mkjoin(LOCINFO *lip,char pbuf[],int plen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		bl = 0 ;
	int		c = 0 ;
	int		f_semi = FALSE ;
	const char	*sp ;

	for (i = 0 ; vecstr_get(&lip->paths,i,&sp) >= 0 ; i += 1) {
	    if (sp == NULL) continue ;

	    if (sp[0] != ';') {

		rs1 = SR_OK ;
		if (lip->f.exists) {
		    struct ustat	sb ;
		    rs1 = u_stat(sp,&sb) ;
		    if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode))) 
			rs1 = SR_NOTDIR ;
		}

		if (rs1 >= 0) {

	            if (c++ > 0) {
		        int	ch ;
		        if (f_semi) {
			    f_semi = FALSE ;
	                    ch = ';' ;
		        } else
	                    ch = ':' ;
		        rs = storebuf_char(pbuf,plen,bl,ch) ;
		        bl += rs ;
		    }

		    if (rs >= 0) {
		        rs = storebuf_strw(pbuf,plen,bl,sp,-1) ;
		        bl += rs ;
		    }

		} /* end if */

	    } else
		f_semi = TRUE ;

	    if (rs < 0) break ;
	} /* end for (paths) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (locinfo_mkjoin) */


static int locinfo_finishpaths(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c ;

	rs1 = vecstr_count(&lip->paths) ;
	c = rs1 ;

/* delete all of the strings in the vector-string object */

#if	CF_VECSTRDELALL /* either the easy way -- or harder! */
	{
	    rs1 = vecstr_delall(&lip->paths) ;
	    if (rs >= 0) rs = rs1 ;
	}
#else /* CF_VECSTRDELALL */
	{
	    int		i ;
	    const char	*sp ;
	    for (i = 0 ; vecstr_get(&lip->paths,i,&sp) >= 0 ; i += 1) {
	        if (sp == NULL) continue ;
	        rs1 = vecstr_del(&lip->paths,i--) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end for */
	} /* end block */
#endif /* CF_VECSTRDELALL */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_finishpaths) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
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

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	    	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	        	switch (oi) {

	        case progopt_join:
	            c += 1 ;
	            lip->f.join = TRUE ;
	            if (vl > 0) {
			rs = optbool(vp,vl) ;
	                lip->f.join = (rs > 0) ;
		    }
	            break ;

	        case progopt_exists:
	            c += 1 ;
	            lip->f.exists = TRUE ;
	            if (vl > 0) {
			rs = optbool(vp,vl) ;
	                lip->f.exists = (rs > 0) ;
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


static int procargs(pip,aip,bop,ofname,afname)
PROGINFO	*pip ;
ARGINFO	*aip ;
BITS		*bop ;
const char	*ofname ;
const char	*afname ;
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"r",0666)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    int		pan = 0 ;
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
	                pan += 1 ;
	                rs = procname(pip,ofp,cp,-1) ;
	                c += rs ;
		    }

	            if (rs < 0) {
	                shio_printf(pip->efp,
	                        "%s: error processing path (%d)\n",
	                        pip->progname,rs) ;
	                shio_printf(pip->efp,
	            	    "%s: path=%t\n",
	            	    pip->progname,cp,-1) ;
	            }

		    if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afname,"-") == 0)
	            afname = STDINFNAME ;

	        if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
		    const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;
    
	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	            cp = lbuf ;
		    cl = len ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procname(pip,ofp,cp,cl) ;
	                c += rs ;
		    }

	            if (rs < 0) {
	                    shio_printf(pip->efp,
	                        "%s: error processing path (%d)\n",
	                        pip->progname,rs) ;
	                shio_printf(pip->efp,
	                    "%s: path=%t\n",
	                    pip->progname,cp,cl) ;
	            }

		    if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	            shio_printf(pip->efp,
	                "%s: inaccessible argument-list (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	    } /* end if */

	} /* end if (processing argument-list file) */

	if ((rs >= 0) && lip->f.join) {

	    rs = procjoin(pip,ofp) ;

	} /* end if */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    shio_printf(pip->efp,"%s: inaccessible output (%d)\n",
		pip->progname,rs) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


/* process a name */
static int procname(PROGINFO *pip,SHIO *ofp,cchar sp[],int sl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*tp, *cp ;

	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathclean/procname: name=>%t<\n",sp,sl) ;
#endif

	while ((tp = strnpbrk(sp,sl,":;")) != NULL) {

	    cp = sp ;
	    cl = (tp - sp) ;
	    c += 1 ;
	    rs = procpath(pip,cp,cl) ;

	    if ((rs >= 0) && (tp[0] == ';'))
	        rs = locinfo_loadpath(lip,";",1) ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    c += 1 ;
	    rs = procpath(pip,sp,sl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathclean/procname: loaded c=%u\n",c) ;
#endif

	if ((rs >= 0) && (c == 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathclean/procname: empty procpath\n") ;
#endif

	    rs = procpath(pip,sp,0) ;

	} /* end if (assume an empty path) */

/* process */

	if ((rs >= 0) && (! lip->f.join)) {

	    rs = procjoin(pip,ofp) ;

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


static int procpath(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		plen = 0 ;
	char		pathbuf[MAXPATHLEN + 1] ;

	if (sp == NULL)
	    return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_pathclean/procpath: path=%t\n",sp,sl) ;
#endif

	pathbuf[0] = '\0' ;
	if (sl > 0) {
	    rs = pathclean(pathbuf,sp,sl) ;
	    plen = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_pathclean/procpath: pathclean() rs=%d\n",rs) ;
	    debugprintf("b_pathclean/procpath: p=%t\n",pathbuf,plen) ;
	}
#endif
	}

	if (rs >= 0)
	    rs = locinfo_loadpath(lip,pathbuf,plen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_pathclean/procpath: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? plen : rs ;
}
/* end subroutine (procpath) */


static int procjoin(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs ;
	int		c = 0 ;

	if ((rs = vecstr_strsize(&lip->paths)) >= 0) {
	    int	plen = rs ;
	    if (plen <= MAXPATHSLEN) {

#if	CF_DYNAMICALLOC
	    {
		char	pbuf[plen+1] ;

	        if ((rs = locinfo_mkjoin(lip,pbuf,plen)) >= 0)
	            rs = printit(pip,ofp,pbuf) ;

	    } /* end if (memory allocation) */
#else /* CF_DYNAMICALLOC */
	    {
	        char	*pbuf ;
	        if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {

	            if ((rs = locinfo_mkjoin(lip,pbuf,plen)) >= 0)
	                rs = printit(pip,ofp,pbuf) ;

	            uc_free(pbuf) ;
	        } /* end if (memory allocation) */
	    }
#endif /* CF_DYNAMICALLOC */

	        c = locinfo_finishpaths(lip) ;
	        if (rs >= 0) rs = c ;
	    } else
	        rs = SR_TOOBIG ;
	} /* end if (size) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathclean/procjoin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procjoin) */


/* print it out */
static int printit(PROGINFO *pip,SHIO *ofp,cchar fname[])
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("b_pathclean/printit: fname=%s\n",fname) ;
	    debugprintf("b_pathclean/printit: nmax=%d c=%d\n",
		lip->nmax,lip->c) ;
	}
#endif

	if (pip->verboselevel > 0) {
	    if ((lip->nmax < 0) || (lip->c < lip->nmax)) {
	        rs = shio_print(ofp,fname,-1) ;
	        wlen += rs ;
	    }
	    lip->c += 1 ;
	} /* end if (verbose) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_pathclean/printit: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printit) */


