/* b_mxalias */

/* front-end subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little program looks up MAILX aliases and prints out the
	corresponding values.

	Synopsis:

	$ mxalias <alias(es)>


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
#include	<estrings.h>
#include	<ascii.h>
#include	<hdbstr.h>
#include	<getxusername.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_mxalias.h"
#include	"defs.h"
#include	"mxalias.h"


/* local defines */

#define	PO_PATHNAMES	"pathnames"
#define	PO_SECTIONS	"sections"

#ifndef	DEFMAFNAME
#define	DEFMAFNAME	"default"
#endif

#ifndef	KCOLEXP
#define	KCOLEXP		16
#endif

#define	LOCINFO_MAGIC	0x99224571
#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	haswhite(const char *,int) ;
extern int	isalphalatin(int) ;
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
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		unresolved:1 ;	/* un-resolved addresses */
	uint		addr:1 ;	/* output-address mode */
	uint		audit:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PARAMOPT	lists ;
	MXALIAS		madb ;
	HDBSTR		addrs ;
	char		username[USERNAMELEN + 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procname(PROGINFO *,SHIO *,const char *,int) ;
static int	procmxdump(PROGINFO *,const char *) ;
static int	procmxprint(PROGINFO *,SHIO *,cchar *,cchar *) ;
static int	procvalprint(PROGINFO *,SHIO *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_outadd(LOCINFO *,const char *,int) ;
static int	locinfo_outprint(LOCINFO *,SHIO *) ;


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
	"if",
	"unresolved",
	"dump",
	"df",
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
	argopt_if,
	argopt_unresolved,
	argopt_dump,
	argopt_df,
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
	"unresolved",
	"addr",
	"audit",
	NULL
} ;

enum progopts {
	progopt_unresolved,
	progopt_addr,
	progopt_audit,
	progopt_overlast
} ;

static const char	blanks[] =
	"                    "
	"                    " ;


/* exported subroutines */


int b_mxalias(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_mxalias) */


int p_mxalias(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mxalias) */


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
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*dfname = NULL ;
	const char	*un = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_mxalias: starting DFD=%d\n",rs) ;
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

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cp = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* dump file */
	                case argopt_dump:
	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_unresolved:
	                    lip->f.unresolved = TRUE ;
	                    lip->final.unresolved = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.unresolved = (rs > 0) ;
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

/* output-address mode */
	                    case 'a':
	                        lip->f.addr = TRUE ;
	                        lip->final.addr = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.addr = (rs > 0) ;
	                            }
	                        }
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

/* username */
	                    case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                un = argp ;
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

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mxalias: debuglevel=%d\n",pip->debuglevel) ;
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

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* option parsing */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: verboselevel=%d\n",
		pip->progname,pip->verboselevel) ;
	}

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mxalias: done w/ proc options\n") ;
#endif

	pip->username = lip->username ; /* initialized in 'locinfo_start()' */

/* open the database */

	if ((un == NULL) || (un[0] == '\0')) {
	    un = pip->username ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = mxalias_open(&lip->madb,pip->pr,un)) >= 0) {

	        if (dfname != NULL) {
	            rs = procmxdump(pip,dfname) ;
	        } else {
	            const char	*ofn = ofname ;
	            const char	*afn = afname ;
	            if (lip->f.addr) {
	                if ((rs = hdbstr_start(&lip->addrs,10)) >= 0) {
	                    lip->open.addr = TRUE ;
	                }
	            } /* end if */
	            if (rs >= 0) {

	                rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;

	                if (lip->open.addr) {
	                    lip->open.addr = FALSE ;
	                    rs1 = hdbstr_finish(&lip->addrs) ;
	                    if (rs >= 0) rs = rs1 ;
	                }
	            } /* end if (ok) */
	        } /* end if (alternative modes) */

	        rs1 = mxalias_close(&lip->madb) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        ex = EX_UNAVAILABLE ;
	        shio_printf(pip->efp,"%s: MX-aliases unavailable (%d)\n",
	            pip->progname,rs) ;
	    }
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mxalias: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("b_mxalias: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("b_mxalias: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("b_mxalias: "
	            "MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("b_mxalias: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("b_mxalias: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("b_mxalias: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("b_mxalias: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("b_mxalias: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("b_mxalias: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("b_mxalias: "
	            "MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("b_mxalias: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("b_mxalias: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("b_mxalias: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        }
	        ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad arg */
badarg:
	ex = EX_USAGE ;
	if (pip->efp != NULL) {
	    shio_printf(pip->efp,
	        "%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	}
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

	    fmt = "%s: USAGE> %s [<name(s)> ...] [-af <afile>]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-u <username>] [-dump <dfile>]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if (error-enabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0, sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->f.unresolved = TRUE ;

	if ((rs = getusername(lip->username,USERNAMELEN,-1)) >= 0) {
	    rs = paramopt_start(&lip->lists) ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.addr) {
	    lip->f.addr = FALSE ;
	    rs1 = hdbstr_finish(&lip->addrs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = paramopt_finish(&lip->lists) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_outadd(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	const int	nrs = SR_NOTFOUND ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if ((rs = hdbstr_fetch(&lip->addrs,vp,vl,NULL,NULL)) == nrs) {
	    rs = hdbstr_add(&lip->addrs,vp,vl,vp,0) ;
	}

	return rs ;
}
/* end subroutine (locinfo_outadd) */


static int locinfo_outprint(LOCINFO *lip,SHIO *ofp)
{
	PROGINFO	*pip = lip->pip ;
	HDBSTR_CUR	cur ;
	int		rs ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if ((rs = hdbstr_curbegin(&lip->addrs,&cur)) >= 0) {
	    int		kl ;
	    cchar	*kp ;

	    while ((kl = hdbstr_enum(&lip->addrs,&cur,&kp,NULL,NULL)) >= 0) {

	        if (kl > 0) {
	            rs = shio_printf(ofp,"%t\n",kp,kl) ;
	            wlen += rs ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    hdbstr_curend(&lip->addrs,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_outprint) */


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

	                case progopt_unresolved:
	                    if (! lip->final.unresolved) {
	                        lip->f.unresolved = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.unresolved = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case progopt_addr:
	                    if (! lip->final.addr) {
	                        lip->f.addr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addr = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case progopt_audit:
	                    if (! lip->final.audit) {
	                        lip->f.audit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.audit = (rs > 0) ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"r",0666)) >= 0) {
	    LOCINFO	*lip = (LOCINFO *) pip->lip ;
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
	                if (rs == SR_NOENT) {
	                    fmt = "%s: variable not present (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                } else {
	                    fmt = "%s: error processing variable (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                }
	                fmt = "%s: var=%s\n" ;
	                shio_printf(pip->efp,fmt,pn,cp) ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn= STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            int		sl ;
	            const char	*tp, *sp ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                sp = lbuf ;
	                sl = len ;
	                if ((tp = strnchr(sp,sl,'#')) != NULL) {
	                    sl = (tp - sp) ;
	                }

	                pan += 1 ;
	                while ((cl = nextfield(sp,sl,&cp)) > 0) {

	                    rs = procname(pip,ofp,cp,cl) ;
	                    c += rs ;

	                    sl -= ((cp + cl) - sp) ;
	                    sp = (cp + cl) ;

	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while */

	                if (rs < 0) {
	                    if (rs == SR_NOENT) {
	                        fmt = "%s: variable not present (%d)\n" ;
	                        shio_printf(pip->efp,fmt,pn,rs) ;
	                    } else {
	                        fmt = "%s: error processing variable (%d)\n" ;
	                        shio_printf(pip->efp,fmt,pn,rs) ;
	                    }
	                    fmt = "%s: var=%s\n" ;
	                    shio_printf(pip->efp,fmt,pn,cp) ;
	                }

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

	    if ((rs >= 0) && lip->f.addr) {
	        rs = locinfo_outprint(lip,ofp) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


/* process a name */
static int procname(PROGINFO *pip,SHIO *ofp,cchar np[],int nl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if ((nl == 0) || (np[0] == '\0'))
	    return SR_INVALID ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: query=%t\n",
	        pip->progname,np,nl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mxalias/procname: query=%t\n",np,nl) ;
#endif

	if (! lip->f.addr) {
	    rs = shio_printf(ofp,"%t:\n",np,nl) ;
	}

	if (rs >= 0) {
	    MXALIAS_CUR	cur ;
	    if ((rs = mxalias_curbegin(&lip->madb,&cur)) >= 0) {

	        if ((rs = mxalias_lookup(&lip->madb,&cur,np,nl)) > 0) {
	            const int	vlen = VBUFLEN ;
	            int		vl ;
	            char	vbuf[VBUFLEN + 1] ;

	            while (rs >= 0) {
	                vl = mxalias_read(&lip->madb,&cur,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;

	                if (rs >= 0) {
	                    c += 1 ;
	                    if (lip->f.addr) {
	                        rs = locinfo_outadd(lip,vbuf,vl) ;
	                    } else
	                        rs = procvalprint(pip,ofp,vbuf,vl) ;
	                } /* end if (ok) */

	            } /* end while */

	        } else if (((rs == 0) || (rs == SR_NOTFOUND)) &&
	            lip->f.unresolved && lip->f.addr) {

	            rs = locinfo_outadd(lip,np,nl) ;

	        } else if (rs == SR_NOTFOUND)
	            rs = SR_OK ;

	        rs1 = mxalias_curend(&lip->madb,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


static int procmxdump(PROGINFO *pip,cchar dfname[])
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	SHIO		dumpfile, *dfp = &dumpfile ;
	int		rs, rs1 ;
	int		c = 0 ;

	if (dfname == NULL) return SR_FAULT ;

	if (dfname[0] == '\0')
	    return SR_OK ;

	if (dfname[0] == '-')
	    dfname = STDOUTFNAME ;

	if ((rs = shio_open(dfp,dfname,"rwct",0666)) >= 0) {
	    MXALIAS_CUR	cur ;

/* dump the database */

	    if ((rs = mxalias_curbegin(&lip->madb,&cur)) >= 0) {
	        const int	klen = KBUFLEN ;
	        const int	vlen = VBUFLEN ;
	        char		kbuf[KBUFLEN + 1] ;
	        char		vbuf[VBUFLEN + 1] ;

	        while (rs >= 0) {

	            rs1 = mxalias_enum(&lip->madb,&cur,kbuf,klen,vbuf,vlen) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;

	            if (rs >= 0) {
	                c += 1 ;
	                rs = procmxprint(pip,dfp,kbuf,vbuf) ;
	            }

	        } /* end while */

	        rs1 = mxalias_curend(&lip->madb,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

	    rs1 = shio_close(dfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dump-file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmxdump) */


/* print out a "dump" mail-alias entry */
static int procmxprint(PROGINFO *pip,SHIO *dfp,cchar *kbuf,cchar *vbuf)
{
	const int	ch = MKCHAR(vbuf[0]) ;
	int		rs ;
	int		klen ;
	int		blen ;
	int		f ;

	klen = strlen(kbuf) ;

	f = ((! isalphalatin(ch)) && (vbuf[0] != ':')) ;

	f = f || haswhite(vbuf,-1) ;

	blen = MAX((KCOLEXP - klen - 1),0) ;
	rs = shio_printf(dfp,"%t:%t\t%s%s%s\n",
	    kbuf,klen,blanks,blen,
	    ((f) ? "\042" : ""),
	    vbuf,
	    ((f) ? "\042" : "")) ;

	return rs ;
}
/* end subroutine (procmxprint) */


static int procvalprint(PROGINFO *pip,SHIO *ofp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*fmt ;
	char		spre[2] ;
	char		ssuf[2] ;

	spre[0] = '\0' ;
	ssuf[0] = '\0' ;
	if (haswhite(vp,vl)) {
	    int	i, ch ;
	    for (i = 0 ; i < 2 ; i += 1) {
	        ch = (i == 0) ? CH_DQUOTE : '\0' ;
	        spre[i] = ch ;
	        ssuf[i] = ch ;
	    }
	}

	fmt = "  %s%t%s\n" ;
	rs = shio_printf(ofp,fmt,spre,vp,vl,ssuf) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procvalprint) */


