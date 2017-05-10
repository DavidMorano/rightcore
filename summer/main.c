/* main */

/* front-end for whatever */


#define	CF_DEBUGS	0		/* non-switchables */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 2002-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous program by
	the same name did.


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<bfile.h>
#include	<vecobj.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
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

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;

extern double	fsum(double *,int) ;
extern double	fam(double *,int) ;
extern double	fhm(double *,int) ;


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

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;

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


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ki, wi ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
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
	                            pmspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pmspec = argp ;
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	        else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	switch (pip->progmode) {
	case progmode_asum:
	    proginfo_setbanner(pip,BANNER_ASUM) ;
	    break ;
	case progmode_amean:
	    proginfo_setbanner(pip,BANNER_AMEAN) ;
	    break ;
	case progmode_hmean:
	    proginfo_setbanner(pip,BANNER_HMEAN) ;
	    break ;
	case progmode_speedup:
	    proginfo_setbanner(pip,BANNER_SPEEDUP) ;
	    break ;
	default:
	    pip->progmode = progmode_asum ;
	    break ;
	} /* end switch */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some progopts */

	rs = procopts(pip,&akopts) ;

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
	        pip->f.sum,pip->f.amean,pip->f.hmean) ;
#endif

	if (pip->debuglevel > 0) {

	    if (lip->f.sum)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_sum]) ;

	    if (lip->f.amean)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_amean]) ;

	    if (lip->f.hmean)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_hmean]) ;

	    if (lip->f.speedup)
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_speedup]) ;

	} /* end if */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    rs = process(pip,&ainfo,&pargs,ofname,afname,ifname) ;
	}

/* done */

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;
#endif

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
badoutopen:
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = TRUE ;
	    bclose(pip->efp) ;
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
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<value(s)> ...] [-o <calculation>] [-Vv]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af {<afile>|-}]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ki ;
	int		wi ;
	int		c = 0 ;
	int		cl ;
	cchar		*cp ;

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

	for (ki = 0 ; progopts[ki] != NULL ; ki += 1) {
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {

	        while (rs >= 0) {

	            rs1 = keyopt_enumvalues(kop,progopts[ki],&cur,&cp) ;
	            if (rs1 < 0) break ;
	            cl = rs1 ;

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

	        } /* end while (enumerating) */

	        keyopt_curend(kop,&cur) ;
	    } /* end if (keyopt-cur) */
	} /* end for (progopts) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(pip,aip,bop,ofn,afn,ifn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
cchar		*ofn ;
cchar		*afn ;
cchar		*ifn ;
{
	LOCINFO		*lip = pip->lip ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    PROCESSOR	nproc ;

	    if ((rs = processor_start(&nproc,NDEFAULT)) >= 0) {
	        int		cl ;
	        int		pan = 0 ;
	        cchar		*cp ;

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
	            } /* end for (looping through requested circuits) */
	        } /* end if (ok) */

/* process any files in the argument filename list file */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            bfile	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	            if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(afp,lbuf,llen)) > 0) {
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

	                rs1 = bclose(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
	                    bprintf(pip->efp,
	                        "%s: inaccessible argument-list (%d)\n",
	                        pip->progname,rs) ;
	                    bprintf(pip->efp,"%s:  afile=%s\n",
	                        pip->progname,afn) ;
	                }
	            } /* end if */

	        } /* end if (processing file argument file list) */

	        if ((rs >= 0) && (pan == 0)) {
	            bfile	ifile, *ifp = &ifile ;

	            if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	                ifn = STDINFNAME ;

	            if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(ifp,lbuf,llen)) > 0) {
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

	                rs1 = bclose(ifp) ;
			if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
	                    bprintf(pip->efp,
	                        "%s: inaccessible input (%d)\n",
	                        pip->progname,rs) ;
	                    bprintf(pip->efp,"%s: ifile=%s\n",
	                        pip->progname,ifn) ;
	                }
	            } /* end if */

	        } /* end if (processing STDIN) */

/* print out the results */

	        if (rs >= 0) {
	            double	fnum ;
	            int	wi ;

	            if (lip->f.sum) {
	                wi = which_sum ;
	                rs = processor_result(&nproc,wi,&fnum) ;
	                if (rs >= 0)
	                    bprintf(ofp,"%14.4f\n",fnum) ;
	            }

	            if (lip->f.amean) {
	                wi = which_amean ;
	                rs = processor_result(&nproc,wi,&fnum) ;
	                if (rs >= 0)
	                    bprintf(ofp,"%14.4f\n",fnum) ;
	            }

	            if (lip->f.hmean) {
	                wi = which_hmean ;
	                rs = processor_result(&nproc,wi,&fnum) ;
	                if (rs >= 0)
	                    bprintf(ofp,"%14.4f\n",fnum) ;
	            }

	            if (lip->f.speedup) {
	                int	size, i ;
	                double	*fa ;

	                size = pan * sizeof(double) ;
	                if ((rs = uc_malloc(size,&fa)) >= 0) {

	                    wi = which_speedup ;
	                    rs = processor_result(&nproc,wi,fa) ;

	                    if (rs >= 0) {
	                        for (i = 0 ; i < pan ; i += 1) {
	                            bprintf(ofp,"%s%14.4f",
	                                ((i > 0) ? " " : ""),
	                                fa[i]) ;
	                        }
	                        bprintf(ofp,"\n") ;
	                    } /* end if */

	                    uc_free(fa) ;
	                } /* end if (memory-allocation) */

	            } /* end if (speedup result) */

	        } /* end if (outputting results) */

	        if ((rs >= 0) && (pan == 0)) {
	            bprintf(pip->efp,"%s: no numbers were specified\n",
	                pip->progname) ;
	        }

	        rs1 = processor_finish(&nproc) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (processor) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: inaccessible output (%d)\n",
	        pip->progname,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


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

#if	CF_DEBUGS
	debugprintf("main/processor_start: vecobj_start() rs=%d\n",rs) ;
#endif

	op->open = (rs >= 0) ;
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


static int processor_add(PROCESSOR *op,cchar s[],int slen)
{
	double		fnum ;
	int		rs = SR_OK ;
	int		sl, cl ;
	const char	*sp ;
	const char	*tp, *cp ;

	if (! op->open)
	    return SR_NOTOPEN ;

	sp = s ;
	sl = (slen >= 0) ? slen : strlen(s) ;

	if ((tp = strnchr(sp,sl,'#')) != NULL)
	    sl = (tp - sp) ;

#if	CF_DEBUGS
	debugprintf("main/processor_add: line=>%t<\n",sp,sl) ;
#endif

	while ((cl = nextfield(sp,sl,&cp)) > 0) {

#if	CF_DEBUGS
	    debugprintf("main/processor_add: str=>%t<\n",cp,cl) ;
#endif

	    rs = cfdecf(cp,cl,&fnum) ;

#if	CF_DEBUGS
	    debugprintf("main/processor_add: cfdecf() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        rs = vecobj_add(&op->numbers,&fnum) ;

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
	int		n ;

	if (! op->open)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	{
	    int	i ;
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
	    double	*fnp ;
	    int		size, i, j ;

	    rs = vecobj_count(&op->numbers) ;
	    n = rs ;
	    size = (n + 1) * sizeof(double) ;
	    if (rs >= 0) {
	        void	*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            VECOBJ	*flp = &op->numbers ;
	            op->fa = p ;

	            j = 0 ;
	            for (i = 0 ; vecobj_get(flp,i,&fnp) >= 0 ; i += 1) {
	                if (fnp == NULL) continue ;
	                op->fa[j++] = *fnp ;
	            } /* end for */

	            n = j ;
	        } /* end if (populating) */

	    } /* end if */
	} /* end if (creating array) */

	if (rs >= 0) {
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
	                if (op->fa[0] > 0)
	                    rp[i] = (op->fa[i] / op->fa[0]) ;
	            } /* end for */
	        } /* end if */
	        break ;
	    } /* end switch */
	} /* end if (ok) */

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


#if	CF_SETLOCENT
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

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
	        *epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_SETLOCENT */


