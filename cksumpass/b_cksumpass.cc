/* b_cksumpass (KSH built-in) */
/* lang=C++11 */

/* main subroutine for CKSUMPASS */


#define	CF_DEBUGS	0		/* non-switchables */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 2002-02-01, David A­D­ Morano
	The program was written from scratch.

	= 2017-10-14, David A­D­ Morano
	Turned into a KSH built-in.

*/

/* Copyright © 2002,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.

        Compute a checksum (POSIX 'cksum' style) on the data passing from input
        to output.

	Synospsis:

	$ cksumpass -sf <ansfile> [<file(s)>] > <outfile>


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
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<cksum.h>
#include	<nulstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_cksumpass.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_STAT	struct locinfo_stat


/* external subroutines */

extern "C" int	b_cksumpass(int,cchar **,void *) ;
extern "C" int	p_cksumpass(int,cchar **,cchar **,void *) ;

extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	matostr(const char **,int,const char *,int) ;
extern "C" int	sfshrink(const char *,int,const char **) ;
extern "C" int	sfskipwhite(const char *,int,const char **) ;
extern "C" int	nextfield(const char *,int,const char **) ;
extern "C" int	cfdeci(const char *,int,int *) ;
extern "C" int	cfdecf(const char *,int, double *) ;
extern "C" int	optbool(const char *,int) ;
extern "C" int	optvalue(const char *,int) ;
extern "C" int	isdigitlatin(int) ;
extern "C" int	isFailOpen(int) ;
extern "C" int	isNotPresent(int) ;

extern "C" int	printhelp(void *,const char *,const char *,const char *) ;
extern "C" int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

extern "C" cchar	*getourenv(const char **,const char *) ;

extern "C" char		*strwcpy(char *,const char *,int) ;
extern "C" char		*strnchr(const char *,int,int) ;

extern "C" double	fsum(double *,int) ;
extern "C" double	fam(double *,int) ;
extern "C" double	fhm(double *,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		sum:1 ;
	uint		rfile:1 ;
} ;

struct locinfo_stat {
	longlong_t	bytes = 0 ;
	locinfo_stat() : bytes(0) { 
	} ;
	void clear() {
	    bytes = 0 ;
	} ;
	locinfo_stat &operator += (int a) {
	    bytes += a ;
	    return (*this) ;
	} ;
	locinfo_stat &operator += (const locinfo_stat &a) {
	    bytes += a.bytes ;
	    return (*this) ;
	} ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	LOCINFO_STAT	st, sf ;
	vecstr		stores ;
	CKSUM		sum ;
	SHIO		rfile ;
	PROGINFO	*pip ;
	cchar		*rfname ;
	int		type ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,SHIO *,cchar *,cchar *) ;
static int	procoutfile(PROGINFO *,cchar *) ;
static int	procoutall(PROGINFO *) ;

static int	procsum_begin(PROGINFO *) ;
static int	procsum_end(PROGINFO *) ;
static int	procsum_pass(PROGINFO *,SHIO *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_ckbegin(LOCINFO *) ;
static int	locinfo_ckend(LOCINFO *) ;
static int	locinfo_cktxbegin(LOCINFO *) ;
static int	locinfo_cktxend(LOCINFO *) ;
static int	locinfo_ckaccum(LOCINFO *,cchar *,int) ;
static int	locinfo_repupdate(LOCINFO *,int) ;
static int	locinfo_repbegin(LOCINFO *) ;
static int	locinfo_repend(LOCINFO *) ;
static int	locinfo_getsum(LOCINFO *,uint *) ;
static int	locinfo_getsumall(LOCINFO *,uint *) ;
static int	locinfo_bytesfile(LOCINFO *,ulonglong *) ;
static int	locinfo_bytesall(LOCINFO *,ulonglong *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static const char	*progmodes[] = {
	"cksumpass",
	NULL
} ;

enum progmodes {
	progmode_cksumpass,
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
	"sf",
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
	argopt_sf,
	argopt_overlast
} ;

static const char	*progopts[] = {
	"type",
	NULL
} ;

enum progopts {
	progopt_type,
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


/* exported subroutines */


int b_cksumpass(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_cksumpass) */


int p_cksumpass(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_cksumpass) */


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

/* summary file */
	                case argopt_sf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->rfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->rfname = argp ;
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
	    SHIO	*efp ;
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    efp = (SHIO *) pip->efp ;
	    shio_control(efp,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    shio_printf(efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
		SHIO	*efp = (SHIO *) pip->efp ;
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(efp,fmt,pn,pm) ;
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
	    SHIO	*efp = (SHIO *) pip->efp ;
	    shio_printf(efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

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

	if (lip->rfname == NULL) lip->rfname = getourenv(envv,VARRFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some progopts */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

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
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            SHIO	*efp = (SHIO *) pip->efp ;
	            shio_printf(efp,"%s: invalid query (%d)\n",
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
	    SHIO	*efp = (SHIO *) pip->efp ;
	    shio_printf(efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    pip->open.errfile = TRUE ;
	    shio_close(efp) ;
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
	{
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument specified (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(efp,fmt,pn,rs) ;
	    usage(pip) ;
	}
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	SHIO		*efp = (SHIO *) pip->efp ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-sf <sumfile>] [<file(s)> ...]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af {<afile>|-}] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
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
	                            rs = optvalue(cp,cl) ;
	                            lip->type = rs ;
	                        }
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		cchar *ofn,cchar *afn,cchar *ifn)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0644)) >= 0) {
	    if ((rs = locinfo_repbegin(lip)) >= 0) {
	        if ((rs = procsum_begin(pip)) >= 0) {
	            if ((rs = procargs(pip,aip,bop,ofp,afn,ifn)) >= 0) {
	                wlen += rs ;
	                rs = procoutall(pip) ;
	            }
	            rs1 = procsum_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procsum) */
	        rs1 = locinfo_repend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (locinfo-rep) */
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_cksumpass/process: shio_close() rs1=%d\n",rs1) ;
#endif
	} else {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(efp,fmt,pn,rs) ;
	    shio_printf(efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_cksumpass/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	wlen &= INT_MAX ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,SHIO *ofp,
		cchar *afn,cchar *ifn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		wlen = 0 ;
	int		cl ;
	cchar		*cp ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

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
	                rs = procsum_pass(pip,ofp,cp,-1) ;
	                wlen += rs ;
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
	                    rs = procsum_pass(pip,ofp,cp,cl) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
	            SHIO	*efp = (SHIO *) pip->efp ;
	            fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(efp,fmt,pn,rs) ;
	            shio_printf(efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
		ifn = STDINFNAME ;

	    pan += 1 ;
	    rs = procsum_pass(pip,ofp,ifn,-1) ;
	    wlen += rs ;

	} /* end if (processing STDIN) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procoutfile(PROGINFO *pip,cchar *sfn)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	if (lip->rfname != NULL) {
	    ulonglong	bytes ;
	    if ((rs = locinfo_bytesfile(lip,&bytes)) >= 0) {
	        uint	sv ;
	        if ((rs = locinfo_getsum(lip,&sv)) >= 0) {
	            SHIO	*rfp = &lip->rfile ;
	            rs = shio_printf(rfp,"%10u %14llu %s\n",sv,bytes,sfn) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_cksumpass/procoutfile: "
				"shio_printf() rs=%d\n",rs) ;
#endif
	        }
	    }
	} /* end if (enabled) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cksumpass/procoutfile: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procoutfile) */


static int procoutall(PROGINFO *pip)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	if (lip->rfname != NULL) {
	    ulonglong	bytes ;
	    if ((rs = locinfo_bytesall(lip,&bytes)) >= 0) {
	        uint	sv ;
	        if ((rs = locinfo_getsumall(lip,&sv)) >= 0) {
	            SHIO	*rfp = &lip->rfile ;
	            rs = shio_printf(rfp,"%10u %14llu TOTAL\n",sv,bytes) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_cksumpass/procoutall: "
			    "shio_printf() rs=%d\n", rs) ;
#endif
	        }
	    }
	} /* end if (enabled) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cksumpass/procoutall: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procoutall) */


static int procsum_begin(PROGINFO *pip)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs ;

	rs = locinfo_ckbegin(lip) ;

#if	CF_DEBUGS
	debugprintf("main/procsum_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsum_begin) */


static int procsum_end(PROGINFO *pip)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = locinfo_ckend(lip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("main/procsum_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsum_end) */


static int procsum_pass(PROGINFO *pip,SHIO *ofp,cchar *np,int nl)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	NULSTR		fs ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*sfn ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_cksumpass/procsum_pass: ent n=%t\n",np,nl) ;
	    debugprintf("b_cksumpass/procsum_pass: open.sum=%u\n",
		lip->open.sum) ;
	}
#endif

	if ((rs = nulstr_start(&fs,np,nl,&sfn)) >= 0) {
	    SHIO	sfile, *sfp = &sfile ;
	    if ((sfn[0] == '\0') || (sfn[0] == '-')) sfn = STDINFNAME ;
	    if ((rs = shio_open(sfp,sfn,"r",0666)) >= 0) {
	        const int	slen = getpagesize() ;
	        char	*sbuf ;
	        if ((rs = uc_malloc(slen,&sbuf)) >= 0) {
	            if ((rs = locinfo_cktxbegin(lip)) >= 0) {
	                while ((rs = shio_read(sfp,sbuf,slen)) > 0) {
	                    const int	len = rs ;
	                    if ((rs = locinfo_ckaccum(lip,sbuf,len)) >= 0) {
	                        if ((rs = shio_write(ofp,sbuf,len)) >= 0) {
	                            wlen += rs ;
	                            rs = locinfo_repupdate(lip,len) ;
	                        }
	                    } /* end if (locinfo_ckaccum) */
	                    if (rs < 0) break ;
	                } /* end while */
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_cksumpass/procsum_pass: "
				"while-out rs=%d\n",rs) ;
#endif
	                if (rs >= 0) {
	                    rs = procoutfile(pip,sfn) ;
	                }
	                rs1 = locinfo_cktxend(lip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (locinfo-ckfile) */
	            uc_free(sbuf) ;
	        } /* end if (m-a-f) */
	        rs1 = shio_close(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        SHIO	*efp = (SHIO *) pip->efp ;
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: inaccessible input (%d)\n" ;
	        shio_printf(efp,fmt,pn,rs) ;
	        shio_printf(efp,"%s: ifile=%t\n",pn,np,nl) ;
	    } /* end if (shio) */
	    rs1 = nulstr_finish(&fs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cksumpass/procsum_pass: ret rs=%d\n",rs) ;
#endif

	wlen &= INT_MAX ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsum_pass) */


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


static int locinfo_ckbegin(LOCINFO *lip)
{
	int		rs ;
	if ((rs = cksum_start(&lip->sum)) >= 0) {
	    lip->open.sum = TRUE ;
	}
	return rs ;
}
/* end subroutine (locinfo_ckbegin) */


static int locinfo_ckend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.sum) {
	    rs1 = cksum_finish(&lip->sum) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->open.sum = FALSE ;
	}
	return rs ;
}
/* end subroutine (locinfo_ckend) */


static int locinfo_cktxbegin(LOCINFO *lip)
{
	lip->sf.clear() ;
	return cksum_begin(&lip->sum) ;
}
/* end subroutine (locinfo_cktxbegin) */


static int locinfo_cktxend(LOCINFO *lip)
{
	lip->st += lip->sf ;
	return cksum_end(&lip->sum) ;
}
/* end subroutine (locinfo_cktxend) */


static int locinfo_ckaccum(LOCINFO *lip,cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	if (lip->open.sum) {
	    const void	*bp = (const void *) sbuf ;
	    rs = cksum_accum(&lip->sum,bp,slen) ;
	}
	return rs ;
}
/* end subroutine (locinfo_ckaccum) */


static int locinfo_repbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->rfname != NULL) {
	    cchar	*rfn = lip->rfname ;
	    if (rfn != NULL) {
	        SHIO		*rfp = &lip->rfile ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_cksumpass/locinfo_repbegin: rfn=%s\n",rfn) ;
#endif
	        if ((rfn[0] == '\0') || (rfn[0] == '-')) rfn = STDOUTFNAME ;
	        if ((rs = shio_open(rfp,rfn,"wct",0666)) >= 0) {
	            lip->open.rfile = TRUE ;
	        } /* end if (shio-report) */
	    } /* end if (non-null) */
	} /* end if (enabled) */
	return rs ;
}
/* end subroutine (locinfo_repbegin) */


static int locinfo_repend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->open.rfile) {
	    SHIO	*rfp = (SHIO *) &lip->rfile ;
	    rs1 = shio_close(rfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_cksumpass/locinfo_repend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_repend) */


static int locinfo_repupdate(LOCINFO *lip,int len)
{
	int		rs = SR_OK ;
	lip->sf += len ;
	return rs ;
}
/* end subroutine (locinfo_repupdate) */


static int locinfo_getsum(LOCINFO *lip,uint *rp)
{
	int		rs = SR_OK ;
	if (lip->open.sum) {
	    rs = cksum_getsum(&lip->sum,rp) ;
	}
	return rs ;
}
/* end subroutine (locinfo_getsum) */


static int locinfo_getsumall(LOCINFO *lip,uint *rp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->open.sum) {
	    rs = cksum_getsumall(&lip->sum,rp) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_cksumpass/locinfo_getsumall: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_getsumall) */


static int locinfo_bytesfile(LOCINFO *lip,ulonglong *bytep)
{
	*bytep = lip->sf.bytes ;
	return SR_OK ;
}
/* end subroutine (locinfo_bytesfile) */


static int locinfo_bytesall(LOCINFO *lip,ulonglong *bytep)
{
	*bytep = lip->st.bytes ;
	return SR_OK ;
}
/* end subroutine (locinfo_bytesall) */


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


