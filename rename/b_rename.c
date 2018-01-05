/* b_rename */

/* SHELL built-in to rename a file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if any) difficulty.

	Synopsis:

	$ rename { <basename> | -b <basename> } <file(s)> [-s <suffix>] 


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
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_rename.h"
#include	"defs.h"
#include	"numincr.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#define	DBUFLEN		18		/* maximum precision */
#define	DEFPREC		4		/* default precision */

#undef	SUFLEN
#define	SUFLEN		20

#define	ZOMBIENAME	struct zombiename

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	ARGVALS		struct argvals


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdecpi(char *,int,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	ipow(int,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	hasuc(const char *,int) ;
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
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

enum sufs {
	suf_null,
	suf_empty,
	suf_plus,
	suf_minus,
	suf_new,
	suf_overlast
} ;

struct locinfo_flags {
	uint		sortkey:1 ;
	uint		alpha:1 ;
	uint		findsuffix:1 ;
} ;

struct locinfo {
	const char	*suffix ;
	const char	*startnum ;
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	NUMINCR		incr ;
	enum sufs	suftype ;
	int		sortkey ;
} ;

/* argument values */
struct argvals {
	const char	*basename ;
	const char	*suffix ;
	const char	*countstr ;
	int		countlen ;
	int		prec ;
	int		incr ;
} ;

struct zombiename {
	VECSTR		*nnp ;
	int		n ;
	char		prefix[MAXNAMELEN + 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,vecstr *,cchar *) ;
static int	entername(PROGINFO *,vecstr *,const char *) ;
static int	procnewnames(PROGINFO *,ARGVALS *,vecstr *) ;
static int	procrename(PROGINFO *,char *,vecstr *,vecstr *) ;
static int	parsesort(PROGINFO *,const char *,int) ;
static int	makefname(PROGINFO *,ARGVALS *,cchar *,char *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_setsuf(LOCINFO *,cchar *) ;
static int	locinfo_findsuf(LOCINFO *,cchar *,int) ;
static int	locinfo_setcount(LOCINFO *,cchar *,int) ;
static int	locinfo_setprec(LOCINFO *,int) ;
static int	locinfo_setsort(LOCINFO *,cchar *,int) ;
static int	locinfo_incr(LOCINFO *,int) ;
static int	locinfo_cvtstr(LOCINFO *,char *,int,int) ;
static int	locinfo_finish(LOCINFO *) ;

static int	zombiename_start(ZOMBIENAME *,VECSTR *) ;
static int	zombiename_rename(ZOMBIENAME *,const char *,char *) ;
static int	zombiename_finish(ZOMBIENAME *) ;

static int	mkfnamenum(char *,const char *,int,int) ;
static int	argvals_check(ARGVALS *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
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

static const char	*akonames[] = {
	"sort",
	NULL
} ;

enum akonames {
	akoname_sort,
	akoname_overlast
} ;

static const char	*sortkeys[] = {
	"default",
	"name",
	"mtime",
	"size",
	"inode",
	NULL
} ;

enum sortkeys {
	sortkey_default,
	sortkey_name,
	sortkey_mtime,
	sortkey_size,
	sortkey_inode,
	sortkey_overlast
} ;


/* exported subroutines */


int b_rename(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_rename) */


int p_rename(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_rename) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	ARGVALS		avs ;		/* argument values */
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

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_rename: starting DFD=%d\n",rs) ;
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

	memset(&avs,0,sizeof(ARGVALS)) ;

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

/* base filename */
	                    case 'b':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            avs.basename = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* starting count */
	                    case 'c':
			    case 'n':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    avs.countstr = argp ;
				    avs.countlen = argl ;
				}
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* increment */
	                    case 'i':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            avs.incr = rs ;
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

/* count precision */
	                    case 'p':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            avs.prec = rs ;
				}
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* filename suffix */
	                    case 's':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            avs.suffix = argp ;
				} else
	                            rs = SR_INVALID ;
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

	        } /* end if (dbuf as argument or not) */

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
	    debugprintf("b_rename: debuglevel=%u\n",pip->debuglevel) ;
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

/* load up the environment options */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (rs >= 0) {
	    if ((rs = argvals_check(&avs)) >= 0) {
	        if ((rs = locinfo_setsuf(lip,avs.suffix)) >= 0) {
		    if (avs.countstr != NULL) {
	    		rs = locinfo_setcount(lip,avs.countstr,avs.countlen) ;
		    }
		    if (rs >= 0) {
	    		rs = locinfo_setprec(lip,avs.prec) ;
		    }
		}
	    }
	}

/* process */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    vecstr	names ;
	    const int	opts = VECSTR_OSTATIONARY | VECSTR_OREUSE ;
	    if ((rs = vecstr_start(&names,20,opts)) >= 0) {
	        if ((rs = procargs(pip,&ainfo,&pargs,&names,afname)) > 0) {
	            if ((rs = vecstr_count(&names)) > 0) {
		        rs = procnewnames(pip,&avs,&names) ;
		    } /* end if (nonzero) */
	        } else if (rs >= 0) {
	    	    cchar	*pn = pip->progname ;
	            rs = SR_INVALID ;
	            shio_printf(pip->efp,"%s: no files specified\n",pn) ;
	        }
	        rs1 = vecstr_finish(&names) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        ex = EX_OSERR ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        shio_printf(pip->efp,
	            "%s: could not perform function (%d)\n",
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
	    debugprintf("b_rename: final mallout=%u\n",(mo-mo_start)) ;
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

	fmt = "%s: USAGE> %s [-b <base>] [-s <suffix>] [<files(s)> ...]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p <precision>] [-c <startcount>] [-i <increment>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
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
	        	case akoname_sort:
		    	    if (! lip->final.sortkey) {
	                	lip->have.sortkey = TRUE ;
	                	lip->final.sortkey = TRUE ;
	                	lip->f.sortkey = FALSE ;
	                	if (vl > 0) {
			    	    rs = parsesort(pip,vp,vl) ;
	                    	    lip->f.sortkey = TRUE ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,vecstr *nlp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
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
	                rs = entername(pip,nlp,cp) ;
		    }
		}

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

		    cp = lbuf ;
	            if (cp[0] != '\0') {
		        pan += 1 ;
	                rs = entername(pip,nlp,cp) ;
		    }

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

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int entername(PROGINFO *pip,vecstr *nlp,cchar *name)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl = strlen(name) ;
	int		c = 0 ;
	const char	*cp = name ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_rename/entername: name=%s\n",name) ;
#endif

	while ((cl > 0) && (cp[cl-1] == '/')) {
	    cl -= 1 ;
	}

	if (cl > 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if (lip->f.findsuffix) rs = locinfo_findsuf(lip,cp,cl) ;
	    if (rs >= 0) {
	        if ((rs = pathclean(tbuf,cp,cl)) >= 0) {
	            rs = vecstr_adduniq(nlp,tbuf,-1) ;
		    if (rs < INT_MAX) c += 1 ;
		}
	    }
	} /* end if (nonzero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (entername) */


static int procnewnames(PROGINFO *pip,ARGVALS *avsp,vecstr *nlp)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		newnames ;
	const int	opts = VECSTR_OSTATIONARY | VECSTR_OREUSE ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_rename/procnewnames: ent\n") ;
#endif

	if ((rs = vecstr_start(&newnames,20,opts)) >= 0) {
	    int		i ;
	    int		len ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    const char	*np ;
	    char	tbuf[MAXPATHLEN+1] ;
	    for (i = 0 ; vecstr_get(nlp,i,&np) >= 0 ; i += 1) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_rename/procnewnames: i=%u np=%s\n",i,np) ;
#endif
		if ((rs = makefname(pip,avsp,np,tbuf)) >= 0) {
		    len = rs ;
		    if (pip->debuglevel > 0) {
			fmt = "%s: fn=%s\n" ;
			shio_printf(pip->efp,fmt,pn,np) ;
			fmt = "%s: -> %s\n" ;
			shio_printf(pip->efp,fmt,pn,tbuf) ;
		    }
		    if ((rs = vecstr_add(&newnames,tbuf,len)) >= 0) {
	        	rs = locinfo_incr(lip,avsp->incr) ;
		    }
		} /* end if (makefname) */
		if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
		rs = procrename(pip,tbuf,nlp,&newnames) ;
	    } /* end if (ok) */
	    rs1 = vecstr_finish(&newnames) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (newnames) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_rename/procnewnames: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procnewnames) */


static int procrename(PROGINFO *pip,char *tbuf,vecstr *nlp,vecstr *nnlp)
{
	ZOMBIENAME	zn ;
	int		rs ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = zombiename_start(&zn,nnlp)) >= 0) {
	    int		i ;
	    cchar	*cp ;
	    for (i = 0 ; vecstr_get(nlp,i,&cp) >= 0 ; i += 1) {
		if (cp != NULL) {
		    if ((rs = vecstr_find(nnlp,cp)) >= 0) {
	                if ((rs = zombiename_rename(&zn,cp,tbuf)) >= 0) {
	                    vecstr_del(nlp,i) ;
	                    rs = vecstr_add(nlp,tbuf,-1) ;
		        }
		    } else if (rs == SR_NOTFOUND) {
			rs = SR_OK ;
	            } /* end if (name conflict) */
		}
		if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
	        cchar	*nnp ;
	        for (i = 0 ; vecstr_get(nlp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                if ((rs = vecstr_get(nnlp,i,&nnp)) >= 0) {
	                   rs = u_rename(cp,nnp) ;
	                }
		    }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */
	    rs1 = zombiename_finish(&zn) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end block (zombiename) */
	return rs ;
}
/* end subroutine (procrename) */


static int parsesort(PROGINFO *pip,cchar *vp,int vl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;

	rs = locinfo_setsort(lip,vp,vl) ;

	return rs ;
}
/* end subroutine (parsesort) */


static int makefname(PROGINFO *pip,ARGVALS *sip,cchar *np,char *tbuf)
{
	LOCINFO		*lip = pip->lip ;
	const int	dlen = DBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		prec = sip->prec ;
	char		dbuf[DBUFLEN + 1] ;
	char		sufbuf[SUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_rename/makefname: np=%s\n",np) ;
#endif

	if ((rs = locinfo_cvtstr(lip,dbuf,dlen,prec)) >= 0) {
	    SBUF	pbuf ;
	    const int	tlen = MAXPATHLEN ;
	    int		dl = rs ;
	    int		sl = -1 ;
	    cchar	*tp ;
	    cchar	*sp = lip->suffix ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("b_rename/makefname: numincr_cvtstr() rs=%d\n",rs) ;
	debugprintf("b_rename/makefname: str=%s\n",dbuf) ;
	}
#endif

	if ((rs = sbuf_start(&pbuf,tbuf,tlen)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_rename/makefname: basename=%s\n",sip->basename) ;
#endif

	    sbuf_strw(&pbuf,sip->basename,-1) ;

	    sbuf_strw(&pbuf,dbuf,dl) ;

	    switch (lip->suftype) {
	    case suf_minus:
	    case suf_null:
		if ((tp = strrchr(np,'.')) != NULL) {
		    sp = (tp+1) ;
		    if (sp[0] != '\0') {
			sl = -1 ;
			if (hasuc(sp,sl)) {
		    	    sl = strwcpylc(sufbuf,sp,SUFLEN) - sufbuf ;
		    	    sp = sufbuf ;
			}
	                sbuf_char(&pbuf,'.') ;
	                sbuf_strw(&pbuf,sp,sl) ;
		    }
		}
		break ;
	    case suf_new:
	    case suf_plus:
		sp = lip->suffix ;
		if (sp == NULL) sp = "xxx" ;
	        sbuf_char(&pbuf,'.') ;
	        sbuf_strw(&pbuf,sp,-1) ;
		break ;
	    case suf_empty:
		break ;
	    default:
		rs = SR_BUGCHECK ;
		break ;
	    } /* end switch */

	    rs1 = sbuf_finish(&pbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	} /* end if (numincr_cvtstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_rename/makefname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (makefname) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	rs = numincr_start(&lip->incr,"1",1) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->suffix != NULL) {
	    rs1 = uc_free(lip->suffix) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->suffix = NULL ;
	}

	rs1 = numincr_finish(&lip->incr) ;
	if (rs >= 0) rs = rs1 ;

	lip->sortkey = 0 ;
	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_setsuf(LOCINFO *lip,cchar *suf)
{
	int		rs = SR_OK ;

	if (suf == NULL) {
	    lip->suftype = suf_null ;
	} else {
	    const int	ch = MKCHAR(suf[0]) ;
	    switch (ch) {
	    case 0:
	        lip->suftype = suf_empty ;
		break ;
	    case '+':
	        lip->suftype = suf_plus ;
		break ;
	    case '-':
	        lip->suftype = suf_minus ;
		break ;
	    default:
	        lip->suftype = suf_new ;
		break ;
	    } /* end switch */
	} /* end if */

	switch (lip->suftype) {
	case suf_new:
	    {
	        cchar	*cp ;
	        rs = uc_mallocstrw(suf,-1,&cp) ;
	        if (rs >= 0) lip->suffix = cp ;
	    }
	    break ;
	case suf_plus:
	    lip->f.findsuffix = TRUE ;
	    break ;
	case suf_null:
	case suf_empty:
	case suf_minus:
	    break ;
	default:
	    rs = SR_BUGCHECK ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (locinfo_setsuf) */


static int locinfo_findsuf(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (lip->f.findsuffix) {
	    int		cl ;
	    cchar	*tp, *cp ;
	    char	sufbuf[SUFLEN + 1] ;
	    if ((tp = strnrchr(sp,sl,'.')) != NULL) {
	        cp = (tp+1) ;
	        cl = (sp + sl) - cp ;
	        if (cl > 0) {
	            const char	*ccp ;
		    if (hasuc(cp,cl)) {
		        cl = strwcpylc(sufbuf,cp,SUFLEN) - sufbuf ;
		        cp = sufbuf ;
		    }
		    if (lip->suffix != NULL) {
		        uc_free(lip->suffix) ;
		        lip->suffix = NULL ;
		    }
		    if ((rs = uc_mallocstrw(cp,cl,&ccp)) >= 0) {
		        lip->suffix = ccp ;
		        lip->f.findsuffix = FALSE ;
		    }
	        }
	    }
	} /* end if (findsuffix) */

	return rs ;
}
/* end subroutine (locinfo_findsuf) */


static int locinfo_setcount(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (sp != NULL) {
	    rs = numincr_load(&lip->incr,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (locinfo_setcount) */


static int locinfo_setprec(LOCINFO *lip,int prec)
{
	int		rs ;

	rs = numincr_setprec(&lip->incr,prec) ;

	return rs ;
}
/* end subroutine (locinfo_setprec) */


static int locinfo_setsort(LOCINFO *lip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		si ;

	if (vl < 0) vl = strlen(vp) ;

	if (vl > 0) {
	    if ((si = matostr(sortkeys,1,vp,vl)) >= 0) {
	        lip->sortkey = si ;
	        lip->f.sortkey = TRUE ;
	    } else {
	        rs = SR_INVALID ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_setsort) */


static int locinfo_incr(LOCINFO *lip,int incr)
{
	return numincr_incr(&lip->incr,incr) ;
}
/* end subroutine (locinfo_incr) */


static int locinfo_cvtstr(LOCINFO *lip,char *dbuf,int dlen,int prec)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("b_rename/locinfo_cvtstr: ent\n") ;
#endif
	rs = numincr_cvtstr(&lip->incr,dbuf,dlen,prec) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("b_rename/locinfo_cvtstr: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_cvtstr) */


static int zombiename_start(ZOMBIENAME *op,VECSTR *nnp)
{

	memset(op,0,sizeof(ZOMBIENAME)) ;
	op->nnp = nnp ;

	return SR_OK ;
}
/* end subroutine (zombiename_start) */


static int zombiename_rename(ZOMBIENAME *op,cchar *fname,char *tmpfname)
{
	struct ustat	usb ;
	struct ustat	*sbp = &usb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nprec = ipow(10,DEFPREC) ;
	int		n ;
	int		len = 0 ;

/* do we have an existing prefix? */

	if (op->prefix[0] == '\0') {
	    rs = sncpy1(op->prefix,MAXNAMELEN,ZOMBIEPREFIX) ;
	}

	if (rs >= 0) {
	    const int	nrs = SR_NOTFOUND ;

	    for (n = op->n ; n < nprec ; n += 1) {
	        if ((rs = mkfnamenum(tmpfname,op->prefix,DEFPREC,n)) >= 0) {
	            len = rs ;
	            if ((rs1 = vecstr_find(op->nnp,tmpfname)) == nrs) {
	                rs1 = u_stat(tmpfname,sbp) ;
	                if (rs1 == SR_NOENT) break ;
	            }
		}
		if (rs < 0) break ;
	    } /* end for */

	    if ((rs >= 0) && (n >= nprec)) {
	        rs = SR_EXIST ;
	    }

	} /* end if (ok) */

	if (rs >= 0) {
	    rs = u_rename(fname,tmpfname) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (zombiename_rename) */


static int zombiename_finish(ZOMBIENAME *op)
{

	op->prefix[0] = '\0' ;
	op->n = 0 ;
	op->nnp = NULL ;
	return SR_OK ;
}
/* end subroutine (zombiename_finish) */


static int mkfnamenum(char *tmpfname,cchar *prefix,int prec,int fn)
{
	const int	dlen = DBUFLEN ;
	int		rs ;
	int		rs1 ;
	char		dbuf[DBUFLEN + 1] ;

	if ((rs = ctdecpi(dbuf,dlen,prec,fn)) >= 0) {
	    SBUF	pbuf ;
	    if ((rs = sbuf_start(&pbuf,tmpfname,MAXPATHLEN)) >= 0) {
	        sbuf_strw(&pbuf,prefix,-1) ;
	        sbuf_strw(&pbuf,dbuf,-1) ;
	        rs1 = sbuf_finish(&pbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	} /* end if (cfdecpi) */

	return rs ;
}
/* end subroutine (mkfnamenum) */


/* argument values */
static int argvals_check(ARGVALS *ap)
{

	if (ap->prec < 0)
	    ap->prec = 0 ;

	if (ap->prec > DBUFLEN)
	    ap->prec = DBUFLEN ;

	if (ap->incr < 1)
	    ap->incr = 1 ;

	if (ap->basename == NULL)
	    ap->basename = DEFBASENAME ;

	if ((ap->suffix != NULL) && (ap->suffix[0] == '-'))
	    ap->suffix =  NULL ;

	return SR_OK ;
}
/* end subroutine (argvals_check) */


