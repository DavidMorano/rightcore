/* b_kshbi */

/* this is a SHELL built-in */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LINEBUFOUT	1		/* line buffering for STDERR */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ kshbi [-Q] [-f <lib>] [<name>] [-af <afile>]


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
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<field.h>
#include	<filebuf.h>
#include	<char.h>
#include	<tmtime.h>
#include	<upt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_kshbi.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif

#ifndef	VBUFLEN
#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif
#endif

#ifndef	OUTLINELEN
#define	OUTLINELEN	80
#endif

#define	OUTPADLEN	20

#ifndef	TO_TMPFILES
#define	TO_TMPFILES	(1*3600)	/* temporary file time-out */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrcmpr(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	pathclean(char *,cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	vecstr_addpath(VECSTR *,cchar *,int) ;
extern int	vecstr_addpathclean(VECSTR *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		libdirs:1 ;
	uint		libdirpath:1 ;
	uint		lib:1 ;
	uint		u:1 ;
	uint		maint:1 ;
	uint		pr:1 ;
	uint		print:1 ;
	uint		tmpmaint:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	cchar		*libfname ;
	cchar		*storedname ;
	void		*dhp ;
	vecstr		stores ;
	vecstr		libdirs ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	pthread_t	tid ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procuserconf_begin(PROGINFO *) ;
static int	procuserconf_end(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procprint(PROGINFO *,void *) ;
static int	procall(PROGINFO *,void *) ;
static int	procsfn(PROGINFO *,char *) ;
static int	procsfner(PROGINFO *,char *,cchar *) ;
static int	procsfnmk(PROGINFO *,cchar *,cchar *) ;
static int	procsfnmkline(PROGINFO *,SHIO *,cchar *,int) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,void *,cchar *,cchar *) ;
static int	procnames(PROGINFO *,SHIO *,cchar *,int) ;
static int	procname(PROGINFO *,SHIO *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_libenv(LOCINFO *,cchar *) ;
static int	locinfo_libdef(LOCINFO *) ;
static int	locinfo_libset(LOCINFO *,cchar *,int) ;
static int	locinfo_libopen(LOCINFO *) ;
static int	locinfo_libopenfind(LOCINFO *,int) ;
static int	locinfo_libclose(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_storedir(LOCINFO *) ;
static int	locinfo_dircheck(LOCINFO *,cchar *) ;
static int	locinfo_minmod(LOCINFO *,cchar *,mode_t) ;
static int	locinfo_storedirtmp(LOCINFO *,char *) ;
static int	locinfo_tmpcheck(LOCINFO *) ;
static int	locinfo_tmpmaint(LOCINFO *) ;
static int	locinfo_chown(LOCINFO *,cchar *) ;
static int	locinfo_fchmodown(LOCINFO *,int,struct ustat *,mode_t) ;
static int	locinfo_loadprids(LOCINFO *) ;

static int	locinfo_libdirfind(LOCINFO *,char *) ;
static int	locinfo_libdirsearch(LOCINFO *,char *,cchar *) ;
static int	locinfo_libdirsearching(LOCINFO *,char *,cchar *) ;
static int	locinfo_libdirsearcher(LOCINFO *,char *,cchar *,vecstr *) ;
static int	locinfo_libdirtest(LOCINFO *,char *) ;
static int	locinfo_libdirs(LOCINFO *) ;
static int	locinfo_libdirpath(LOCINFO *) ;
static int	locinfo_libdirinit(LOCINFO *) ;
static int	locinfo_libdirpr(LOCINFO *) ;
static int	locinfo_libdiradd(LOCINFO *,cchar *) ;

static int	vecstr_loadnames(vecstr *,cchar *) ;
static int	vecstr_loadnamers(vecstr *,cchar *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
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
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
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
	"lib",
	"maint",
	"pr",
	"print",
	NULL
} ;

enum akonames {
	akoname_lib,
	akoname_maint,
	akoname_pr,
	akoname_print,
	akoname_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*exts[] = {
	"so",
	"o",
	"",
	NULL
} ;


/* exported subroutines */


int b_kshbi(int argc,cchar *argv[],void *contextp)
{
	int	rs ;
	int	rs1 ;
	int	ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_kshbi) */


int p_kshbi(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_kshbi) */


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
	int		cl ;
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
	cchar		*ifname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_kshbi: starting DFD=%d\n",rs) ;
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

	pip->lip = &li ;
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

/* argument liste file */
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

/* input file name */
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

/* all */
	                    case 'a':
	                        pip->f.all = TRUE ;
	                        break ;

/* specify a search library */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	    			        lip->have.lib = TRUE ;
	    			        lip->final.lib = TRUE ;
	                                lip->libfname = argp ;
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

/* quiet */
	                    case 'q':
	                        cp = NULL ;
	                        cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        }
	                        if (cp != NULL) {
	                            if ((rs = optbool(cp,cl)) > 0) {
	                                pip->verboselevel = 0 ;
	                            }
	                        } else {
	                            pip->verboselevel = 0 ;
				}
	                        break ;

			    case 'p':
	                        lip->final.print = TRUE ;
	                        lip->have.print = TRUE ;
	                        lip->f.print = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.print = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* unbuffered (really line-buffered) */
	                    case 'u':
	                        lip->f.u = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.u = (rs > 0) ;
	                            }
	                        }
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

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_kshbi: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n", pip->progname,VERSION) ;
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
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: afile=%s\n",pn,afname) ;
	    shio_printf(pip->efp,"%s: ifile=%s\n",pn,ifname) ;
	}

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	        if ((rs = locinfo_libenv(lip,VARBILIB)) >= 0) {
		    rs = locinfo_libdef(lip) ;
		}
	    }
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*lfn = lip->libfname ;
	    shio_printf(pip->efp,"%s: lib=%s\n",pn,lfn) ;
	}

/* do it */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procuserconf_begin(pip)) >= 0) {
		{
	            cchar	*ofn = ofname ;
	            cchar	*afn = afname ;
	            cchar	*ifn = ifname ;
	            rs = process(pip,&ainfo,&pargs,ofn,afn,ifn) ;
		}
	        rs1 = procuserconf_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procuseronf) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

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

/* early return thing */
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
	    debugprintf("b_kshbi: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad argument paths come here */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",pip->progname,rs) ;
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

	fmt = "%s: USAGE> %s [-f <shlib>] [{-a|<name(s)> ...}]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procuserconf_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	pip->euid = geteuid() ;
	return rs ;
}
/* end subroutine (procuserconf_begin) */


static int procuserconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (procuserconf_end) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
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
	                case akoname_lib:
	                    if (! lip->final.lib) {
	                        lip->have.lib = TRUE ;
	                        lip->final.lib = TRUE ;
	                        if (vl > 0) {
	                            rs = locinfo_libset(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_maint:
	                    if (! lip->final.maint) {
	                        lip->have.maint = TRUE ;
	                        lip->final.maint = TRUE ;
	                        lip->f.maint = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.maint = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_pr:
	                    if (! lip->final.pr) {
	                        lip->have.pr = TRUE ;
	                        lip->final.pr = TRUE ;
	                        lip->f.pr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.pr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_print:
	                    if (! lip->final.print) {
	                        lip->have.print = TRUE ;
	                        lip->final.print = TRUE ;
	                        lip->f.print = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.print = (rs > 0) ;
	                        }
	                    }
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		cchar *ofn,cchar *afn,cchar *ifn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_kshbi/process: ent\n") ;
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"r",0666)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    if (lip->f.u) {
	        rs = shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;
	    }
	    if (rs >= 0) {
		LOCINFO	*lip = pip->lip ;
		if (lip->f.print) {
		    rs = procprint(pip,ofp) ;
	            wlen += rs ;
	        } else if (pip->f.all) {
	            rs = procall(pip,ofp) ;
	            wlen += rs ;
	        } else {
	            rs = procargs(pip,aip,bop,ofp,afn,ifn) ;
	            wlen += rs ;
	        }
	    } /* end if (ok) */
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_kshbi/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procprint(PROGINFO *pip,void *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;
	char		rbuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_kshbi/procprint: ent\n") ;
#endif
	if ((rs = locinfo_libdirfind(lip,rbuf)) >= 0) {
	    const int	rl = rs ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: shlib=%t\n" ;
	        shio_printf(pip->efp,fmt,pn,rbuf,rl) ;
	    }
	    rs = shio_print(ofp,rbuf,rl) ;
	    wlen += rs ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_kshbi/procprint: ret rs=%d wlen=%u\n",
	    rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprint) */


static int procall(PROGINFO *pip,void *ofp)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		sfname[MAXPATHLEN+1] ;
	if ((rs = procsfn(pip,sfname)) >= 0) {
	    SHIO	sfile, *sfp = &sfile ;
	    if ((rs = shio_open(sfp,sfname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        cchar		*y = "YES" ;
	        char		lbuf[LINEBUFLEN+1] ;
	        while ((rs = shio_readline(sfp,lbuf,llen)) > 0) {
	            len = rs ;
	            if (lbuf[len-1] == '\n') len -= 1 ;
	            if (len > 0) {
	                rs = shio_printf(ofp,"%3s %t\n",y,lbuf,len) ;
	                wlen += rs ;
	            }
	        } /* end if (reading-lines) */
	        rs1 = shio_close(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sfname) */
	} /* end if (procsfn) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_kshbi/procall: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procall) */


static int procsfn(PROGINFO *pip,char *sfname)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		pl = 0 ;
	if (lip->libfname != NULL) {
	    char	rbuf[MAXPATHLEN+1] ;
	    if ((rs = locinfo_libdirfind(lip,rbuf)) > 0) {
		const int	rl = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_kshbi/procall: sfn=%s\n",rbuf) ;
#endif
	        if ((rs = procsfner(pip,sfname,rbuf)) >= 0) {
	            pl = rs ;
	            rs = locinfo_libset(lip,rbuf,rl) ;
	        }
	    } /* end if (locinfo_libdirfind) */
	} else {
	    rs = SR_NOENT ;
	}
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (procsfn) */


static int procsfner(PROGINFO *pip,char *sfname,cchar *rbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	int		pl = 0 ;
	cchar		*cp ;
	if ((cl = sfbasename(rbuf,-1,&cp)) > 0) {
	    cchar	*tp = strnchr(cp,cl,'.') ;
	    if (tp != NULL) cl = (tp-cp) ;
	    if ((rs = locinfo_storedir(lip)) >= 0) {
	        if ((rs = locinfo_tmpcheck(lip)) >= 0) {
	            cchar	*sdname = lip->storedname ;
	            if ((rs = mkpath2w(sfname,sdname,cp,cl)) >= 0) {
	                struct ustat	sb ;
	                const int	rsn = SR_NOENT ;
	                pl = rs ;
	                if ((rs = uc_stat(sfname,&sb)) >= 0) {
	                    struct ustat	lsb ;
	                    if ((rs = uc_stat(rbuf,&lsb)) >= 0) {
	                        if (lsb.st_mtime > sb.st_mtime) {
	                            rs = procsfnmk(pip,sfname,rbuf) ;
	                        } /* end if (time comparison) */
	                    } /* end if (stat-libdname) */
	                } else if (rs == rsn) {
	                    rs = procsfnmk(pip,sfname,rbuf) ;
	                } /* end if */
	            } /* end if (mkpath) */
	        } /* end if (tmpcheck) */
	    } /* end if (locinfo_storedir) */
	} /* end if (sfbasename) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (procsfner) */


static int procsfnmk(PROGINFO *pip,cchar *sfname,cchar *rbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	char		tbuf[MAXPATHLEN+1] ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = locinfo_storedirtmp(lip,tbuf)) >= 0) {
	    SHIO	sfile, *sfp = &sfile ;
	    if ((rs = shio_open(sfp,tbuf,"wc",0666)) >= 0) {
	        const int	of = O_RDONLY ;
	        int		i = 0 ;
	        cchar		*av[5] ;
	        cchar		**ev = pip->envv ;
	        cchar		*pfname = "sys:nm" ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procsfnmk: pfname=%s\n",pfname) ;
#endif
	        av[i++] = "NM" ;
	        av[i++] = "-P" ;
	        av[i++] = rbuf ;
	        av[i] = NULL ;
	        if ((rs = uc_openprog(pfname,of,av,ev)) >= 0) {
	            FILEBUF	b ;
	            const int	fd = rs ;
	            if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN+1] ;
	                while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	                    int		len = rs ;
	                    if (lbuf[len-1] == '\n') len -= 1 ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("procsfnmk: l=>%t<\n",
	                            lbuf,strlinelen(lbuf,len,40)) ;
#endif
	                    if ((len > 0) && (! CHAR_ISWHITE(lbuf[0]))) {
	                        rs = procsfnmkline(pip,sfp,lbuf,len) ;
	                    } /* end if (positive) */
	                    if (rs < 0) break ;
	                } /* end if (reading lines) */
	                rs1 = filebuf_finish(&b) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (filebuf) */
	            u_close(fd) ;
	        } /* end if (r-file) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procsfnmk: rfile-out rs=%d\n",rs) ;
#endif
	        rs1 = shio_close(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (t-file) */
	    if (rs >= 0) {
	        if ((rs = locinfo_chown(lip,tbuf)) >= 0) {
	            if ((rs = u_rename(tbuf,sfname)) >= 0) {
	                tbuf[0] = '\0' ;
	            }
	        }
	    } /* end if (ok) */
	    if (rs < 0) {
	        if (tbuf[0] != '\0') {
		    uc_unlink(tbuf) ;
		}
	    }
	} /* end if (locinfo_storedirtmp) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procsfnmk: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procsfnmk) */


static int procsfnmkline(PROGINFO *pip,SHIO *sfp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	if (pip == NULL) return SR_FAULT ;
	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    if ((cl > 2) && (strncmp(cp,"b_",2) == 0)) {
	        int	tl ;
	        cchar	*tp ;
	        sl -= ((cp+cl)-sp) ;
	        sp = (cp+cl) ;
	        if ((tl = nextfield(sp,sl,&tp)) > 0) {
	            if ((tl == 1) && (tp[0] == 'T')) {
	                rs = shio_print(sfp,(cp+2),(cl-2)) ;
	            }
	        }
	    } /* end if (possible) */
	} /* end if (nextfield) */
	return rs ;
}
/* end subroutine (procsfnmkline) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp,
		cchar *afn,cchar *ifn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		wlen = 0 ;
	int		cl ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

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
	                rs = procname(pip,ofp,cp,-1) ;
	                wlen += rs ;
		    }
	        }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (positional arguments) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = procnames(pip,ofp,cp,cl) ;
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
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (file-argument) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("n_kshbi/procargs: mid2 rs=%d pan=%u\n",rs,pan) ;
#endif

	if ((rs >= 0) && (pan == 0)) {
	    SHIO	ifile, *ifp = &ifile ;

	    if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	        ifn = STDINFNAME ;

	    if ((rs = shio_open(ifp,ifn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = procnames(pip,ofp,cp,cl) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: inaccessible input (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: ifile=%s\n",pn,ifn) ;
	    } /* end if (opened) */

	} /* end if (file-input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("n_kshbi/procargs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,SHIO *ofp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procname(pip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnames) */


static int procname(PROGINFO *pip,SHIO *ofp,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	const int	nlen = MAXNAMELEN ;
	int		rs ;
	int		wlen = 0 ;
	char		nbuf[MAXNAMELEN + 1] ;
	void		*sp ;

	if ((np == NULL) || (np[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_kshbi/procname: name=%t\n",np,nl) ;
#endif

	if ((rs = sncpy2w(nbuf,nlen,"b_",np,nl)) >= 0) {
	    int		f ;

	    sp = dlsym(RTLD_DEFAULT,nbuf) ;
	    f = (sp != NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_kshbi/procname: dlsym() f=%u lib.init=%u\n",
	            f,lip->f.lib) ;
#endif

	    if (sp == NULL) {
	        if ((rs = locinfo_libopen(lip)) >= 0) {
	            if (lip->dhp != NULL) {
	                sp = dlsym(lip->dhp,nbuf) ;
	                f = (sp != NULL) ;
	            }
	        } /* end if (locinfo_libopen) */
	    } /* end if */

	    if (rs >= 0) {
	        cchar	*ans = ((f) ? "YES" : "NO") ;
	        if (pip->verboselevel > 0) {
	            rs = shio_printf(ofp,"%3s %t\n",ans,np,nl) ;
	            wlen += rs ;
	        }
	        if (pip->debuglevel > 0) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: %3s %t\n" ;
	            shio_printf(pip->efp,fmt,pn,ans,np,nl) ;
	        }
	    }

	} /* end if (name-make) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->uid_pr = -1 ;
	lip->gid_pr = -1 ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = locinfo_libclose(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.libdirs) {
	    lip->open.libdirs = FALSE ;
	    rs1 = vecstr_finish(&lip->libdirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->f.tmpmaint) {
	    int	trs ;
	    rs1 = uptjoin(lip->tid,&trs) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs >= 0) rs = trs ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


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


static int locinfo_libdirfind(LOCINFO *lip,char *rbuf)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	int		len = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_kshbi/locinfo_libdirfind: ent\n") ;
#endif
	if (pip == NULL) return SR_FAULT ;
	if ((rs = locinfo_libdirinit(lip)) >= 0) {
	    cchar	*name = lip->libfname ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_kshbi/locinfo_libdirfind: name=%s\n",name) ;
#endif
	    if (name != NULL) {
	        if (strchr(name,'/') == NULL) {
	            if ((rs = locinfo_libdirs(lip)) >= 0) {
	                if (strchr(name,'.') != NULL) {
	                    rs = locinfo_libdirsearch(lip,rbuf,name) ;
			    len = rs ;
	                } else {
	                    rs = locinfo_libdirsearching(lip,rbuf,name) ;
			    len = rs ;
	                }
	            }
	        } else {
	            if ((rs = mkpath1(rbuf,name)) >= 0) {
		        len = rs ;
	                if ((rs = locinfo_libdirtest(lip,rbuf)) == 0) {
	                    len = 0 ;
	                }
	            }
	        }
	    } else {
		rs = SR_NOENT ;
	    }
	} /* end if (locinfo_libdirinit) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_kshbi/locinfo_libdirfind: ret rs=%d len=%u\n",
		rs,len) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_libdirfind) */


static int locinfo_libdirsearch(LOCINFO *lip,char *rbuf,cchar *name)
{
	vecstr		*ldp = &lip->libdirs ;
	int		rs = SR_OK ;
	int		len = 0 ;
	int		i ;
	cchar		*cp ;
	for (i = 0 ; vecstr_get(ldp,i,&cp) >= 0 ; i += 1) {
	    if (cp != NULL) {
	        if ((rs = mkpath2(rbuf,cp,name)) >= 0) {
	            len = rs ;
	            if ((rs = locinfo_libdirtest(lip,rbuf)) == 0) {
	                len = 0 ;
	            }
	        } /* end if (mkpath) */
	    }
	    if (len > 0) break ;
	    if (rs < 0) break ;
	} /* end for */
	if ((rs >= 0) && (len == 0)) rbuf[0] = '\0' ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_libdirsearch) */


static int locinfo_libdirsearching(LOCINFO *lip,char *rbuf,cchar *name)
{
	vecstr		*ldp = &lip->libdirs ;
	vecstr		ns ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = vecstr_start(&ns,6,0)) >= 0) {
	    if ((rs = vecstr_loadnames(&ns,name)) >= 0) {
	        int	i ;
	        cchar	*cp ;
	        for (i = 0 ; vecstr_get(ldp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                rs = locinfo_libdirsearcher(lip,rbuf,cp,&ns) ;
	                len = rs ;
	            }
	            if (len > 0) break ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (vecstr_loadnames) */
	    rs1 = vecstr_finish(&ns) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	if ((rs >= 0) && (len == 0)) rbuf[0] = '\0' ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_libdirsearching) */


static int locinfo_libdirsearcher(LOCINFO *lip,char *rbuf,cchar *dp,vecstr *nlp)
{
	int		rs = SR_OK ;
	int		i ;
	int		len = 0 ;
	cchar		*np ;
	for (i = 0 ; vecstr_get(nlp,i,&np) >= 0 ; i += 1) {
	    if (np != NULL) {
	        if ((rs = mkpath2(rbuf,dp,np)) >= 0) {
	            len = rs ;
	            if ((rs = locinfo_libdirtest(lip,rbuf)) == 0) {
	                len = 0 ;
	            }
	        } /* end if (mkpath) */
	    }
	    if (len > 0) break ;
	    if (rs < 0) break ;
	} /* end for */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_libdirsearcher) */


static int locinfo_libdirtest(LOCINFO *lip,char *rbuf)
{
	struct ustat	sb ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("b_kshbi/locinfo_libdirtest: ent name=%s\n",rbuf) ;
#endif
	if ((rs = uc_stat(rbuf,&sb)) >= 0) {
	    rs = (S_ISREG(sb.st_mode)) ? 1 : 0 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("b_kshbi/locinfo_libdirtest: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_libdirtest) */


static int locinfo_libdirs(LOCINFO *lip)
{
	int		rs ;
	if ((rs = locinfo_libdirpath(lip)) >= 0) {
	    if (lip->f.pr) {
	        rs = locinfo_libdirpr(lip) ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("b_kshbi/locinfo_libdirs: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_libdirs) */


static int locinfo_libdirpath(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	cchar		*lp ;
	if (! lip->f.libdirpath) {
	    lip->f.libdirpath = TRUE ;
	    if ((lp = getourenv(pip->envv,VARLIBPATH)) != NULL) {
	        if ((rs = locinfo_libdirinit(lip)) >= 0) {
	            vecstr	*ldp = &lip->libdirs ;
	            rs = vecstr_addpathclean(ldp,lp,-1) ;
	        }
	    }
	} /* end if (need libdirpath) */
#if	CF_DEBUGS
	debugprintf("b_kshbi/locinfo_libdirpath: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_libdirpath) */


static int locinfo_libdirpr(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pip->pr,"lib")) >= 0) {
	    struct ustat	sb ;
	    if (uc_stat(tbuf,&sb) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            rs = locinfo_libdiradd(lip,tbuf) ;
	        }
	    } /* end if (uc_stat) */
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (locinfo_libdirpr) */


static int locinfo_libdiradd(LOCINFO *lip,cchar *libdir)
{
	int		rs ;
	if ((rs = locinfo_libdirinit(lip)) >= 0) {
	    vecstr	*ldp = &lip->libdirs ;
	    char	rbuf[MAXPATHLEN+1] ;
	    if ((rs = pathclean(rbuf,libdir,-1)) >= 0) {
	        const int	rsn = SR_NOTFOUND ;
	        const int	rl = rs ;
	        if ((rs = vecstr_findn(ldp,rbuf,rs)) == rsn) {
	            rs = vecstr_add(ldp,rbuf,rl) ;
	        }
	    }
	} /* end if (locinfo_libdirinit) */
	return rs ;
}
/* end subroutine (locinfo_libdiradd) */


static int locinfo_libdirinit(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	vecstr		*ldp = &lip->libdirs ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("b_kshbi/locinfo_libdirinit: ent\n") ;
#endif
	if (pip == NULL) return SR_FAULT ;
	if (! lip->open.libdirs) {
	    if ((rs = vecstr_start(ldp,10,0)) >= 0) {
	        lip->open.libdirs = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("b_kshbi/locinfo_libdirinit: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_libdirinit) */


static int locinfo_libenv(LOCINFO *lip,cchar *varname)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->libfname == NULL) {
	    if (varname == NULL) {
	        cchar	*libfname ;
	        if ((libfname = getourenv(pip->envv,varname)) != NULL) {
	            lip->libfname = libfname ;
	        }
	    }
	} /* end if (was NULL) */

	return rs ;
}
/* end subroutine (locinfo_libenv) */


static int locinfo_libdef(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->libfname == NULL) {
	    lip->libfname = DEFLIBFNAME ;
	}
	return rs ;
}
/* end subroutine (locinfo_libdef) */


static int locinfo_libset(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_kshbi/locinfo_libset: f_lib=%u\n",lip->f.lib) ;
#endif

	if (vp != NULL) {
	    cchar	**vpp = &lip->libfname ;
	    rs = locinfo_setentry(lip,vpp,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (locinfo_libset) */


static int locinfo_libopen(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	dlflags = (RTLD_LAZY|RTLD_LOCAL) ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_kshbi/locinfo_libopen: ent f_lib=%u\n",
	        lip->f.lib) ;
#endif

	if (! lip->open.lib) {
	    cchar	*libfname = lip->libfname ;
	    lip->open.lib = TRUE ;
	    if (libfname != NULL) {
	        if (strchr(libfname,'/') == NULL) {
	            rs = locinfo_libopenfind(lip,dlflags) ;
	        } else {
	            lip->dhp = dlopen(libfname,dlflags) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("b_kshbi/locinfo_libopen: dhp=%p\n",
	                    lip->dhp) ;
	                if (lip->dhp == NULL)
	                    debugprintf("b_kshbi/locinfo_libopen: "
	                        "dlerr=%s\n", dlerror()) ;
	            }
#endif /* CF_DEBUG */
	        }
	        if (lip->dhp == NULL) {
	            if (pip->debuglevel > 0) {
	                cchar	*pn = pip->progname ;
	                cchar	*fmt = "%s: library unavailable\n" ;
	                shio_printf(pip->efp,fmt,pn) ;
	            }
	        } /* end if (library unavailable) */
	    }
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_kshbi/locinfo_libopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_libopen) */


static int locinfo_libclose(LOCINFO *lip)
{
	if (lip->dhp != NULL) {
	    dlclose(lip->dhp) ;
	    lip->dhp = NULL ;
	}
	return SR_OK ;
}
/* end subroutine (locinfo_libclose) */


static int locinfo_libopenfind(LOCINFO *lip,int dlflags)
{
	PROGINFO	*pip = lip->pip ;
	VECSTR		ns ;
	int		rs ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_kshbi/locinfo_libopenfind: ent n=%s\n",
		lip->libfname) ;
#endif

	if ((rs = vecstr_start(&ns,6,0)) >= 0) {
	    cchar	*name = lip->libfname ;
	    if ((rs = vecstr_loadnames(&ns,name)) >= 0) {
	        int		i ;
	        cchar		*np ;

	        for (i = 0 ; vecstr_get(&ns,i,&np) >= 0 ; i += 1) {
	            if (np != NULL) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_kshbi/locinfo_libopenfind: n=%s\n",
				np) ;
#endif

	                lip->dhp = dlopen(np,dlflags) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_kshbi/locinfo_libopenfind: dhp=%p\n",
	                    lip->dhp) ;
	                    if (lip->dhp == NULL) {
	                        debugprintf("b_kshbi/locinfo_libopenfind: "
	                            "dlerr=%s\n", dlerror()) ;
			    }
	                }
#endif /* CF_DEBUG */

		    }
	            if (lip->dhp != NULL) break ;
	        } /* end for */

	    } /* end if (vecstr_loadnames) */

	    rs1 = vecstr_finish(&ns) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_kshbi/locinfo_libopenfind: ret rs=%d c=%u\n",
	        rs,(lip->dhp != NULL)) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_libopenfind) */


static int locinfo_storedir(LOCINFO *lip)
{
	int		rs ;
	if ((rs = locinfo_loadprids(lip)) >= 0) {
	    PROGINFO	*pip = lip->pip ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(tbuf,pip->pr,VDNAME)) >= 0) {
	        if ((rs = locinfo_dircheck(lip,tbuf)) >= 0) {
	            cchar	*sn = pip->searchname ;
	            char	sbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(sbuf,tbuf,sn)) >= 0) {
	                const int	pl = rs ;
	                if ((rs = locinfo_dircheck(lip,sbuf)) >= 0) {
	                    cchar	**vpp = &lip->storedname ;
	                    rs = locinfo_setentry(lip,vpp,sbuf,pl) ;
	                }
	            }
	        } /* end if (locinfo_dircheck) */
	    } /* end if (mkpath) */
	} /* end if (locinfo_loadprids) */
	return rs ;
}
/* end subroutine (locinfo_storedir) */


static int locinfo_dircheck(LOCINFO *lip,cchar *dname)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("kshbi/locinfo_dircheck: ent dn=%s\n",dname) ;
#endif

	{
	    struct ustat	sb ;
	    const uid_t		euid = pip->euid ;
	    const mode_t	dm = (0777 | S_ISGID) ;
	    const int		rsn = SR_NOENT ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
	        if (sb.st_uid == euid) {
	            rs = locinfo_minmod(lip,dname,dm) ;
	        }
	    } else if (rs == rsn) {
	        if ((rs = mkdirs(dname,dm)) >= 0) {
	            rs = locinfo_minmod(lip,dname,dm) ;
	        } /* end if (mkdirs) */
	    } /* end if (stat) */
	} /* end block */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("kshbi/locinfo_dircheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_dircheck) */


static int locinfo_minmod(LOCINFO *lip,cchar *dname,mode_t dm)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if ((rs = uc_minmod(dname,dm)) >= 0) {
	    const uid_t		euid = pip->euid ;
	    if (lip->uid_pr != euid) {
	        u_chown(dname,lip->uid_pr,lip->gid_pr) ;
	    }
	} /* end if (uc_minmod) */
	return rs ;
}
/* end subroutine (locinfo_minmod) */


static int locinfo_storedirtmp(LOCINFO *lip,char *tbuf)
{
	int		rs ;
	cchar		*tt = "tmpXXXXXXXXXXX" ;
	char		template[MAXPATHLEN+1] ;
	tbuf[0] = '\0' ;
	if ((rs = mkpath2(template,lip->storedname,tt)) >= 0) {
	    rs = mktmpfile(tbuf,0664,template) ;
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (locinfo_storedirtmp) */


static int locinfo_tmpcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->storedname != NULL) {
	    TMTIME	t ;
	    if ((rs = tmtime_localtime(&t,pip->daytime)) >= 0) {
	        if ((t.hour >= HOUR_MAINT) && lip->f.maint) {
		    uptsub_t	thr = (uptsub_t) locinfo_tmpmaint ;
	            pthread_t	tid ;
	            if ((rs = uptcreate(&tid,NULL,thr,lip)) >= 0) {
	                rs = 1 ;
	                lip->tid = tid ;
	                lip->f.tmpmaint = TRUE ;
	            } /* end if (uptcreate) */
	        } /* end if (after hours) */
	    } /* end if (tmtime_localtime) */
	} /* end if (store-dname) */

	return rs ;
}
/* end subroutine (locinfo_tmpcheck) */


/* this runs as an independent thread */
static int locinfo_tmpmaint(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = TO_TMPFILES ;
	int		rs ;
	int		c = 0 ;
	int		f_need = lip->f.maint ;
	cchar		*dname = lip->storedname ;
	char		tsfname[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tsfname,dname,TSFNAME)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = (O_WRONLY|O_CREAT) ;
	    if ((rs = u_open(tsfname,of,om)) >= 0) {
	        struct ustat	usb ;
	        const int	fd = rs ;
	        if ((rs = u_fstat(fd,&usb)) >= 0) {
	            time_t	dt = pip->daytime ;
	            if ((rs = locinfo_fchmodown(lip,fd,&usb,om)) >= 0) {
	                int	maintlapse = (dt - usb.st_mtime) ;
	                f_need = f_need || (usb.st_size == 0) ;
	                f_need = f_need || (maintlapse >= to) ;
	                if (f_need) {
	                    int		tl ;
	                    char	timebuf[TIMEBUFLEN + 3] ;
	                    timestr_logz(dt,timebuf) ;
	                    tl = strlen(timebuf) ;
	                    timebuf[tl++] = '\n' ;
	                    rs = u_write(fd,timebuf,tl) ;
	                } /* end if (timed-out) */
	            } /* end if (locinfo_fchmodown) */
	        } /* end if (stat) */
	        u_close(fd) ;
	    } /* end if (open file) */
	} /* end if (mkpath timestamp) */

	if ((rs >= 0) && f_need) {
	    rs = rmdirfiles(dname,"tmp",to) ;
	    c = rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_tmpmaint) */


static int locinfo_chown(LOCINFO *lip,cchar *fname)
{
	int		rs ;
	int		f = FALSE ;
	if ((rs = locinfo_loadprids(lip)) >= 0) {
	    const uid_t		uid_pr = lip->uid_pr ;
	    const gid_t		gid_pr = lip->gid_pr ;
	    const int		n = _PC_CHOWN_RESTRICTED ;
	    if ((rs = u_pathconf(fname,n,NULL)) == 0) {
	        f = TRUE ;
	        u_chown(fname,uid_pr,gid_pr) ; /* may fail */
	    } else if (rs == SR_NOSYS) {
	        rs = SR_OK ;
	    }
	} /* end if (locinfo_loadprids) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_chown) */


static int locinfo_fchmodown(LOCINFO *lip,int fd,struct ustat *sbp,mode_t mm)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if ((sbp->st_size == 0) && (pip->euid == sbp->st_uid)) {
	    if ((sbp->st_mode & S_IAMB) != mm) {
	        if ((rs = locinfo_loadprids(lip)) >= 0) {
	            if ((rs = uc_fminmod(fd,mm)) >= 0) {
	                const uid_t	uid_pr = lip->uid_pr ;
	                const gid_t	gid_pr = lip->gid_pr ;
	                const int	n = _PC_CHOWN_RESTRICTED ;
	                if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
	                    f = TRUE ;
	                    u_fchown(fd,uid_pr,gid_pr) ; /* may fail */
	                } else if (rs == SR_NOSYS) {
	                    rs = SR_OK ;
	                }
	            }
	        } /* end if (locinfo_loadprids) */
	    } /* end if (need change) */
	} /* end if (zero-file) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_fchmodown) */


static int locinfo_loadprids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->uid_pr < 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	        lip->uid_pr = sb.st_uid ;
	        lip->gid_pr = sb.st_gid ;
	    } /* end if (u_stat) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_loadprids) */


static int vecstr_loadnames(vecstr *nlp,cchar *name)
{
	int		rs ;
	int		c = 0 ;
	if ((rs = vecstr_loadnamers(nlp,name)) >= 0) {
	    c += rs ;
	    if (strncmp(name,"lib",3) != 0) {
	        const int	nlen = MAXNAMELEN ;
	        char		nbuf[MAXNAMELEN+1] ;
	        if ((rs = sncpy2(nbuf,nlen,"lib",name)) >= 0) {
	            rs = vecstr_loadnamers(nlp,nbuf) ;
	            c += rs ;
	        }
	    }
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadnames) */


static int vecstr_loadnamers(vecstr *nlp,cchar *name)
{
	const int	nlen = MAXNAMELEN ;
	int		rs = SR_OK ;
	int		c = 0 ;
	if (strchr(name,'.') == NULL) {
	    int		i ;
	    char	nbuf[MAXNAMELEN+1] ;
	    for (i = 0 ; (rs >= 0) && (exts[i] != NULL) ; i += 1) {
	        int	nl = -1 ;
	        cchar	*np = name ;
	        if (exts[i][0] != '\0') {
	            if ((rs = snsds(nbuf,nlen,name,exts[i])) >= 0) {
	                np = nbuf ;
	                nl = rs ;
	            }
	        }
	        if (rs >= 0) {
	            rs = vecstr_add(nlp,np,nl) ;
	            c += 1 ;
	        }
	    } /* end for */
	} /* end if (no-suffix) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadnamers) */


