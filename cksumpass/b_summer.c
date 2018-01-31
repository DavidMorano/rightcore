/* b_summer (KSH built-in) */

/* main subroutine for SUMMER */


#define	CF_DEBUGS	0		/* non-switchables */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 2002-02-01, David A­D­ Morano
	The program was written from scratch.

	= 2017-08-04, David A­D­ Morano
	Turned into a KSH built-in.

*/

/* Copyright © 2002,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


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
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<vecobj.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_summer.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROCESSOR	struct processor


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecf(const char *,int, double *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;

extern double	fsum(double *,int) ;
extern double	fam(double *,int) ;
extern double	fhm(double *,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct processor {
	VECOBJ		numbers ;
	double		*fa ;
	uint		open:1 ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		sum:1 ;
	uint		amean:1 ;
	uint		hmean:1 ;
	uint		speedup:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procout(PROGINFO *,void *,PROCESSOR *,int) ;

static int	processor_start(PROCESSOR *,int) ;
static int	processor_finish(PROCESSOR *) ;
static int	processor_add(PROCESSOR *,const char *,int) ;
static int	processor_result(PROCESSOR *,int,double *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static const char	*progmodes[] = {
	"asum",
	"amean",
	"hmean",
	"speedup",
	NULL
} ;

enum progmodes {
	progmode_asum,
	progmode_amean,
	progmode_hmean,
	progmode_speedup,
	progmode_overlast
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"option",
	"af",
	"ef",
	"of",
	"if",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_option,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_overlast
} ;

static const char	*progopts[] = {
	"type",
	"sum",
	"asum",
	"amean",
	"hmean",
	"speedup",
	NULL
} ;

enum progopts {
	progopt_type,
	progopt_sum,
	progopt_asum,
	progopt_amean,
	progopt_hmean,
	progopt_speedup,
	progopt_overlast
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

static const char	*whiches[] = {
	"sum",
	"amean",
	"hmean",
	"speedup",
	NULL
} ;

enum whiches {
	which_sum,
	which_amean,
	which_hmean,
	which_speedup,
	which_overlast
} ;


/* exported subroutines */


int b_summer(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_summer) */


int b_asum(int argc,cchar *argv[],void *contextp)
{
	return b_summer(argc,argv,contextp) ;
}
/* end subroutine (b_asum) */


int b_amean(int argc,cchar *argv[],void *contextp)
{
	return b_summer(argc,argv,contextp) ;
}
/* end subroutine (b_amean) */


int b_hmean(int argc,cchar *argv[],void *contextp)
{
	return b_summer(argc,argv,contextp) ;
}
/* end subroutine (b_hmean) */


int b_speedup(int argc,cchar *argv[],void *contextp)
{
	return b_summer(argc,argv,contextp) ;
}
/* end subroutine (b_speedup) */


int p_summer(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_summer) */


int p_asum(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_asum) */


int p_amean(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_amean) */


int p_hmean(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_hmean) */


int p_speedup(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_speedup) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar **argv,cchar **envv,void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pm = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

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

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

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

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* search name */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
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

/* input file */
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

	        } /* end if (digits or progopts) */

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
	    shio_control(pip->efp,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(pip->efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(pip->efp,fmt,pn,pm) ;
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else {
	        debugprintf("main: progmode=NONE\n") ;
	    }
	}
#endif /* CF_DEBUG */

	switch (pip->progmode) {
	case progmode_asum:
	    rs = proginfo_setbanner(pip,BANNER_ASUM) ;
	    break ;
	case progmode_amean:
	    rs = proginfo_setbanner(pip,BANNER_AMEAN) ;
	    break ;
	case progmode_hmean:
	    rs = proginfo_setbanner(pip,BANNER_HMEAN) ;
	    break ;
	case progmode_speedup:
	    rs = proginfo_setbanner(pip,BANNER_SPEEDUP) ;
	    break ;
	default:
	    pip->progmode = progmode_asum ;
	    break ;
	} /* end switch */

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

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some progopts */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* if we don't have a request for something yet, use our progmode */

	f = FALSE ;
	f = f || lip->f.sum ;
	f = f || lip->f.amean ;
	f = f || lip->f.hmean ;
	f = f || lip->f.speedup ;
	if (! f) {
	    switch (pip->progmode) {
	    case progmode_speedup:
	        lip->f.speedup = TRUE ;
	        break ;
	    case progmode_amean:
	        lip->f.amean = TRUE ;
	        break ;
	    case progmode_hmean:
	        lip->f.hmean = TRUE ;
	        break ;
	    case progmode_asum:
	    default:
	        lip->f.sum = TRUE ;
	        break ;
	    } /* end switch */
	} /* end if (didn't get anuthing yet) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: requests- sum=%u amean=%u hmean=%u\n",
	        lip->f.sum,lip->f.amean,lip->f.hmean) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: request> %s\n" ;

	    if (lip->f.sum) {
	        shio_printf(pip->efp,fmt,pn,whiches[which_sum]) ;
	    }

	    if (lip->f.amean) {
	        shio_printf(pip->efp,fmt,pn,whiches[which_amean]) ;
	    }

	    if (lip->f.hmean) {
	        shio_printf(pip->efp,fmt,pn,whiches[which_hmean]) ;
	    }

	    if (lip->f.speedup) {
	        shio_printf(pip->efp,fmt,pn,whiches[which_speedup]) ;
	    }

	} /* end if */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    rs = process(pip,aip,bop,ofname,afname,ifname) ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
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
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = TRUE ;
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

	fmt = "%s: USAGE> %s [<value(s)> ...] [-o <calculation>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af {<afile>|-}] [-of <ofile>]\n" ;
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
	int		cl ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    KEYOPT_CUR	cur ;
	    debugprintf("main: progopts specified:\n") ;
	    keyopt_curbegin(kop,&cur) ;
	    while ((rs = keyopt_enumkeys(kop,&cur,&cp)) >= 0) {
	        if (cp == NULL) continue ;
	        debugprintf("main: | optkey=%s\n",cp) ;
	    }
	    keyopt_curend(kop,&cur) ;
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {
	    int		ki ;
	    int		wi ;
	    for (ki = 0 ; progopts[ki] != NULL ; ki += 1) {
	        KEYOPT_CUR	cur ;
	        if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {

	            while (rs >= 0) {
	                cl = keyopt_enumvalues(kop,progopts[ki],&cur,&cp) ;
	                if (cl == SR_NOTFOUND) break ;
		        rs = cl ;
		        if (rs >= 0) {
	                    switch (ki) {
	                    case progopt_type:
	                        if (cl > 0) {
	                            wi = matostr(whiches,2,cp,cl) ;
	                            switch (wi) {
	                            case which_sum:
	                                lip->f.sum = TRUE ;
	                                break ;
	                            case which_amean:
	                                lip->f.amean = TRUE ;
	                                break ;
	                            case which_hmean:
	                                lip->f.hmean = TRUE ;
	                                break ;
	                            case which_speedup:
	                                lip->f.speedup = TRUE ;
	                                break ;
	                            } /* end switch */
	                        } /* end if (non-zero value) */
	                        break ;
	                    case progopt_sum:
	                    case progopt_asum:
	                        lip->f.sum = TRUE ;
	                        break ;
	                    case progopt_amean:
	                        lip->f.amean = TRUE ;
	                        break ;
	                    case progopt_hmean:
	                        lip->f.hmean = TRUE ;
	                        break ;
	                    case progopt_speedup:
	                        lip->f.speedup = TRUE ;
	                        break ;
	                    } /* end switch */
		        } /* end if (ok) */
	            } /* end while (enumerating) */

	            keyopt_curend(kop,&cur) ;
	        } /* end if (keyopt-cur) */
	    } /* end for (progopts) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn,
			cchar *ifn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0644)) >= 0) {
	    PROCESSOR	nproc ;

	    if ((rs = processor_start(&nproc,NDEFAULT)) >= 0) {
	        int	cl ;
	        int	pan = 0 ;
	        cchar	*cp ;

	        if (rs >= 0) {
	            int		ai ;
	            int		argc = aip->argc ;
	            int		f ;
	            cchar	**argv = aip->argv ;
	            for (ai = 1 ; ai < argc ; ai += 1) {

	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = argv[ai] ;
	                    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = processor_add(&nproc,cp,-1) ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (ok) */

/* process any files in the argument filename list file */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            SHIO	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

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
	                    	    rs = processor_add(&nproc,cp,cl) ;
	                        }
	                    }

			    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
			    fmt = "%s: inaccessible argument-list (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                    shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	                }
	            } /* end if */

	        } /* end if (processing file argument file list) */

	        if ((rs >= 0) && (pan == 0)) {
	            SHIO	ifile, *ifp = &ifile ;

	            if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	                ifn = STDINFNAME ;

	            if ((rs = shio_open(ifp,ifn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                        if (cp[0] != '#') {
	                            pan += 1 ;
	                    	    rs = processor_add(&nproc,cp,cl) ;
	                        }
	                    }

			    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(ifp) ;
			if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
			    fmt = "%s: inaccessible input (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                    shio_printf(pip->efp,"%s: ifile=%s\n",pn,ifn) ;
	                }
	            } /* end if */

	        } /* end if (processing STDIN) */

/* print out the results */

	        if (rs >= 0) {
		    rs = procout(pip,ofp,&nproc,pan) ;
		    wlen += rs ;
	        } /* end if (ok) */

	        rs1 = processor_finish(&nproc) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (processor) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procout(PROGINFO *pip,void *ofp,PROCESSOR *pp,int pan)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (pan > 0) {
	    double	fnum ;
	    int		wi ;

	    if (lip->f.sum) {
	        wi = which_sum ;
	        rs = processor_result(pp,wi,&fnum) ;
	        if (rs >= 0) {
	            rs = shio_printf(ofp,"%14.4f\n",fnum) ;
		    wlen += rs ;
		}
	    }

	    if (lip->f.amean) {
	        wi = which_amean ;
	        rs = processor_result(pp,wi,&fnum) ;
	        if (rs >= 0)
	            rs = shio_printf(ofp,"%14.4f\n",fnum) ;
		    wlen += rs ;
	    }

	    if (lip->f.hmean) {
	        wi = which_hmean ;
	        rs = processor_result(pp,wi,&fnum) ;
	        if (rs >= 0)
	            rs = shio_printf(ofp,"%14.4f\n",fnum) ;
		    wlen += rs ;
	    }

	    if (lip->f.speedup) {
	        const int	size = pan * sizeof(double) ;
	        double		*fa ;
	        if ((rs = uc_malloc(size,&fa)) >= 0) {
	            wi = which_speedup ;
	            rs = processor_result(pp,wi,fa) ;
	            if (rs >= 0) {
			int	i ;
	                cchar	*s = "" ;
	                fmt = "%s%14.4f" ;
	                for (i = 0 ; (rs >= 0) && (i < pan) ; i += 1) {
	                    if (i > 0) s = " " ;
	                    rs = shio_printf(ofp,fmt,s,fa[i]) ;
		    	    wlen += rs ;
	                }
			if (rs >= 0) {
	                    rs = shio_printf(ofp,"\n") ;
		    	    wlen += rs ;
			}
	            } /* end if */
	            uc_free(fa) ;
	        } /* end if (m-a-f) */
	    } /* end if (speedup result) */

	} else {
	    fmt = "%s: no numbers were specified\n" ;
	    shio_printf(pip->efp,fmt,pn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int processor_start(PROCESSOR *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	if (n < 3) n = 3 ;

	memset(op,0,sizeof(PROCESSOR)) ;

	size = sizeof(double) ;
	opts = VECOBJ_OCOMPACT ;
	rs = vecobj_start(&op->numbers,size,n,opts) ;
	op->open = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("main/processor_start: vecobj_start() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (processor_start) */


static int processor_finish(PROCESSOR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (! op->open)
	    return SR_NOTOPEN ;

	if (op->fa != NULL) {
	    rs1 = uc_free(op->fa) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fa = NULL ;
	}

	rs1 = vecobj_finish(&op->numbers) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (processor_finish) */


static int processor_add(PROCESSOR *op,cchar *sp,int sl)
{
	double		fnum ;
	int		rs = SR_OK ;
	int		cl ;
	const char	*tp, *cp ;

	if (! op->open)
	    return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	if ((tp = strnchr(sp,sl,'#')) != NULL) {
	    sl = (tp - sp) ;
	}

#if	CF_DEBUGS
	debugprintf("main/processor_add: line=>%t<\n",sp,sl) ;
#endif

	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    if ((rs = cfdecf(cp,cl,&fnum)) >= 0) {
	        rs = vecobj_add(&op->numbers,&fnum) ;
	    }

	    sl -= (cp + cl - sp) ;
	    sp = (cp + cl) ;

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (processor_add) */


static int processor_result(PROCESSOR *op,int which,double *rp)
{
	int		rs = SR_OK ;

	if (rp == NULL) return SR_FAULT ;

	if (! op->open) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	    int		i ;
	    double	*fnp ;
	    rs = vecobj_count(&op->numbers) ;
	    n = rs ;
	    debugprintf("main/processor_result: n=%u\n",n) ;
	    for (i = 0 ; vecobj_get(&op->numbers,i,&fnp) >= 0 ; i += 1) {
	        debugprintf("main/processor_result: num[%u]=%14.4f\n",
	            i,*fnp) ;
	    }
	}
#endif /* CF_DEBUG */

	if (op->fa == NULL) {
	    if ((rs = vecobj_count(&op->numbers)) >= 0) {
	    	const int	size = ((rs + 1) * sizeof(double)) ;
	        int		n = rs ;
	        void		*p ;

	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            VECOBJ	*flp = &op->numbers ;
	    	    double	*fnp ;
		    int		j = 0 ;
		    int		i ;
	            op->fa = p ;

	            for (i = 0 ; vecobj_get(flp,i,&fnp) >= 0 ; i += 1) {
	                if (fnp != NULL) {
	                    op->fa[j++] = *fnp ;
			}
	            } /* end for */

	            n = j ;
	        } /* end if (m-a) */

	        switch (which) {
	        case which_sum:
	            *rp = fsum(op->fa,n) ;
	            break ;
	        case which_amean:
	            *rp = fam(op->fa,n) ;
	            break ;
	        case which_hmean:
	            *rp = fhm(op->fa,n) ;
	            break ;
	        case which_speedup:
	            if (n > 0) {
	                int	i ;
	                rp[0] = 1.0 ;
	                for (i = 1 ; i < n ; i += 1) {
	                    rp[0] = 1.0 ;
	                    if (op->fa[0] > 0) {
	                        rp[i] = (op->fa[i] / op->fa[0]) ;
			    }
	                } /* end for */
	            } /* end if */
	            break ;
	        } /* end switch */

	    } /* end if (vecobj_count) */
	} /* end if (creating array) */

	return rs ;
}
/* end subroutine (processor_result) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

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
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(&lip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&lip->stores,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


