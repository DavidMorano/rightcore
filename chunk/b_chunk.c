/* b_chunk (KSH builtin) */

/* program to chunk-a-size up a large file into smaller ones */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_LOCSETENT	0		/* allow |locinfo_setentry()| */


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This program was originally written (as a tutorial exercise
	for a software guy!).

	= 1998-02-01, David A­D­ Morano
	This program was enhanced slightly to adopt the new
	invocation style program argument/options.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program is used to binary-chunkasize files into parts.

	Synopsis:

	$ chunk [-V] [-p <prefix>] [-<size>] [<file(s)>]


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
#include	<char.h>
#include	<ascii.h>
#include	<cfdec.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_chunk.h"
#include	"defs.h"


/* local defines */

#ifndef	STAGEBUFLEN
#define	STAGEBUFLEN	(8 * 1024 * 1024)
#endif

#define	MINCHUNK	1

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)

#ifndef	BCEIL		/* just a little honey of ours */
#define	BCEIL(v,m)	(((v) + ((m) - 1)) & (~ ((m) - 1)))
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROCOUT		struct procout


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	readignore(int,offset_t) ;
extern int	bufprintf(char *,int,const char *,...) ;
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

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

#if	CF_MAKEDATE
extern cchar	chunk_makedate[] ;
#endif

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*prefix ;
	cchar		*suffix ;
	offset_t	chunklen ;
	offset_t	off_begin ;
	offset_t	off_end ;
	int		ps ;
	int		acount ;
} ;

struct procout {
	offset_t	lenrem ;
	int		ofd ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

#if	CF_MAKEDATE
static int	makedate_get(const char *,const char **) ;
#endif /* CF_MAKEDATE */

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procfile(PROGINFO *,PROCOUT *,cchar *) ;
static int	procdata(PROGINFO *,PROCOUT *,int) ;

static int	procout_begin(PROGINFO *,PROCOUT *) ;
static int	procout_write(PROGINFO *,PROCOUT *,cchar *,int) ;
static int	procout_end(PROGINFO *,PROCOUT *) ;
static int	procout_open(PROGINFO *,PROCOUT *) ;
static int	procout_close(PROGINFO *,PROCOUT *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_defaults(LOCINFO *,cchar *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"DEBUG",
	"VERBOSE",
	"VV",
	"HELP",
	"sn",
	"af",
	"ef",
	"ob",
	"oe",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_debug,
	argopt_verbose,
	argopt_vv,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_ob,
	argopt_oe,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
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
	{ SR_NOEXEC, EX_NOEXEC },
	{ SR_LIBACC, EX_NOEXEC },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int b_chunk(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_chunk) */


int p_chunk(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_chunk) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	SHIO		errfile, *efp = &errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_USAGE ;
	int		f_usage = FALSE ;
	int		f_optplus = FALSE ;
	int		f_optminus = FALSE ;
	int		f_optequal = FALSE ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
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

/* other initialization */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    break ;

	                case argopt_vv:
	                    f_makedate = TRUE ;
	                    break ;

/* debug level */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search name */
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

/* beginning offset */
	                case argopt_ob:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            LONG	lw ;
	                            rs = cfdecmfll(argp,argl,&lw) ;
	                            lip->off_begin = lw ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* ending offset */
	                case argopt_oe:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            LONG	lw ;
	                            rs = cfdecmfll(argp,argl,&lw) ;
	                            lip->off_end = lw ;
	                        }
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

/* starting part count */
	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                int	v ;
	                                rs = cfdecmfi(argp,argl,&v) ;
	                                lip->acount = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* beginning offset */
	                    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                LONG	lw ;
	                                rs = cfdecmfll(argp,argl,&lw) ;
	                                lip->off_begin = lw ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* ending offset */
	                    case 'e':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                LONG	lw ;
	                                rs = cfdecmfll(argp,argl,&lw) ;
	                                lip->off_end = lw ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* chunk file prefix */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->prefix = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* chunk file suffix */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->suffix = argp ;
	                            }
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
#endif

	if (f_version + f_makedate) {
	    shio_printf(efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

#if	CF_MAKEDATE
	if (f_makedate) {
	    makedate_get(chunk_makedate,&cp) ;
	    shio_printf(efp,"%s: makedate %s\n",pip->progname,cp) ;
	}
#endif /* CF_MAKEDATE */

/* program search name */

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

	if (f_usage)
	    usage(pip) ;

/* help file */

#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

	if (f_usage || f_help || f_version || f_makedate)
	    goto retearly ;


	ex = EX_OK ;

/* check argument defaults */

	rs = locinfo_defaults(lip,argval) ;

/* go */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: open files\n") ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,afn) ;
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
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	}

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
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
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
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad things come here */
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
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<file(s)>] [-af <afile>] [-<size>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-ob <offbegin>] [-oe <offend>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p <prefix>] [-s <suffix>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	PROCOUT		po ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = procout_begin(pip,&po)) >= 0) {
	    int		pan = 0 ;
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
	                    rs = procfile(pip,&po,cp) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                int	len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
			    if (cp[0] != '#') {
			        lbuf[(cp+cl)-lbuf] = '\0' ;
	                        pan += 1 ;
	                        rs = procfile(pip,&po,cp) ;
	                        wlen += rs ;
			    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
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

	    } /* end if (procesing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

		cp = "-" ;
	        pan += 1 ;
	        rs = procfile(pip,&po,cp) ;
	        wlen += rs ;

	    } /* end if (standard-input) */

	    rs1 = procout_end(pip,&po) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procout) */

	if ((pip->debuglevel > 0) && (rs >= 0)) {
	    fmt = "%s: written=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,wlen) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procfile(PROGINFO *pip,PROCOUT *pop,cchar *fn)
{
	int		rs = SR_OK ;
	int		ifd = FD_STDIN ;
	int		f_notstdin = TRUE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((fn == NULL) || (fn[0] == '\0') || (strcmp(fn,"-") == 0)) {
	    fn = STDINFNAME ;
	    f_notstdin = FALSE ;
	}

	if (pip->debuglevel > 0) {
	    fmt = "%s: file=%s\n" ;
	    shio_printf(pip->efp,pn,fn) ;
	}

	if (f_notstdin) {
	    rs = uc_open(fn,O_RDONLY,0666) ;
	    ifd = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: we opened the input file rs=%d\n",rs) ;
#endif

	if (rs >= 0) { /* opened input */
	    rs = procdata(pip,pop,ifd) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_chunk: procdata() rs=%d\n",rs) ;
#endif

	    if (f_notstdin) u_close(ifd) ;
	} else {
	    cchar	*pn = pip->progname ;
	    fmt = "%s: inaccessible input (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	} /* end if (opened input) */

	return rs ;
}
/* end subroutine (procfile) */


static int procdata(PROGINFO *pip,PROCOUT *pop,int ifd)
{
	struct ustat	sb ;
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_chunk/procdata: ent\n") ;
#endif

	if ((rs = u_fstat(ifd,&sb)) >= 0) {
	    int		rlen = (2*1024) ;
	    char	*rbuf ;
	    if (S_ISREG(sb.st_mode) && (sb.st_size > 0)) {
	        size_t	msize = (size_t) MIN(STAGEBUFLEN,sb.st_size) ;
	        rlen = BCEIL(msize,lip->ps) ;
	    }
	    if ((rs = uc_valloc(rlen,&rbuf)) >= 0) {
	        offset_t	inoff = 0 ;
	        offset_t	eoff = lip->off_end ;

	        if (lip->off_begin > 0) {
	            offset_t	soff = lip->off_begin ;
	            if (S_ISREG(sb.st_mode)) {
	                rs = u_seek(ifd,soff,SEEK_SET) ;
	            } else {
	                rs = readignore(ifd,soff) ;
	            }
	            inoff += soff ;
	        } /* end if (starting offset specified) */

	        if (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_chunk/procdata: procout-ing\n") ;
#endif

	            while ((rs >= 0) && ((eoff < 0) || (inoff < eoff))) {
	                int	mlen ;
	                int	rl ;

	                if (eoff > 0) {
	                    mlen = MIN(rlen,(eoff - inoff)) ;
	                } else {
	                    mlen = rlen ;
	                }

	                if ((rs = u_read(ifd,rbuf,mlen)) > 0) {
	                    rl = rs ;
	                    inoff += rl ;
	                    rs = procout_write(pip,pop,rbuf,rl) ;
	                    wlen += rs ;
	                } else if (rs == 0) {
	                    rl = 0 ;
	                    break ;
	                }

	            } /* end while */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_chunk/procdata: while-end\n") ;
#endif

	        } /* end if (ok) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_chunk/procdata: mid rs=%d wlen=%u\n",
			rs,wlen) ;
#endif

	        uc_free(rbuf) ;
	    } /* end if (m-a-f) */
	} /* end if (fstat) */
	wlen &= INT_MAX ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_chunk/procdata: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdata) */


static int procout_begin(PROGINFO *pip,PROCOUT *pop)
{

	if (pip == NULL) return SR_FAULT ;
	if (pop == NULL) return SR_FAULT ;

	memset(pop,0,sizeof(PROCOUT)) ;
	pop->ofd = -1 ;

	return SR_OK ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,PROCOUT *pop)
{
	int		rs ;

	rs = procout_close(pip,pop) ;

	return rs ;
}
/* end subroutine (procout_end) */


static int procout_write(PROGINFO *pip,PROCOUT *pop,cchar *obuf,int olen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	while ((rs >= 0) && (olen > 0)) {
	    if (pop->lenrem == 0) {
	        rs = procout_open(pip,pop) ;
	    }
	    if (rs >= 0) {
	        int	mlen = MIN(olen,pop->lenrem) ;
	        rs = uc_writen(pop->ofd,(obuf+wlen),mlen) ;
	        wlen += rs ;
	        pop->lenrem -= rs ;
	        olen -= rs ;
	        if ((rs >= 0) && (pop->lenrem == 0)) {
	            rs1 = procout_close(pip,pop) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    }
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout_write) */


static int procout_open(PROGINFO *pip,PROCOUT *pop)
{
	LOCINFO		*lip = pip->lip ;
	const int	olen = MAXPATHLEN ;
	int		rs ;
	int		acount ;
	const char	*pre ;
	const char	*suf ;
	const char	*fmt ;
	const char	*cp ;
	char		obuf[MAXPATHLEN+1] ;

	{
	    acount = lip->acount ;
	    pre = lip->prefix ;
	    suf = lip->suffix ;
	}

	fmt = "%t%03u%s%t" ;
	cp = ((suf[0] != '\0') ? "." : "") ;
	if ((rs = bufprintf(obuf,olen,fmt,pre,11,acount,cp,suf,11)) >= 0) {
	    if ((rs = u_open(obuf,O_FLAGS,0666)) >= 0) {
	        pop->ofd = rs ;
	        pop->lenrem = lip->chunklen ;
	        lip->acount += 1 ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procout_open: ret rs=%d ofd=%u\n",rs,pop->ofd) ;
#endif

	return rs ;
}
/* end subroutine (procout_open) */


static int procout_close(PROGINFO *pip,PROCOUT *pop)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	if (pop->ofd >= 0) {
	    rs1 = u_close(pop->ofd) ;
	    if (rs >= 0) rs = rs1 ;
	    pop->ofd = -1 ;
	}

	pop->lenrem = 0 ;
	return rs ;
}
/* end subroutine (procout_close) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->ps = getpagesize() ;
	lip->off_end = -1 ; /* default is all */

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


static int locinfo_defaults(LOCINFO *lip,cchar *argval)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: chunklen=%d\n", lip->chunklen) ;
	    debugprintf("main: prefix=%s suffix=%s\n",
	        lip->prefix,lip->suffix) ;
	}
#endif /* CF_DEBUG */

	if ((lip->chunklen == 0) && (argval != NULL)) {
	    LONG	lw ;
	    if ((rs = cfdecmfll(argval,-1,&lw)) >= 0) {
	        lip->chunklen = lw ;
	    }
	}

	if (lip->chunklen == 0) lip->chunklen = DEFCHUNKLEN ;

	if (lip->acount < 0) lip->acount = 0 ;

	if (lip->prefix == NULL) lip->prefix = "x" ;
	if (lip->suffix == NULL) lip->suffix = "" ;

	return rs ;
}
/* end subroutine (locinfo_defaults) */


/* get the date out of the ID string */
static int makedate_get(cchar *md,cchar **rpp)
{
	int		ch ;
	const char	*sp ;
	const char	*cp ;

	if (rpp != NULL) *rpp = NULL ;

	if ((cp = strchr(md,CH_RPAREN)) == NULL)
	    return SR_NOENT ;

	while (CHAR_ISWHITE(*cp)) cp += 1 ;

	ch = MKCHAR(cp[0]) ;
	if (! isdigitlatin(ch)) {
	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;
	} /* end if (skip over the name) */

	sp = cp ;
	if (rpp != NULL) *rpp = cp ;

	while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	return (cp - sp) ;
}
/* end subroutine (makedate_get) */


