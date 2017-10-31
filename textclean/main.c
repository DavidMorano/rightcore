/* main */

/* this is a generic "main" module modified for the 'textclean' program */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-alloctions */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for small programs.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<char.h>
#include	<nulstr.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	OPTIONS		struct options
#define	OPTIONS_FL	struct options_flags

#define	STATE		struct state
#define	STATE_FL	struct state_flags


/* external subroutines */

extern int	snwcpycompact(char *,int,cchar *,int) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfnumi(cchar *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getpwusername(struct passwd *,char *,int,uid_t) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	hasallwhite(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	iseol(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,int,cchar *) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct options_flags {
	uint		inplace:1 ;
	uint		lower:1 ;
	uint		doublespace:1 ;
	uint		half:1 ;
	uint		oneblank:1 ;
	uint		noblanks:1 ;
	uint		rmleading:1 ;
	uint		rmtrailing:1 ;
	uint		mscrap:1 ;		/* M$ crap! */
	uint		pad:1 ;
	uint		snug:1 ;
	uint		subnbsp:1 ;
} ;

struct options {
	OPTIONS_FL	f ;
	int		pad ;
} ;

struct state_flags  {
	uint		blank:1 ;
} ;

struct state {
	STATE_FL	f ;
	int		dummy ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procextras(PROGINFO *) ;
static int	procmsmap(PROGINFO *,OPTIONS *,cchar *) ;
static int	procmsmaper(PROGINFO *,OPTIONS *,cchar *) ;
static int	procmsmapline(PROGINFO *,char *,cchar *,int) ;
static int	procloadsched(PROGINFO *,vecstr *) ;

static int	procout_begin(PROGINFO *,OPTIONS *,void *,cchar *) ;
static int	procout_end(PROGINFO *,OPTIONS *,void *) ;
static int	procout_post(PROGINFO *,OPTIONS *,bfile *,bfile *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,OPTIONS *,void *,cchar *) ;
static int	procfile(PROGINFO *,OPTIONS *,bfile *,const char *,int) ;
static int	procfiler(PROGINFO *,bfile *,bfile *) ;
static int	procline(PROGINFO *,bfile *,STATE *, cchar *,int) ;

static int	msmapdef(char *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"mf",
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
	argopt_mf,
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
	{ 0, 0 }
} ;

static cchar	*akonames[] = {
	"inplace",
	"lower",
	"doublespace",
	"half",
	"leading",
	"trailing",
	"oneblank",
	"noblanks",
	"mscrap",
	"pad",
	"snug",
	"compact",
	"subnbsp",
	NULL
} ;

enum akonames {
	akoname_inplace,
	akoname_lower,
	akoname_double,
	akoname_half,
	akoname_leading,
	akoname_trailing,
	akoname_oneblank,
	akoname_noblanks,
	akoname_mscrap,
	akoname_pad,
	akoname_snug,
	akoname_compact,
	akoname_subnbsp,
	akoname_overlast
} ;

static cchar	*msmaps[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static cchar	selfchars[] = {
	CH_TAB,
	CH_CR,
	CH_NL,
	CH_FF,
	CH_VT,
	0
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	struct options	opts ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*mfname = NULL ;
	cchar		*cp ;

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

/* initialize options */

	pip->config = &opts ;
	memset(&opts,0,sizeof(struct options)) ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
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

/* argument list file */
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

/* get an output file name other than using STDOUT! */
	                case argopt_of:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        ofname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* MS-map file */
	                case argopt_mf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            mfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                mfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

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

	                    case 'h':
	                        opts.f.half = TRUE ;
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
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

	            } /* end if */

	        } /* end if (individual option key letters) */

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
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    struct options	*opp = pip->config ;
	    const char		*pn = pip->progname ;
	    bfile		*efp = pip->efp ;
	    bprintf(efp,"%s: inplace=%u\n",pn,opp->f.inplace) ;
	    bprintf(efp,"%s: lower=%u\n",pn,opp->f.lower) ;
	    bprintf(efp,"%s: double=%u\n",pn,opp->f.doublespace) ;
	    bprintf(efp,"%s: half=%u\n",pn,opp->f.half) ;
	    bprintf(efp,"%s: oneblank=%u\n",pn,opp->f.oneblank) ;
	    bprintf(efp,"%s: noblank=%u\n",pn,opp->f.noblanks) ;
	    bprintf(efp,"%s: leading=%u\n",pn,opp->f.rmleading) ;
	    bprintf(efp,"%s: trailing=%u\n",pn,opp->f.rmtrailing) ;
	    bprintf(efp,"%s: mscrap=%u\n",pn,opp->f.mscrap) ;
	} /* end if (run options) */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procextras(pip)) >= 0) {
		OPTIONS		*opp = (OPTIONS *) pip->config ;
	        if ((rs = procmsmap(pip,opp,mfname)) >= 0) {
	            bfile	ofile, *ofp = &ofile ;
	            if ((rs = procout_begin(pip,&opts,ofp,ofname)) >= 0) {
	                {
			    ARGINFO	*aip = &ainfo ;
			    BITS	*bop = &pargs ;
			    cchar	*afn = afname ;
	                    rs = process(pip,aip,bop,&opts,ofp,afn) ;
	                }
	                rs1 = procout_end(pip,&opts,ofp) ;
	                if (rs >= 0) rs = rs1 ;
		    } else {
			cchar	*pn = pip->progname ;
			cchar	*fmt = "%s: inaccessible output (%d)\n" ;
			bprintf(pip->efp,fmt,pn,rs) ;
	            } /* end if (procout) */
	        } /* end if (procmsmap) */
	    } /* end if (procextras) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done rs=%d\n",rs) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    const char	*fmt = NULL ;
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOINPUT ;
	        fmt = "%s: file not found (%d)\n" ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	    if ((! pip->f.quiet) && (fmt != NULL)) {
	        bprintf(pip->efp,fmt,pip->progname,rs) ;
	    }
	} /* end if (error) */

/* we are out of here */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
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
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff comes here */
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
	int     	rs = SR_OK ;
	int     	wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file(s)>] [-af <afile>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-h]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	if (rs >= 0) rs = bprintf(pip->efp,"%s:\n", pn) ;
	if (rs >= 0) wlen += rs ;

	fmt = "%s:  -h                           half space where possible\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -o option[,options,...]      processing options\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	if (rs >= 0) rs = bprintf(pip->efp,"%s:\n",pn) ;
	wlen += rs ;

	fmt = "%s:  processing options include:\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  inplace                      modify file \"in place\"\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  lower                        convert to lower case\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  double                       double space\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  half                         half space where possible\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  oneblank                     only allow one blank line\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  noblanks                     strip blank lines\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  leading                      string leading white space\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  trailing                     strip trailing white space\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  mscrap                       remove M$ crap\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  pad=<n>                      path to <n> characters\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  snug                         remove extra white-space\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	struct options	*opp = pip->config ;
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
	                case akoname_inplace:
	                    opp->f.inplace = TRUE ;
	                    break ;
	                case akoname_lower:
	                    opp->f.lower = TRUE ;
	                    break ;
	                case akoname_double:
	                    opp->f.doublespace = TRUE ;
	                    break ;
	                case akoname_half:
	                    opp->f.half = TRUE ;
	                    break ;
	                case akoname_oneblank:
	                    opp->f.oneblank = TRUE ;
	                    break ;
	                case akoname_noblanks:
	                    opp->f.noblanks = TRUE ;
	                    break ;
	                case akoname_leading:
	                    opp->f.rmleading = TRUE ;
	                    break ;
	                case akoname_trailing:
	                    opp->f.rmtrailing = TRUE ;
	                    break ;
	                case akoname_mscrap:
	                    opp->f.mscrap = TRUE ;
	                    break ;
	                case akoname_pad:
	                    opp->f.pad = TRUE ;
	                    if (vl > 0) {
	                        rs = optvalue(vp,vl) ;
	                        opp->pad = rs ;
	                    }
	                    break ;
	                case akoname_snug:
	                case akoname_compact:
	                    opp->f.snug = TRUE ;
	                    break ;
	                case akoname_subnbsp:
	                    opp->f.subnbsp = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        opp->f.subnbsp = (rs > 0) ;
	                    }
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


static int procextras(PROGINFO *pip)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	        cchar	**vpp = &pip->username ;
	        if ((rs = proginfo_setentry(pip,vpp,pw.pw_name,-1)) >= 0) {
		    vpp = &pip->homedname ;
	            rs = proginfo_setentry(pip,vpp,pw.pw_dir,-1) ;
	        }
	    } /* end if (getpwusername) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */
	return rs ;
}
/* end subroutine (procextras) */


static int procmsmap(PROGINFO *pip,OPTIONS *opp,cchar *mfn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if ((opp != NULL) && opp->f.mscrap) {
	    if (mfn != NULL) {
	        rs = procmsmaper(pip,opp,mfn) ;
	    } else {
	        vecstr		cs ;
	        if ((rs = vecstr_start(&cs,2,0)) >= 0) {
	            if ((rs = procloadsched(pip,&cs)) >= 0) {
		        const int	tlen = MAXPATHLEN ;
		        cchar		**spp = msmaps ;
		        char		tbuf[MAXPATHLEN+1] ;
		        mfn = TEXTCLEAN_MSMAP ;
		        if ((rs = permsched(spp,&cs,tbuf,tlen,mfn,R_OK)) >= 0) {
	    		    rs = procmsmaper(pip,opp,tbuf) ;
		        } else if (isNotPresent(rs)) {
		            rs = SR_OK ;
			}
	            } /* end if (procloadsched) */
	            rs1 = vecstr_finish(&cs) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecstr) */
	    } /* end if (msmap-file) */
	} /* end if (mscrap) */
	return rs ;
}
/* end subroutine (procmsmap) */


static int procmsmaper(PROGINFO *pip,OPTIONS *opp,cchar *mfn)
{
	bfile		mfile, *mfp = &mfile ;
	int		rs ;
	int		rs1 ;
	if ((rs = bopen(mfp,mfn,"r",0666)) >= 0) {
	    const int	size = 256 ;
	    char	*bp ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: mapfile=%s\n" ;
		bprintf(pip->efp,fmt,pn,mfn) ;
	    }
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		char		lbuf[LINEBUFLEN+1] ;
		pip->msmap = bp ;
		msmapdef(bp) ;
		while ((rs = breadline(mfp,lbuf,llen)) > 0) {
		    len = rs ;
		    if (lbuf[len-1] == '\n') len -= 1 ;
		    if ((len > 0) && (lbuf[0] != '#')) {
		        rs = procmsmapline(pip,bp,lbuf,len) ;
		    }
		    if (rs < 0) break ;
		} /* end while (reading lines) */
		if (rs < 0) {
		    uc_free(bp) ;
		    pip->msmap = NULL ;
		}
	    } /* end if (m-a) */
	    rs1 = bclose(mfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (MS-file) */
	return rs ;
}
/* end subroutine (procmsmaper) */


static int procmsmapline(PROGINFO *pip,char *mp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	cchar		*tp ;
	if (pip == NULL) return SR_FAULT ;
	if ((tp = strnchr(sp,sl,'#')) != NULL) {
	    sl = (tp-sp) ;
	}
	if (sl > 0) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfnext(sp,sl,&cp)) > 0) {
		int	kch ;
		if ((rs = cfnumi(cp,cl,&kch)) >= 0) {
		    sl -= (cp+cl+1-sp) ;
		    sp = (cp+cl+1) ;
		    if ((cl = sfnext(sp,sl,&cp)) > 0) {
			int	v ;
			if (cl == 1) {
			    mp[kch] = cp[0] ;
			} else if ((rs = cfnumi(cp,cl,&v)) >= 0) {
			    mp[kch] = v ;
			}
		    }
		}
	    }
	} /* end if (strnchr) */
	return rs ;
}
/* end subroutine (procmsmapline) */


static int procloadsched(PROGINFO *pip,vecstr *svp)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	    const char	*keys = "phen" ;
	    for (i = 0 ; keys[i] != '\0' ; i += 1) {
		const int	kc = MKCHAR(keys[i]) ;
		const char	*vp = NULL ;
		int		vl = -1 ;
	 	switch (kc) {
		case 'p':
		    vp = pip->pr ;
		    break ;
		case 'h':
		    vp = pip->homedname ;
		    break ;
		case 'e':
		    vp = "etc" ;
		    break ;
		case 'n':
		    vp = pip->searchname ;
		    break ;
		} /* end switch */
		if (vp != NULL) {
		    char	kbuf[2] = "x" ;
		    kbuf[0] = kc ;
	            rs = vecstr_envadd(svp,kbuf,vp,vl) ;
		    if (rs < INT_MAX) c += 1 ;
		}
		if (rs < 0) break ;
	    } /* end for */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadsched) */


static int procout_begin(PROGINFO *pip,OPTIONS *optp,void *ofp,cchar *ofn)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (optp->f.inplace) {
	    rs = bopentmp(ofp,NULL,"rwct",0666) ;
	} else {
	    if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-')) {
	        ofn = BFILE_STDOUT ;
	    }
	    rs = bopen(ofp,ofn,"wct",0666) ;
	} /* end if (opening the output file) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procout_begin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,OPTIONS *optp,void *ofp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	rs1 = bclose(ofp) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (procout_end) */


static int procout_post(PROGINFO *pip,OPTIONS *optp,bfile *ofp,bfile *ifp)
{
	int		rs = SR_OK ;
	int		clen = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procout_post: f_inplace=%u\n",optp->f.inplace) ;
#endif
	if (optp->f.inplace) {
	    if ((rs = brewind(ofp)) >= 0) {
		if ((rs = brewind(ifp)) >= 0) {
		    if ((rs = bwriteblock(ifp,ofp,-1)) >= 0) {
			offset_t	toff = rs ;
			clen = rs ;
			if ((rs = btruncate(ifp,toff)) >= 0) {
	    		    rs = brewind(ofp) ;
			}
		    }
		}
	    }
	} /* end if (in-place) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procout_post: ret rs=%d clen=%d\n",rs,clen) ;
#endif
	return (rs >= 0) ? clen : rs ;
}
/* end subroutine (procout_post) */


static int process(pip,aip,bop,optp,ofp,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
OPTIONS		*optp ;
void		*ofp ;
const char	*afn ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		wlen = 0 ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ent\n") ;
#endif

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procfile(pip,optp,ofp,cp,-1) ;
	                wlen += rs ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through requested circuits) */
	} /* end if (ok) */

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
	                    rs = procfile(pip,optp,ofp,cp,cl) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: file=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procfile(pip,optp,ofp,cp,1) ;
	    wlen += rs ;

	} /* end if (program invocation arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procfile(PROGINFO *pip,OPTIONS *opp,bfile *ofp,cchar *fp,int fl)
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*fname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfile: f=%t\n",fp,fl) ;
#endif

	if ((fp == NULL) || (fp[0] == '\0') || (fp[0] == '-')) {
	    fp = BFILE_STDIN ;
	    fl = -1 ;
	}

	if ((rs = nulstr_start(&n,fp,fl,&fname)) >= 0) {
	    bfile	ifile, *ifp = &ifile ;
	    cchar	*ostr = (opp->f.inplace) ? "rw" : "r" ;
	    if ((rs = bopen(ifp,fname,ostr,0666)) >= 0) {
	        if ((rs = procfiler(pip,ifp,ofp)) >= 0) {
	            wlen += rs ;
		    rs = procout_post(pip,opp,ofp,ifp) ;
	        }
	        rs1 = bclose(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (file-open) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procfiler(PROGINFO *pip,bfile *ifp,bfile *ofp)
{
	struct options	*opp = pip->config ;
	STATE		fs ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		len ;
	int		ll ;
	int		fo = 0 ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN+1] ;

	if (opp == NULL) return SR_FAULT ;

	memset(&fs,0,sizeof(STATE)) ;

	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;
	    ll = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procfiler: breadline() rs=%d\n",rs) ;
	        debugprintf("procfiler: fo=%u c1=%02x c2=%02x\n",
			fo,lbuf[len-1],lbuf[len-2]) ;
	    }
#endif

	    while (ll && iseol(lbuf[ll-1])) {
	        ll -= 1 ;
	    }

	    rs = procline(pip,ofp,&fs,lbuf,ll) ;
	    wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procfiler: procline() rs=%d\n",rs) ;
#endif

	    fo += len ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfiler: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfiler) */


static int procline(PROGINFO *pip,bfile *ofp,STATE *fsp,cchar *lbuf,int ll)
{
	struct options	*opp = pip->config ;
	const int	olen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		ol = 0 ;
	int		wlen = 0 ;
	int		f_print = TRUE ;
	char		obuf[LINEBUFLEN+1] ;

	if (fsp == NULL) return SR_FAULT ;

	obuf[0] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procline: ll=%u l=>%t<\n",
	        ll,lbuf,strlinelen(lbuf,ll,40)) ;
#endif

	if (opp->f.rmleading) {
	    while (ll && CHAR_ISWHITE(lbuf[0])) {
		lbuf += 1 ;
		ll -= 1 ;
	    }
	}

	if ((ll == 0) && (lbuf[0] == 0x04)) {
	    f_print = FALSE ;
	}

	if (opp->f.mscrap && (pip->msmap != NULL)) {
	    int		kch ;
	    int		rch ;
	    for (i = 0 ; (i < ll) && (ol < olen) ; i += 1) {
	        kch = MKCHAR(lbuf[i]) ;
		rch = MKCHAR(pip->msmap[kch]) ;
		if (rch) obuf[ol++] = rch ;
	    } /* end for */
	} else {
	    for (i = 0 ; (i < ll) && (ol < olen) ; i += 1) {
	        obuf[ol++] = lbuf[i] ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procline: ol=%u\n",ol) ;
#endif

	if (opp->f.snug && ol) {
	    int		c = 0 ;
	    int		sl = ol ;
	    int		cl ;
	    cchar	*sp = obuf ;
	    cchar	*cp ;
	    ol = 0 ;
	    while ((cl = nextfield(sp,sl,&cp)) > 0) {
	        if (c++ > 0) obuf[ol++] = ' ' ;
	        strncpy((obuf+ol),cp,cl) ;
	        ol += cl ;
	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;
	    } /* end while */
	} /* end if (snug) */

	if (opp->f.rmtrailing && ol) {
	    while (ol && CHAR_ISWHITE(obuf[ol-1])) ol -= 1 ;
	}

	if (opp->f.subnbsp && ol) {
	    int		i ;
	    for (i = 0 ; i < ol ; i += 1) {
	        if (obuf[i] == CH_SP) obuf[i] = CH_NBSP ;
	    } /* end for */
	}

	if (opp->pad > 0) {
	    while ((ol < olen) && (ol < opp->pad)) obuf[ol++] = ' ' ;
	}

	if (f_print) {
	        if ((ol == 0) || hasallwhite(obuf,ol)) {
	            if (opp->f.half && (! fsp->f.blank)) {
	                f_print = FALSE ;
	            }
	            fsp->f.blank = TRUE ;
	        } else {
	            fsp->f.blank = FALSE ;
	        }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procline: ol=%u f_print=%u\n",ol,f_print) ;
	    debugprintf("procline: o=>%t<\n",obuf,ol) ;
	}
#endif

	if (f_print) {
	    rs = bprintline(ofp,obuf,ol) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && opp->f.doublespace) {
	    rs = bprintline(ofp,obuf,0) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procline: ret rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


static int msmapdef(char *bp)
{
	int		i ;
	for (i = 0 ; i < 32 ; i += 1) {
	    bp[i] = '¿' ;
	}
	for ( ; i < 256 ; i += 1) {
	    bp[i] = i ;
	}
	{
	    int		ch ;
	    cchar	*scp = selfchars ;
	    while ((ch = MKCHAR(*scp++)) > 0) {
		bp[ch] = ch ;
	    }
	}
	bp[CH_DEL] = '¿' ;
	return SR_OK ;
}
/* end subroutine (msmapdef) */


