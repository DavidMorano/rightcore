/* b_pathenum */

/* front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_GETEV	1		/* use 'getev(3dam)' */


/* revision history:

	= 2004-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end for retrieving environment variables and
	outputting them in a packaged-up format for SHELL interpretation.

	Synopsis:

	$ pathenum <varname>


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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<nulstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_pathenum.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO_MAGIC	0x99224571
#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getev(const char **,const char *,int,const char **) ;
extern int	isalnumlatin(int) ;
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
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo {
	uint		magic ;
	PROGINFO	*pip ;
	const char	*es ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procname(PROGINFO *,SHIO *,const char *,int) ;
static int	printit(PROGINFO *,SHIO *,const char *,int) ;
static int	isenvnameok(const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_setes(LOCINFO *,const char *) ;
static int	locinfo_getes(LOCINFO *,const char **) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_DEBUG || CF_DEBUGS
static int	locinfo_debug(LOCINFO *,const char *) ;
static int	strdeblen(const char *) ;
#endif


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
	"es",
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
	argopt_es,
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
	"es",
	NULL
} ;

enum progopts {
	progopt_es,
	progopt_overlast
} ;


/* exported subroutines */


int b_pathenum(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_pathenum) */


int p_pathenum(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_pathenum) */


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
	    debugprintf("b_pathenum: starting DFD=%d\n",rs) ;
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

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

#if	CF_DEBUGS || CF_DEBUG
	locinfo_debug(&li,"0") ;
#endif

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

	                case argopt_es:
	                        if (argr > 0) {
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        rs = locinfo_setes(&li,argp) ;
				} else
	                            rs = SR_INVALID ;
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

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

	if (rs < 0) {
	    ex = EX_USAGE ;
	    if (pip->efp != NULL)
	        shio_printf(pip->efp,
	            "%s: invalid argument specified (%d)\n",
	            pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathenum: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: verboselevel=%d\n",
	        pip->progname,pip->verboselevel) ;
	}

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

/* option parsing */

#if	CF_DEBUGS || CF_DEBUG
	locinfo_debug(&li,"1") ;
#endif

	rs = procopts(pip,&akopts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pathenum: done w/ proc options\n") ;
#endif

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

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
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOINPUT ;
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
	    debugprintf("b_pathenum: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
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

	    fmt = "%s: USAGE> %s <varname(s)> [...] [-es <string>]\n" ;
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
	int		size ;

#if	CF_DEBUGS
	debugprintf("locinfo_start: ent\n") ;
#endif

	if (lip == NULL) return SR_FAULT ;

	size = sizeof(LOCINFO) ;
	memset(lip,0,size) ;
	lip->pip = pip ;
	lip->magic = LOCINFO_MAGIC ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("locinfo_finish: magic=%s\n",
	    ((lip->magic == LOCINFO_MAGIC) ? "OK" : "BAD")) ;
#endif

	if (lip->magic != LOCINFO_MAGIC)
	    return SR_NOTOPEN ;

	if (lip->es != NULL) {
	    rs1 = uc_free(lip->es) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->es = NULL ;
	}

	lip->magic = 0 ;
	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_setes(LOCINFO *lip,cchar *es)
{
	int		rs = SR_OK ;
	const char	*cp ;

	if (lip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("locinfo_setes: magic=%s\n",
	    ((lip->magic == LOCINFO_MAGIC) ? "OK" : "BAD")) ;
#endif

	if (lip->magic != LOCINFO_MAGIC) return SR_NOTOPEN ;

	if (es == NULL) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("locinfo_setes: es=>%s<\n",es) ;
	debugprintf("locinfo_setes: old_es=>%s<%d\n",
	    lip->es,strdeblen(lip->es)) ;
#endif

	if (lip->es != NULL) {
	    uc_free(lip->es) ;
	    lip->es = NULL ;
	}

	rs = uc_mallocstrw(es,-1,&cp) ;
	if (rs >= 0) lip->es = cp ;

#if	CF_DEBUGS
	debugprintf("locinfo_setes: new_es=>%s<%d\n",
	    lip->es,strdeblen(lip->es)) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_setes) */


static int locinfo_getes(LOCINFO *lip,cchar **rpp)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("locinfo_getes: magic=%s\n",
	    ((lip->magic == LOCINFO_MAGIC) ? "OK" : "BAD")) ;
#endif

	if (lip->magic != LOCINFO_MAGIC) return SR_NOTOPEN ;

	if (lip->es != NULL) {
	    rs = strlen(lip->es) ;
	}

#if	CF_DEBUGS
	debugprintf("locinfo_getes: cur_es=>%s<%d rs=%d\n",
	    lip->es,strdeblen(lip->es),rs) ;
#endif

	if (rpp != NULL)
	    *rpp = lip->es ;

	return rs ;
}
/* end subroutine (locinfo_getes) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
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

	                case progopt_es:
	                    c += 1 ;
	                    if (vl > 0)
	                        rs = locinfo_setes(lip,vp) ;
	                    break ;

	                } /* end switch */

	                c += 1 ;
	            } else
	                rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        rs1 = keyopt_curend(kop,&kcur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"r",0666)) >= 0) {
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
	            }

	            if (rs < 0) {
	                if (rs == SR_NOENT) {
	                    shio_printf(pip->efp,
	                        "%s: variable not present (%d)\n",
	                        pip->progname,rs) ;
	                } else
	                    shio_printf(pip->efp,
	                        "%s: error processing variable (%d)\n",
	                        pip->progname,rs) ;
	                shio_printf(pip->efp,
	                    "%s: var=%s\n",
	                    pip->progname,cp) ;
	            }

	            if (rs < 0) break ;
	        } /* end for */
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

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procname(pip,ofp,cp,cl) ;
	                    }
	                }

	                if (rs < 0) {
	                    if (rs == SR_NOENT) {
	                        shio_printf(pip->efp,
	                            "%s: variable not present (%d)\n",
	                            pip->progname,rs) ;
	                    } else
	                        shio_printf(pip->efp,
	                            "%s: error processing variable (%d)\n",
	                            pip->progname,rs) ;
	                    shio_printf(pip->efp,
	                        "%s: var=%s\n",
	                        pip->progname,cp) ;
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
	                    pip->progname,afn) ;
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    shio_printf(pip->efp,"%s: inaccessible output (%d)\n",
	        pip->progname,rs) ;
	}

	return rs ;
}
/* end subroutine (procargs) */


/* process a name */
static int procname(PROGINFO *pip,SHIO *ofp,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procname: name=%t\n",np,nl) ;
#endif

	if (isenvnameok(np,nl)) {
	    const char	*sp ;

#if	CF_GETEV
	    rs = getev(pip->envv,np,nl,&sp) ;
#else
	    {
	        NULSTR	ename ;
	        char	*name ;
	        if ((rs = nulstr_start(&ename,np,nl,&name)) >= 0) {
	            sp = getourenv(pip->envv,name) ;
	            nulstr_finish(&ename) ;
	            if (sp == NULL) rs = SR_NOENT ;
	        } /* end if (nulstr) */
	    }
#endif /* CF_GETEV */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procname: splitting\n") ;
#endif

	    if (rs >= 0) {
	        const char	*tp ;

	        while ((tp = strpbrk(sp,":;")) != NULL) {

	            c += 1 ;
	            rs = printit(pip,ofp,sp,(tp - sp)) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("procname: printit() rs=%d\n",rs) ;
#endif

	            if ((rs >= 0) && (*tp == ';'))
	                rs = shio_printf(ofp,"+\n") ;

	            sp = (tp + 1) ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (sp[0] != '\0')) {

	            c += 1 ;
	            rs = printit(pip,ofp,sp,-1) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("procname: printit() rs=%d\n",rs) ;
#endif

	        } /* end if (left-over) */

	    } /* end if (ok) */

	} else
	    rs = SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


/* print it out */
static int printit(PROGINFO *pip,SHIO *ofp,cchar *sp,int sl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (sl < 0) sl = strlen(sp) ;

/* handle an empty string with the option of an empty-string value */

	if (sl == 0) {
	    const char	*es ;
	    if ((rs = locinfo_getes(lip,&es)) > 0) {
	        if (es != NULL) {
	            sp = es ;
		    sl = rs ;
	        }
	    }
	} /* end if (empty) */

	if (rs >= 0) {
	    rs = shio_print(ofp,sp,sl) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printit) */


#if	CF_DEBUG || CF_DEBUGS

static int locinfo_debug(LOCINFO *lip,cchar s[])
{
	if (lip == NULL) return SR_FAULT ;
	debugprintf("locinfo_debug: magic=%s\n",
	    ((lip->magic == LOCINFO_MAGIC) ? "OK" : "BAD")) ;
	if (lip->magic != LOCINFO_MAGIC) return SR_NOTOPEN ;
	debugprintf("locinfo_debug: %s cur_es=>%s<%d \n",
	    s,lip->es,strdeblen(lip->es)) ;
	return SR_OK ;
}
/* end subroutine (locinfo_debug) */


static int strdeblen(cchar *s)
{
	if (s == NULL) return -1 ;
	return (strlen(s)) ;
}

#endif /* CF_DEBUGS */


/* is the given environment varialbe OK? */
static int isenvnameok(cchar *kp,int kl)
{
	int		ch ;
	int		f_ok = TRUE ;

	if ((kp == NULL) || (*kp == '\0'))
	    return FALSE ;

/* first character has to be AlNum */

	if (f_ok && kl && *kp) {
	    ch = (*kp & 0xff) ;
	    f_ok = isalnumlatin(ch) ;
	    kp += 1 ;
	    kl -= 1 ;
	}

/* subsequent characters can be more relaxed */

	while (f_ok && kl && *kp) {
	    ch = (*kp & 0xff) ;
	    f_ok = (isalnumlatin(ch) || (ch == '_')) ;
	    kp += 1 ;
	    kl -= 1 ;
	} /* end while */

	return f_ok ;
}
/* end subroutine (isenvnameok) */


