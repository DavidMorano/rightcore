/* b_pt */

/* front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEFSECTION	0		/* use default MAN section? */


/* revision history:

	= 2004-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end for retrieving environment variables and
	outputting them in a packaged-up format for SHELL interpretation.

	Synopsis:

	$ pt [-p <varname>] <name(s)>

	Notes:

	This is not entirely pretty everywhere.  But it works, and certainly
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
#include	<paramopt.h>
#include	<ids.h>
#include	<sbuf.h>
#include	<fsdir.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_pt.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO_MAGIC	0x99224571
#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_PT	struct locinfo_ptypes

#define	PATHTRY		struct pathtry

#define	PO_PATHNAMES	"pathnames"
#define	PO_SECTIONS	"sections"

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARFPATH
#define	VARFPATH	"FPATH"
#endif

#ifndef	VARCDPATH
#define	VARCDPATH	"CDPATH"
#endif

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matnstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isprintlatin(int) ;
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
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_ptypes {
	uint		a:1 ;		/* "all" */
	uint		c:1 ;		/* change-directory */
	uint		e:1 ;		/* execution */
	uint		f:1 ;		/* functions */
	uint		l:1 ;		/* library */
	uint		m:1 ;		/* manual */
	uint		i:1 ;		/* include */
	uint		x:1 ;		/* X-Windows */
	uint		xu:1 ;		/* X-Windows user */
} ;

struct locinfo {
	PROGINFO	*pip ;
	const char	*po_pathnames ;
	const char	*po_sections ;
	LOCINFO_PT	pt ;
	PARAMOPT	lists ;
	IDS		id ;
	int		nmax ;
	int		n ;
	int		na ; /* supports the 'nargs' feature */
	int		f_q ;
} ;

struct pathtry {
	LOCINFO		*lip ;
	struct ustat	*sbp ;
	SHIO		*ofp ;
	char		*fname ;
	const char	*name ;
	int		pathlen ;
	int		namelen ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procname(PROGINFO *,SHIO *,const char *) ;
static int	procpathname(PROGINFO *,SHIO *,cchar *,cchar *,int) ;
static int	procpathtry(PROGINFO *,SHIO *,int,cchar *,int,cchar *,int) ;
static int	procpathtry_cd(PROGINFO *,PATHTRY *) ;
static int	procpathtry_exec(PROGINFO *,PATHTRY *) ;
static int	procpathtry_func(PROGINFO *,PATHTRY *) ;
static int	procpathtry_lib(PROGINFO *,PATHTRY *) ;
static int	procpathtry_man(PROGINFO *,PATHTRY *) ;
static int	procpathtry_manany(PROGINFO *,PATHTRY *) ;
static int	procpathtry_inc(PROGINFO *,PATHTRY *) ;
static int	procpathtry_liber(PROGINFO *,PATHTRY *) ;
static int	procpathtry_xfile(PROGINFO *,PATHTRY *) ;
static int	procqualname(PROGINFO *,SHIO *,const char *) ;
static int	printit(PROGINFO *,SHIO *,cchar *) ;
static int	pathmk(PATHTRY *,cchar *,int) ;
static int	pathmkman(PATHTRY *,const char *,const char *) ;
static int	pathmkinc(PATHTRY *,const char *,int) ;
static int	pathmklib(PATHTRY *,cchar *,cchar *,int,cchar *) ;
static int	isenvok(const char *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_defspec(LOCINFO *) ;
static int	locinfo_pathspecs(LOCINFO *,int) ;
static int	locinfo_pathdef(LOCINFO *) ;
static int	locinfo_idbegin(LOCINFO *) ;
static int	locinfo_idend(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_notdone(LOCINFO *,int) ;


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

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
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
	progopt_xu,
	progopt_overlast
} ;

static const char	*pathnames[] = {
	VARCDPATH,
	VARPATH,
	VARFPATH,
	VARLIBPATH,
	VARMANPATH,
	VARINCPATH,
	VARXFILESEARCHPATH,
	VARXUSERFILESEARCHPATH,
	NULL
} ;

enum pathnames {
	pathname_cdpath,
	pathname_execpath,
	pathname_funcpath,
	pathname_libpath,
	pathname_manpath,
	pathname_incpath,
	pathname_xpath,
	pathname_xupath,
	pathname_overlast
} ;

static const char	*mannames[] = {
	"man",
	"sman",
	NULL
} ;

static const char	*libexts[] = {
	"so",
	"a",
	NULL
} ;


/* exported subroutines */


int b_pt(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_pt) */


int p_pt(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_pt) */


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
	int		nargs = 0 ;
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
	    debugprintf("b_pt: starting DFD=%d\n",rs) ;
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

	                case argopt_xu:
	                    li.pt.xu = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            li.pt.xu = (rs > 0) ;
	                        }
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

	                    case 'a':
	                        li.pt.a = TRUE ;
	                        break ;

	                    case 'c':
	                        li.pt.c = TRUE ;
	                        break ;

	                    case 'e':
	                        li.pt.e = TRUE ;
	                        break ;

	                    case 'f':
	                        li.pt.f = TRUE ;
	                        break ;

	                    case 'i':
	                        li.pt.i = TRUE ;
	                        break ;

	                    case 'l':
	                        li.pt.l = TRUE ;
	                        break ;

	                    case 'm':
	                        li.pt.m = TRUE ;
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

/* path names */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                const char	*po = PO_PATHNAMES ;
	                                rs = paramopt_loads(&li.lists,po,
	                                    argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        li.f_q = TRUE ;
	                        break ;

/* section(s) */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                const char	*po = PO_SECTIONS ;
	                                rs = paramopt_loads(&li.lists,po,
	                                    argp,argl) ;
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

	                    case 'x':
	                        li.pt.x = TRUE ;
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
	    debugprintf("b_pt: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* option parsing */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

/* some argument adjustments */

	if (lip->pt.a) {
	    lip->pt.e = TRUE ;
	    lip->pt.c = TRUE ;
	    lip->pt.f = TRUE ;
	    lip->pt.l = TRUE ;
	    lip->pt.m = TRUE ;
	    lip->pt.i = TRUE ;
	    lip->pt.x = TRUE ;
	    lip->pt.xu = TRUE ;
	}

/* load specified path names */

	if (rs >= 0) {
	    if ((rs = locinfo_defspec(&li)) >= 0) {
		int	v = -1 ;
	        if (argval != NULL) {
	            rs = optvalue(argval,-1) ;
		    v = rs ;
	        }
	        if (rs >= 0) {
	            if ((rs = locinfo_pathspecs(lip,v)) >= 0) {
	    		rs = locinfo_pathdef(&li) ;
		    }
	        }
	    }
	} /* end if (ok) */

#if	CF_DEFSECTION
	if (rs >= 0) {
	    const char	*ccp ;

	    cl = paramopt_fetch(&li.lists,PO_SECTIONS,NULL,&ccp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("b_pt: def section cl=%d\n",cl) ;
	        if (cl >= 0)
	            debugprintf("b_pt: def section=%t\n",ccp,cl) ;
	    }
#endif /* CF_DEBUG */

	    if (cl == SR_NOTFOUND) {
	        rs = paramopt_loads(&li.lists,PO_SECTIONS,"1",1) ;
	    }

	} /* end block (default manual section) */
#endif /* CF_DEFSECTION */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = locinfo_idbegin(lip)) >= 0) {
	        {
	            rs = procargs(pip,&ainfo,&pargs,ofname,afname) ;
	            nargs = rs ;
	        }
	        rs1 = locinfo_idend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ids) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} else if ((rs >= 0) && (lip->na < nargs)) {
	    ex = EX_NOPROG ;
	} else if (rs >= 0) {
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
	    debugprintf("b_pt: final mallout=%u\n",(mo-mo_start)) ;
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
/* end subroutine (b_pt) */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	if (pip->efp != NULL) {

	    fmt = "%s: USAGE> %s "
	        "[-e] [-f] [-l] [-i] [-m] [-x] [-xu] [-c] [-a] "
	        "<name(s)>\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-q] [-p <varname(s)>] [-s <section(s)>] \n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if (error-output enabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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

	                case progopt_xu:
	                    c += 1 ;
	                    lip->pt.xu = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->pt.xu = (rs > 0) ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *ofn)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	int		na = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"r",0666)) >= 0) {
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
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procname(pip,ofp,cp) ;
	                    na += rs ;
	                }
	            }

	            if (rs < 0) {
	                if (rs == SR_NOENT) {
	                    fmt = "%s: variable not present (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                } else {
	                    fmt = "%s: error processing variable (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                }
	                shio_printf(pip->efp,"%s: var=%s\n",pn,cp) ;
	                break ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
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
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        lbuf[(cp-lbuf)+cl] = '\0' ;
	                        rs = procname(pip,ofp,cp) ;
	                        na += rs ;
	                    }
	                }

	                if (rs < 0) {
	                    if (rs == SR_NOENT) {
	                        fmt = "%s: variable not present (%d)\n" ;
	                        shio_printf(pip->efp,fmt,pn,rs) ;
	                    } else {
	                        fmt = "%s: error processing variable (%d)\n" ;
	                        shio_printf(pip->efp,fmt,pn,rs) ;
	                    }
	                    shio_printf(pip->efp,"%s: var=%s\n",pn,cp) ;
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

	    } /* end if (processing file argument file list) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	lip->na = na ; /* supports the 'nargs' feature */
	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


/* process a name */
static int procname(PROGINFO *pip,SHIO *ofp,cchar *np)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	const int	nl = strlen(np) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	int		c = 0 ;
	int		f = FALSE ;
	const char	*kn = PO_PATHNAMES ;
	const char	*vp ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: query=%s\n",
	        pip->progname,np) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pt/procname: query=%s\n",np) ;
#endif

	if (np[0] != '/') {
	    PARAMOPT		*plp = &lip->lists ;
	    PARAMOPT_CUR	cur ;
	    if ((rs = paramopt_curbegin(plp,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = paramopt_enumvalues(plp,kn,&cur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if ((rs >= 0) && (vp != NULL)) {
			if ((rs = locinfo_notdone(lip,0)) > 0) {
	                    rs = procpathname(pip,ofp,vp,np,nl) ;
	                    c += rs ;
			} else
			    f = TRUE ;
	            }
		    if (f) break ;
	        } /* end while */

	        rs1 = paramopt_curend(&lip->lists,&cur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	} else {
	    rs = procqualname(pip,ofp,np) ;
	    c += rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


static int procpathname(PROGINFO *pip,SHIO *ofp,cchar *pname,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	int		f = FALSE ;

	if (pname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathname: pname=>%s< name=%t\n", 
	        pname,np,nl) ;
#endif

	if (isenvok(pname)) {
	    LOCINFO	*lip = pip->lip ;
	    int		pni ;
	    const char	*tp, *sp ;
	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"%s: pathname=%s\n",
	            pip->progname,pname) ;
	    }
	    if ((sp = getourenv(pip->envv,pname)) != NULL) {
	        pni = matstr(pathnames,pname,-1) ;
	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,"%s: pni=%d\n",
	                pip->progname,pni) ;
	        }
	        while ((tp = strpbrk(sp,":;")) != NULL) {
		    if ((rs = locinfo_notdone(lip,0)) > 0) {
	                rs = procpathtry(pip,ofp,pni,np,nl,sp,(tp-sp)) ;
		        c += rs ;
		    } else {
			f = TRUE ;
		    }
	            sp = (tp + 1) ;
		    if (f) break ;
	            if (rs < 0) break ;
	        } /* end while */
	        if ((rs >= 0) && (sp[0] != '\0')) {
		    if ((rs = locinfo_notdone(lip,0)) > 0) {
	                rs = procpathtry(pip,ofp,pni,np,nl,sp,-1) ;
	                c += rs ;
		    }
	        } /* end if (left-over) */
	    } /* end if (non-null) */
	} /* end if (env-ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procpathname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathname) */


static int procpathtry(PROGINFO *pip,SHIO *ofp,int pni,cchar *np,int nl,
		cchar *pp,int pl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	int		f = TRUE ;
	char		fname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procpathtry: ent pni=%d\n",pni) ;
	    debugprintf("procpathtry: name=%t\n",np,nl) ;
	    debugprintf("procpathtry: pathlen=%d\n",pl) ;
	    debugprintf("procpathtry: path=%t\n",pp,pl) ;
	}
#endif

	fname[0] = '\0' ;
	if ((pp[0] != '\0') && (pl != 0)) {
	    f = FALSE ;
	    if ((rs1 = mkpath1w(fname,pp,pl)) > 0) {
	        pl = rs1 ;
	        if ((rs1 = uc_stat(fname,&sb)) >= 0) {
	            f = S_ISDIR(sb.st_mode) ;
	        }
	    }
	} else
	    pl = 0 ;

	if (pl == 0) {
	    memset(&sb,0,sizeof(struct ustat)) ;
	}

/* switch on individual path type */

	if (f) {
	    PATHTRY	pt ;
	    memset(&pt,0,sizeof(PATHTRY)) ;
	    pt.lip = lip ;
	    pt.ofp = ofp ;
	    pt.sbp = &sb ;
	    pt.name = np ;
	    pt.namelen = nl ;
	    pt.fname = fname ;
	    pt.pathlen = pl ;
	    switch (pni) {
	    case pathname_cdpath:
	        rs = procpathtry_cd(pip,&pt) ;
	        break ;
	    default:
	    case pathname_execpath:
	        rs = procpathtry_exec(pip,&pt) ;
	        break ;
	    case pathname_funcpath:
	        rs = procpathtry_func(pip,&pt) ;
	        break ;
	    case pathname_libpath:
	        rs = procpathtry_lib(pip,&pt) ;
	        break ;
	    case pathname_manpath:
	        rs = procpathtry_man(pip,&pt) ;
	        break ;
	    case pathname_incpath:
	        rs = procpathtry_inc(pip,&pt) ;
	        break ;
	    case pathname_xpath:
	    case pathname_xupath:
	        rs = procpathtry_xfile(pip,&pt) ;
	        break ;
	    } /* end switch */
	    c += rs ;
	} /* end if (execute) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry) */


static int procpathtry_cd(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	struct ustat	*sbp = ptp->sbp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	usbp = (struct ustat *) sbp ;
	rs1 = pathmk(ptp,ptp->name,ptp->namelen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_cd: pathmk() rs=%d\n",rs1) ;
#endif

	if (rs1 > 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procpathtry_cd: fname=%s\n",ptp->fname) ;
#endif
	    rs1 = uc_stat(ptp->fname,sbp) ;
	}

	if (rs1 >= 0) {
	    rs1 = SR_NOTDIR ;
	    if (S_ISDIR(usbp->st_mode)) {
	        rs1 = sperm(&lip->id,sbp,(X_OK | R_OK)) ;
	    }
	}

	if (rs1 >= 0) {
	    rs = printit(pip,ptp->ofp,ptp->fname) ;
	    c += 1 ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_cd) */


static int procpathtry_exec(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	struct ustat	*sbp = ptp->sbp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	usbp = (struct ustat *) sbp ;
	rs1 = pathmk(ptp,ptp->name,ptp->namelen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_exec: pathmk() rs=%d\n",rs1) ;
#endif

	if (rs1 > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procpathtry_exec: fname=%s\n",ptp->fname) ;
#endif

	    rs1 = uc_stat(ptp->fname,sbp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("procpathtry_exec: uc_stat() rs=%d\n",rs) ;
	        debugprintf("procpathtry_exec: mode=\\x%04X\n",
	            usbp->st_mode) ;
	    }
#endif

	    if ((rs1 >= 0) && S_ISREG(usbp->st_mode)) {

	        rs1 = sperm(&lip->id,sbp,X_OK) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procpathtry_exec: sperm() rs=%d\n",rs1) ;
#endif

	        if (rs1 >= 0) {
	            rs = printit(pip,ptp->ofp,ptp->fname) ;
	            c += 1 ;
	        }

	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_exec: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_exec) */


static int procpathtry_func(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	struct ustat	*sbp = ptp->sbp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	usbp = (struct ustat *) sbp ;
	if ((rs1 = pathmk(ptp,ptp->name,ptp->namelen)) >= 0) {

	    if ((rs1 = uc_stat(ptp->fname,sbp)) >= 0) {
	        if (S_ISREG(usbp->st_mode)) {
	            if ((rs1 = sperm(&lip->id,sbp,R_OK)) >= 0) {
	                rs = printit(pip,ptp->ofp,ptp->fname) ;
	                c += 1 ;
	            }
		}
	    }

	} /* end if (pathmk) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_func: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_func) */


static int procpathtry_lib(PROGINFO *pip,PATHTRY *ptp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl = ptp->namelen ;
	int		c = 0 ;
	const char	*tp ;
	const char	*np = ptp->name ;
	const char	*prelib = "lib" ;


/* straight up */

	rs1 = pathmk(ptp,ptp->name,ptp->namelen) ;

	if (rs1 > 0)
	    rs = procpathtry_liber(pip,ptp) ;

/* simple extensions */

	if (((tp = strnrchr(np,nl,'.')) == NULL) ||
	    (matstr(libexts,(tp+1),((np+nl)-tp)) < 0)) {
	    int		i ;
	    const char	*pre, *suf ;

	    for (i = 0 ; (rs >= 0) && (libexts[i] != NULL) ; i += 1) {

	        pre = NULL ;
	        suf = libexts[i] ;
	        rs1 = pathmklib(ptp,pre,ptp->name,ptp->namelen,suf) ;
	        if (rs1 > 0) {
	            rs = procpathtry_liber(pip,ptp) ;
	            c += rs ;
	        }

	        if (rs >= 0) {

	            if ((ptp->namelen <= 3) || 
	                (strcmp(ptp->name,prelib) != 0)) {

	                pre = prelib ;
	                rs1 = pathmklib(ptp,pre,ptp->name,ptp->namelen,suf) ;
	                if (rs1 > 0) {
	                    rs = procpathtry_liber(pip,ptp) ;
	                    c += rs ;
	                }

	            } /* end if */

	        } /* end if */

	    } /* end for (extensions) */

	} /* end if (simple extensions) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_lib) */


static int procpathtry_liber(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	usbp = (struct ustat *) ptp->sbp ;
	rs1 = uc_stat(ptp->fname,ptp->sbp) ;

	if ((rs1 >= 0) && S_ISREG(usbp->st_mode)) {
	    if ((rs1 = sperm(&lip->id,ptp->sbp,R_OK)) >= 0) {
	        rs = printit(pip,ptp->ofp,ptp->fname) ;
	        c += 1 ;
	    }
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_liber) */


static int procpathtry_man(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	struct ustat	*sbp = ptp->sbp ;
	PARAMOPT	*plp ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	int		i ;
	int		c = 0 ;
	int		f = FALSE ;
	const char	*kn = PO_SECTIONS ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_man: ent\n") ;
#endif

	usbp = (struct ustat *) sbp ;
	plp = &lip->lists ;

	if ((rs = paramopt_curbegin(plp,&cur)) >= 0) {

	    while (rs >= 0) {
	        vl = paramopt_enumvalues(plp,kn,&cur,&vp) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;
	        if ((rs >= 0) && (vp == NULL)) continue ;

	        f = TRUE ;
	        for (i = 0 ; (rs >= 0) && (mannames[i] != NULL) ; i += 1) {

	            rs1 = pathmkman(ptp,mannames[i],vp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("procpathtry_man: pathmk() rs=%d\n",rs1) ;
#endif

	            if (rs1 > 0)
	                rs1 = uc_stat(ptp->fname,sbp) ;

	            if (rs1 >= 0) {
	                rs1 = SR_ACCESS ;
	                if (S_ISREG(usbp->st_mode)) {
	                    rs1 = sperm(&lip->id,sbp,R_OK) ;
	                }
	            }

	            if (rs1 >= 0) {
	                rs = printit(pip,ptp->ofp,ptp->fname) ;
	                c += 1 ;
	            }

	        } /* end for */

	    } /* end while */

	    rs1 = paramopt_curend(plp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	if ((rs >= 0) && (! f)) {
	    ptp->fname[ptp->pathlen] = '\0' ;
	    rs = procpathtry_manany(pip,ptp) ;
	    c += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_man: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_man) */


static int procpathtry_manany(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp ;
	struct ustat	*sbp = ptp->sbp ;
	FSDIR		d ;
	FSDIR_ENT	ds ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_manany: dir=%s\n",ptp->fname) ;
#endif

	usbp = (struct ustat *) sbp ;
	ptp->fname[ptp->pathlen] = '\0' ;
	if ((rs1 = fsdir_open(&d,ptp->fname)) >= 0) {
	    int		dnl ;
	    int		mni ;
	    const char	*tp ;

	    while ((rs = fsdir_read(&d,&ds)) > 0) {
	        dnl = rs ;
	        if (ds.name[0] == '.') continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procpathtry_manany: de=%s\n",ds.name) ;
#endif

	        mni = matpstr(mannames,3,ds.name,dnl) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procpathtry_manany: matostr() rs=%d\n",mni) ;
#endif

	        if (mni < 0) continue ;

	        tp = strnpbrk(ds.name,dnl,"123456789") ;
	        if (tp == NULL) continue ;

/* optional test */

	        rs1 = pathmk(ptp,ds.name,dnl) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procpathtry_manany: pathmk() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0) continue ;

	        rs1 = uc_stat(ptp->fname,sbp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("procpathtry_manany: mandir=%s\n",ptp->fname) ;
	            debugprintf("procpathtry_manany: uc_stat() rs=%d\n",rs1) ;
	        }
#endif

	        if ((rs1 < 0) || (! S_ISDIR(usbp->st_mode))) continue ;

/* required test */

	        rs1 = pathmkman(ptp,mannames[mni],tp) ;

	        if (rs1 > 0)
	            rs1 = uc_stat(ptp->fname,sbp) ;

	        if (rs1 >= 0) {
	            rs1 = SR_ACCESS ;
	            if (S_ISREG(usbp->st_mode))
	                rs1 = sperm(&lip->id,sbp,R_OK) ;
	        }

/* print anything found out */

	        if (rs1 >= 0) {
	            rs = printit(pip,ptp->ofp,ptp->fname) ;
	            c += 1 ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    fsdir_close(&d) ;
	    ptp->fname[ptp->pathlen] = '\0' ;
	} /* end if (fsdir) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_manany) */


static int procpathtry_inc(PROGINFO *pip,PATHTRY *ptp)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	*usbp = ptp->sbp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs1 = pathmkinc(ptp,ptp->name,ptp->namelen)) > 0) {
	    if ((rs1 = uc_stat(ptp->fname,usbp)) >= 0) {
	        if S_ISREG(usbp->st_mode) {
	            if ((rs1 = sperm(&lip->id,usbp,R_OK)) >= 0) {
	                rs = printit(pip,ptp->ofp,ptp->fname) ;
	                c += 1 ;
	            }
	        }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procpathtry_inc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_inc) */


static int procpathtry_xfile(PROGINFO *pip,PATHTRY *ptp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if (ptp == NULL) return SR_FAULT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathtry_xfile) */


static int pathmk(PATHTRY *ptp,cchar *np,int nl)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	int		lr = (MAXPATHLEN - ptp->pathlen) ;
	char		*fp = (ptp->fname + ptp->pathlen) ;

	if ((rs = sbuf_start(&b,fp,lr)) >= 0) {

	    if (ptp->fname[0] != '\0')
	        sbuf_char(&b,'/') ;

	    sbuf_strw(&b,np,nl) ;

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pathmk) */


static int pathmkman(PATHTRY *ptp,cchar *mname,cchar *sname)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	int		lr = (MAXPATHLEN - ptp->pathlen) ;
	char		*fp = (ptp->fname + ptp->pathlen) ;

	if ((rs = sbuf_start(&b,fp,lr)) >= 0) {

	    if (ptp->fname[0] != '\0')
	        sbuf_char(&b,'/') ;

	    sbuf_strw(&b,mname,-1) ;

	    sbuf_strw(&b,sname,-1) ;

	    sbuf_char(&b,'/') ;

	    sbuf_strw(&b,ptp->name,ptp->namelen) ;

	    if (sname[0] != '\0')
	        sbuf_char(&b,'.') ;

	    sbuf_strw(&b,sname,-1) ;

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	}  /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pathmkman) */


static int pathmkinc(PATHTRY *ptp,cchar *np,int nl)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	int		lr = (MAXPATHLEN - ptp->pathlen) ;
	char		*fp = (ptp->fname + ptp->pathlen) ;

	if ((rs = sbuf_start(&b,fp,lr)) >= 0) {

	    if (ptp->fname[0] != '\0')
	        sbuf_char(&b,'/') ;

	    sbuf_strw(&b,np,nl) ;

	    sbuf_strw(&b,".h",2) ;

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	}  /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pathmkinc) */


static int pathmklib(PATHTRY *ptp,cchar *pre,cchar *np,int nl,cchar *suf)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	int		lr = (MAXPATHLEN - ptp->pathlen) ;
	char		*fp = (ptp->fname + ptp->pathlen) ;

	if ((rs = sbuf_start(&b,fp,lr)) >= 0) {

	    if (ptp->fname[0] != '\0')
	        sbuf_char(&b,'/') ;

	    if (pre != NULL)
	        sbuf_strw(&b,pre,-1) ;

	    sbuf_strw(&b,np,nl) ;

	    if (suf != NULL) {
	        sbuf_char(&b,'.') ;
	        sbuf_strw(&b,suf,-1) ;
	    }

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pathmklib) */


/* process a qualified (path-qualified) name */
static int procqualname(PROGINFO *pip,SHIO *ofp,cchar *fname)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (uc_stat(fname,&sb) >= 0) {

	    if ((rs >= 0) && lip->pt.c) {
	        if (S_ISDIR(sb.st_mode)) {
	            rs = printit(pip,ofp,fname) ;
	            c += 1 ;
	        }
	    } /* end if */

	    if ((rs >= 0) && lip->pt.e) {
	        if (S_ISREG(sb.st_mode) &&
	            (sperm(&lip->id,&sb,X_OK) >= 0)) {
	            rs = printit(pip,ofp,fname) ;
	            c += 1 ;
	        }
	    } /* end if */

	    if ((rs >= 0) && lip->pt.f) {
	        if (S_ISREG(sb.st_mode)) {
	            rs = printit(pip,ofp,fname) ;
	            c += 1 ;
	        }
	    } /* end if */

	} /* end if (stat) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procqualname) */


/* print it out */
static int printit(PROGINFO *pip,SHIO *ofp,cchar *fname)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("printit: ent fname=%s\n",fname) ;
#endif

	if ((! lip->f_q) && (pip->verboselevel > 0)) {
	    if ((rs = locinfo_notdone(lip,1)) > 0) {
	        rs = shio_printf(ofp,"%s\n",fname) ;
	        wlen += rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printit) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;
	int		size ;

	size = sizeof(LOCINFO) ;
	memset(lip,0,size) ;
	lip->pip = pip ;
	lip->nmax = -1 ;
	lip->po_pathnames = PO_PATHNAMES ;
	lip->po_sections = PO_SECTIONS ;

	rs = paramopt_start(&lip->lists) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = paramopt_finish(&lip->lists) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_notdone(LOCINFO *lip,int inc)
{
	const int	f = (lip->nmax < 0) || (lip->n < lip->nmax) ;
	lip->n += inc ;
	return f ;
}
/* end subroutine (locinfo_notdone) */


static int locinfo_defspec(LOCINFO *lip)
{
	int		i ;
	int		c = 0 ;
	int		f ;

	for (i = 1 ; i < 10 ; i += 1) {
	    f = FALSE ;
	    switch (i) {
	    case 1:
	        f = lip->pt.c ;
	        break ;
	    case 2:
	        f = lip->pt.e ;
	        break ;
	    case 3:
	        f = lip->pt.f ;
	        break ;
	    case 4:
	        f = lip->pt.l ;
	        break ;
	    case 5:
	        f = lip->pt.m ;
	        break ;
	    case 6:
	        f = lip->pt.i ;
	        break ;
	    case 7:
	        f = lip->pt.x ;
	        break ;
	    case 9:
	        f = lip->pt.xu ;
	        break ;
	    } /* end switch */
	    if (f) c += 1 ;
	} /* end for */

	if (c == 0) {
	    lip->pt.e = TRUE ;
	    c += 1 ;
	}

#if	CF_DEBUG
	{
	    PROGINFO	*pip = lip->pip ;
	    if (DEBUGLEVEL(3))
	        debugprintf("b_pt/locinfo_defspec: ret c=%u\n",c) ;
	}
#endif

	return c ;
}
/* end subroutine (locinfo_defspec) */


static int locinfo_pathspecs(LOCINFO *lip,int nmax)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*kn = PO_PATHNAMES ;

/* maximum entries to print out */

	lip->nmax = nmax ;

/* local up selected paths */

	if ((rs >= 0) && lip->pt.c) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARCDPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.e) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.f) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARFPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.l) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARLIBPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.m) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARMANPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.i) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARINCPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.x) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARXFILESEARCHPATH,-1) ;
	}

	if ((rs >= 0) && lip->pt.xu) {
	    c += 1 ;
	    rs = paramopt_load(&lip->lists,kn, VARXUSERFILESEARCHPATH,-1) ;
	}

#if	CF_DEBUG
	{
	    PROGINFO	*pip = lip->pip ;
	    if (DEBUGLEVEL(3))
	        debugprintf("b_pt/locinfo_pathspecs: ret rs=%d c=%u\n",rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_pathspecs) */


static int locinfo_pathdef(LOCINFO *lip)
{
	PARAMOPT	*pop = &lip->lists ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*kn = PO_PATHNAMES ;

	if ((rs = paramopt_fetch(pop,kn,NULL,NULL)) == SR_NOTFOUND) {
	    c += 1 ;
	    rs = paramopt_load(pop,kn,VARPATH,-1) ;
	} /* end if */

#if	CF_DEBUG
	{
	    PROGINFO	*pip = lip->pip ;
	    if (DEBUGLEVEL(3))
	        debugprintf("b_pt/locinfo_pathdef: ret rs=%d c=%u\n",rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_pathdef) */


static int locinfo_idbegin(LOCINFO *lip)
{
	int		rs ;
	rs = ids_load(&lip->id) ;
	return rs ;
}
/* end subroutine (locinfo_idbegin) */


static int locinfo_idend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = ids_release(&lip->id) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (locinfo_idend) */


static int isenvok(cchar *sp)
{
	if (*sp == '\0') return FALSE ;
	while (*sp) {
	    const int	sch = MKCHAR(*sp) ;
	    if (! isprintlatin(sch)) break ;
	    if ((*sp == '\"') || (*sp == '=')) break ;
	    sp += 1 ;
	} /* end while */
	return (*sp == '\0') ? TRUE : FALSE ;
}
/* end subroutine (isenvok) */


