/* main (sha1) */
/* lang=C++98 */

/* this SHA1 object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_LOCSETENT	0		/* |locinfo_setent()| */


/* revision history:

	= 2000-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ sha1 [<file(s)> ...]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<new>
#include	<algorithm>
#include	<functional>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<sha1.h>
#include	<vecstr.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* name spaces (default) */

using namespace	std ;


/* external subroutines */

extern "C" int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern "C" int	mkpath2(char *,cchar *,cchar *) ;
extern "C" int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern "C" int	matstr(cchar **,cchar *,int) ;
extern "C" int	matostr(cchar **,int,cchar *,int) ;
extern "C" int	vstrcmp(const void *,const void *) ;
extern "C" int	vstrkeycmp(const void *,const void *) ;
extern "C" int	cfdeci(cchar *,int,int *) ;
extern "C" int	cfdecui(cchar *,int,uint *) ;
extern "C" int	cfdecti(cchar *,int,int *) ;
extern "C" int	cthexstr(char *,int,cchar *,int) ;
extern "C" int	optbool(cchar *,int) ;
extern "C" int	optvalue(cchar *,int) ;
extern "C" int	isdigitlatin(int) ;
extern "C" int	isFailOpen(int) ;
extern "C" int	isNotPresent(int) ;

extern "C" int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern "C" int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugprinthex(cchar *,int,cchar *,int) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;
extern "C" char	*strnrchr(cchar *,int,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		cvtcase:1 ;
	uint		cvtuc:1 ;
	uint		cvtlc:1 ;
	uint		cvtfc:1 ;
	uint		outer:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	int		to ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procsetcase(PROGINFO *,cchar *,int) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procfile(PROGINFO *,bfile *,cchar *) ;
static int	procout(PROGINFO *,bfile *,SHA1 *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif


/* local variables */

static cchar *argopts[] = {
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

static cchar *akonames[] = {
	"cvtcase",
	"cc",
	"casecvt",
	"bufwhole",
	"bufline",
	"bufnone",
	"whole",
	"line",
	"none",
	"un",
	"",
	NULL
} ;

enum akonames {
	akoname_cvtcase,
	akoname_cc,
	akoname_casecvt,
	akoname_bufwhole,
	akoname_bufline,
	akoname_bufnone,
	akoname_whole,
	akoname_line,
	akoname_none,
	akoname_un,
	akoname_empty,
	akoname_overlast
} ;

static cchar	*cases[] = {
	"upper",
	"lower",
	"fold",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

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

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*tos_open = NULL ;
	cchar		*tos_read = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("sha1: starting DFD=%u\n",rs) ;
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

#if	CF_DEBUGS
	debugprintf("sha1: args\n") ;
#endif

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
	        const int ach = MKCHAR(argp[1]) ;

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
	                            if (argl) {
	                                tos_open = argp ;
	                                tos_read = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* line-buffered */
	                    case 'u':
	                        pip->have.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
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

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("sha1: args-out rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("sha1: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: version %s\n",
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("sha1: pr=%s\n",pip->pr) ;
	    debugprintf("sha1: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    rs = procargs(pip,aip,bop,ofn,afn) ;
	} else if (ex == EX_OK) {
	    bfile	*efp = (bfile *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    default:
	        if (! pip->f.quiet) {
	    	    bfile	*efp = (bfile *) pip->efp ;
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: could not process (%d)\n" ;
	            bprintf(efp,fmt,pn,rs) ;
	        }
		break ;
	    case SR_PIPE:
		break ;
	    } /* end switch */
	    ex = mapex(mapexs,rs) ;
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("sha1: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bfile	*efp = (bfile *) pip->efp ;
	    pip->open.errfile = FALSE ;
	    bclose(efp) ;
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
	    debugprintf("sha1: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	{
	    bfile	*efp = (bfile *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument specified (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(efp,fmt,pn,rs) ;
	    usage(pip) ;
	}
	goto retearly ;

}
/* end subroutine (mainsub) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	bfile		*efp = (bfile *) pip->efp ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<files(s)> ...] [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = bprintf(efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-to <to_open>] [-tr <to_read>]\n" ;
	if (rs >= 0) rs = bprintf(efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

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
	                case akoname_cvtcase:
	                case akoname_casecvt:
	                case akoname_cc:
	                    if (! lip->final.cvtcase) {
	                        lip->have.cvtcase = TRUE ;
	                        lip->final.cvtcase = TRUE ;
	                        lip->f.cvtcase = TRUE ;
	                        if (vl > 0) {
	                            rs = procsetcase(pip,vp,vl) ;
				}
	                    }
	                    break ;
	                case akoname_bufwhole:
	                case akoname_whole:
	                    if (! pip->final.bufwhole) {
	                        pip->have.bufwhole = TRUE ;
	                        pip->final.bufwhole = TRUE ;
	                        pip->f.bufwhole = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufwhole = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufline:
	                case akoname_line:
	                    if (! pip->final.bufline) {
	                        pip->have.bufline = TRUE ;
	                        pip->final.bufline = TRUE ;
	                        pip->f.bufline = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufline = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_bufnone:
	                case akoname_none:
	                case akoname_un:
	                    if (! pip->final.bufnone) {
	                        pip->have.bufnone = TRUE ;
	                        pip->final.bufnone = TRUE ;
	                        pip->f.bufnone = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.bufnone = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_empty:
	                    break ;
	                default:
	                    rs = SR_INVALID ;
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


static int procsetcase(PROGINFO *pip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		ci ;

	if ((ci = matostr(cases,1,vp,vl)) >= 0) {
	    LOCINFO	*lip = (LOCINFO *) pip->lip ;
	    const int	ch = CHAR_TOLC(vp[0]) ;
	    switch (ch) {
	    case 'l':
	        lip->f.cvtlc = TRUE ;
	        break ;
	    case 'u':
	        lip->f.cvtuc = TRUE ;
	        break ;
	    case 'f':
	        lip->f.cvtfc = TRUE ;
	        break ;
	    } /* end switch */
	    if (pip->debuglevel > 0) {
	        bfile	*efp = (bfile *) pip->efp ;
	        cchar	*pn = pip->progname ;
	        bprintf(efp,"%s: conversion=%u\n",pn,ci) ;
	    }
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (procsetcase) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    LOCINFO	*lip = (LOCINFO *) pip->lip ;
	        int	pan = 0 ;
	        cchar	*cp ;

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
	                        rs = procfile(pip,ofp,cp) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (ok) */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            bfile	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	            if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if (len > 0) {
	                        pan += 1 ;
	                        rs = procfile(pip,ofp,lbuf) ;
	                        wlen += rs ;
	                    }

	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = bclose(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
	    		    bfile	*efp = (bfile *) pip->efp ;
			    fmt = "%s: inaccessible argument-list (%d)\n" ;
	                    bprintf(efp,fmt,pn,rs) ;
	                    bprintf(efp,"%s: afile=%s\n",pn,afn) ;
	                }
	            } /* end if */

	        } /* end if (procesing file argument file list) */

	        if ((rs >= 0) && (pan == 0)) {

	            cp = "-" ;
	            pan += 1 ;
	            rs = procfile(pip,ofp,cp) ;
	            wlen += rs ;

	        } /* end if (standard-input) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bfile	*efp = (bfile *) pip->efp ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(efp,fmt,pn,rs) ;
	    bprintf(efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	if ((pip->debuglevel > 0) && (rs >= 0)) {
	    bfile	*efp = (bfile *) pip->efp ;
	    fmt = "%s: written=%u\n" ;
	    bprintf(efp,fmt,pn,wlen) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


/* process a file */
static int procfile(PROGINFO *pip,bfile *ofp,cchar *fname)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	SHA1		d ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("sha1/procfile: fname=%s\n",fname) ;
	    debugprintf("sha1/procfile: f_outer=%u\n",lip->open.outer) ;
	}
#endif

	if (fname == NULL) return SR_FAULT ;

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0)) {
	    fname = STDINFNAME ;
	}

	if ((rs = sha1_start(&d)) >= 0) {
	    bfile	infile, *ifp = &infile ;

	    if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		char		lbuf[LINEBUFLEN + 1] ;

		while ((rs = breadline(ifp,lbuf,llen)) > 0) {

	                rs = sha1_update(&d,lbuf,rs) ;

		    if (rs < 0) break ;
		} /* end while */

	        rs1 = bclose(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file-output) */

	    if (rs >= 0) {
		rs = procout(pip,ofp,&d) ;
		wlen = rs ;
	    }

	    sha1_finish(&d) ;
	} /* end if (sha1) */

	if (pip->debuglevel > 0) {
	    bfile	*efp = (bfile *) pip->efp ;
	    const int	v = ((rs >= 0) ? wlen : rs) ;
	    cchar	*pn = pip->progname ;
	    bprintf(efp,"%s: file=%s (%d)\n",pn,fname,v) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procout(PROGINFO *pip,bfile *ofp,SHA1 *sop)
{
	const int	m = 20 ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	uchar		*digest ;

	if ((digest = new(nothrow) uchar[m]) != NULL) {
	    if ((rs = sha1_digest(sop,digest)) >= 0) {
		const int	olen = (m*3) ;
		char		*obuf ;
		if ((obuf = new(nothrow) char[(olen+1)]) != NULL) {
		    cchar	*mp = (cchar *) digest ;
		    if ((rs = cthexstr(obuf,olen,mp,m)) >= 0) {
		        rs = bprintln(ofp,obuf,rs) ;
			wlen += rs ;
		    }
		    delete [] obuf ;
		} else {
		    rs = SR_NOMEM ;
		}
	    } /* end if */
	    delete [] digest ;
	} else {
	    rs = SR_NOMEM ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*varterm = VARTERM ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

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
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
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


