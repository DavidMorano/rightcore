/* b_numcvt */
/* lang=C++11 */

/* convert numbers from one base to another */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CFDECMFUI	1		/* use 'cfdecmfui(3dam)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.

	= 2017-09-19, David A­D­ Morano
	Converted this to C++ so that we can use the |ctwords(3dam)| C++
	subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We converts numbers (given as arguments or on STDIN) from one numberic
	base to another.

	This subroutine is fairly standard as front-ends go.  It calls
	|procspec()| to do the real work.


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
#include	<stdlib.h>
#include	<string.h>
#include	<string>

#include	<vsystem.h>
#include	<toxc.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<cfnum.h>
#include	<cfhex.h>
#include	<cfdec.h>
#include	<cfoct.h>
#include	<cfbin.h>
#include	<cthex.h>
#include	<ctdec.h>
#include	<ctoct.h>
#include	<ctbin.h>
#include	<ctroman.h>
#include	<ctwords.hh>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_numcvt.h"
#include	"defs.h"


/* local defines */

#define	OUTBUFLEN	100

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	BASERECORD	struct baserecord


/* name-spaces */

using namespace	std ;


/* typedefs */


/* external subroutines */

extern "C" int	b_numcvt(int,cchar **,void *) ;
extern "C" int	p_numcvt(int,cchar **,cchar **,void *) ;

extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	matostr(const char **,int,const char *,int) ;
extern "C" int	sfshrink(const char *,int,const char **) ;
extern "C" int	cfromani(cchar *,int,int *) ;
extern "C" int	optbool(const char *,int) ;
extern "C" int	optvalue(const char *,int) ;
extern "C" int	bufprintf(char *,int,const char *,...) ;
extern "C" int	hasalldig(cchar *,int) ;
extern "C" int	isdigitlatin(int) ;
extern "C" int	isFailOpen(int) ;
extern "C" int	isNotPresent(int) ;
extern "C" int	isNotValid(int) ;

extern "C" int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern "C" int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strnchr(cchar *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		bufline:1 ;
	uint		uppercase:1 ;	/* for hexadecimal */
	uint		mnz:1 ;		/* modulus w/ no zero value */
} ;

struct locinfo {
	LOCINFO_FL	f, init ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	int		basei ;
	int		baseo ;
	int		mod ;
} ;

struct baserecord {
	const char	*name ;
	int		base ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	findbase(PROGINFO *,const BASERECORD *,cchar *,int) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,SHIO *,cchar *,int) ;
static int	procspec(PROGINFO *,SHIO *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_base(LOCINFO *,cchar *,int) ;

static int	isNotValidBase(int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"set",
	"follow",
	"mnz",
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
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_mnz,
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

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const BASERECORD	bases[] = {
	{ "hexadecimal", 16 },
	{ "decimal", 10 },
	{ "octal", 8 },
	{ "binary", 2 },
	{ "roman", 1 },
	{ "words", 3 },
	{ NULL, 0 }
} ;


/* exported subroutines */


int b_numcvt(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_numcvt) */


int p_numcvt(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_numcvt) */


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
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = STDINFNAME ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*basei = NULL ;
	const char	*baseo = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_numcvt: starting DFD=%d\n",rs) ;
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

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

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

/* non-zero modulus */
	                case argopt_mnz:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        lip->f.mnz = TRUE ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->mod = rs ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
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

/* argument file */
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

/* default action and user specified help */
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

/* quiet */
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

			    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_base(lip,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* default input base */
	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                basei = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* lower case */
	                    case 'l':
	                        lip->f.uppercase = FALSE ;
	                        break ;

/* perform a final modulo operation */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                lip->mod = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* line-buffered */
	                    case 'u':
	                        lip->f.bufline = TRUE ;
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
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badargs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_numcvt: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    shio_printf(efp,"%s: version %s\n",pn,VERSION) ;
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
	if (DEBUGLEVEL(4))
	    debugprintf("b_numcvt: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    shio_printf(efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_numcvt: basei=>%s< baseo=>%s< \n",
	        basei,baseo) ;
#endif

	if ((rs >= 0) && (basei != NULL) && (basei[0] != '\0')) {
	    rs = findbase(pip,bases,basei,-1) ;
	    lip->basei = rs ;
	    if ((rs >= 0) && isNotValidBase(rs)) {
	        rs = SR_INVALID ;
	    }
	} /* end if */

	if ((rs >= 0) && (baseo != NULL) && (baseo[0] != '\0')) {
	    rs = findbase(pip,bases,baseo,-1) ;
	    lip->baseo = rs ;
	    if ((rs >= 0) && isNotValidBase(rs)) {
	        rs = SR_INVALID ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_numcvt: basei=%d baseo=%d\n",
	        lip->basei,lip->baseo) ;
#endif

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*ofn = ofname ;
	    const char	*ifn = ifname ;
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,ifn,afn) ;
	} else if (ex == EX_OK) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_IO:
	        ex = EX_DATAERR ;
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
	    SHIO	*efp = (SHIO *) pip->efp ;
	    shio_printf(efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_numcvt: exiting rs=%d ex=%u\n",rs,ex) ;
#endif

	if (pip->efp != NULL) {
	    SHIO	*efp = (SHIO *) pip->efp ;
	    pip->open.errfile = FALSE ;
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

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_numcvt: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badargs:
	{
	    SHIO	*efp = (SHIO *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    ex = EX_USAGE ;
	    shio_printf(efp,"%s: invalid argument specified (%d)\n",pn,rs) ;
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

	fmt = "%s: USAGE> %s [<value(s)> ...] [-af {<afile>|-}]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-b [<basei>][:<baseo>] [-m <mod>] [-u]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* find the base from a name */
static int findbase(PROGINFO *pip,const BASERECORD *basemap,
		cchar *bname,int blen)
{
	int		rs ; /* initialization (here) not needed */
	int		cl ;
	int		base ;
	const char	*cp ;

#if	CF_DEBUG
	if (pip->debuglevel > 2) {
	    int nl = strnlen(bname,blen) ;
	    debugprintf("findbase: base=>%t<\n",
	        bname,nl) ;
	}
#endif

	if ((cl = sfshrink(bname,blen,&cp)) >= 0) {
	    if (hasalldig(cp,cl)) {
	        rs = cfdeci(cp,cl,&base) ;
		base = rs ;
	    } else {
		int	i ;
	        for (i = 0 ; basemap[i].base > 0 ; i += 1) {
	            if (strncmp(basemap[i].name,cp,cl) == 0) break ;
	        } /* end for */
	        base = basemap[i].base ;
	        rs = (base > 0) ? SR_OK : SR_INVALID ;
	    }
	} else {
	    rs = SR_INVALID ;
	} /* end if (positive) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("findbase: ret rs=%d base=%d\n",
	        rs,base) ;
#endif

	return (rs >= 0) ? base : rs ;
}
/* end subroutine (findbase) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *ifn,
		cchar *afn)
{
	SHIO		*efp = (SHIO *) pip->efp ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    LOCINFO	*lip = (LOCINFO *) pip->lip ;
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (lip->f.bufline)
	        rs = shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;

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
	                    rs = procspec(pip,ofp,cp,-1) ;
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
	            int		len ;
	            char	lbuf[LINEBUFLEN+1] ;

	            if (lip->f.bufline)
	                shio_control(afp,SHIO_CSETBUFLINE,TRUE) ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        if (cl == 5) {
				    if (strncmp(":exit",cp,cl) == 0)
	                                break ;
				}
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
	                    }
	                } /* end if (non-empty) */

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: inaccesible argument-list (%d)\n" ;
	            shio_printf(efp,fmt,pn,rs) ;
	            shio_printf(efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {
	        SHIO	infile, *ifp = &infile ;

	        if ((rs = shio_open(ifp,ifn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN+1] ;

	            if (lip->f.bufline)
	                shio_control(ifp,SHIO_CSETBUFLINE,TRUE) ;

	            while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        if (cl == 5) {
				    if (strncmp(":exit",cp,cl) == 0)
	                                break ;
				}
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
	                    } /* end if (non-comment) */
	                } /* end if (non-empty) */

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(ifp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: inaccessible input (%d)\n" ;
	            shio_printf(efp,fmt,pn,rs) ;
	            shio_printf(efp,"%s: ifile=%s\n",pn,ifn) ;
	        } /* end if (opened input) */

	    } /* end if (processing input) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(efp,fmt,pn,rs) ;
	    shio_printf(efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,SHIO *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,ofp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,SHIO *ofp,cchar *name,int namelen)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	unsigned int	value = 0 ;
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;

	if (name == NULL) return SR_FAULT ;

	if (namelen == 0)
	    return SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	nl = strnlen(name,namelen) ;
	    debugprintf("process: name=>%t<\n",name,nl) ;
	}
#endif

	if ((cl = sfshrink(name,namelen,&cp)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: shrunk name=>%t<\n",cp,cl) ;
#endif

	    rs = SR_INVALID ;
	    if (cp[0] == '\\') {
	        rs = cfnumui(cp,cl,&value) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: num rs=%d v=%08x\n",rs,value) ;
#endif

	    if (isNotValid(rs)) {
	        switch (lip->basei) {
		case 1:
		    {
			int	ival ;
		        rs = cfromani(cp,cl,&ival) ;
			value = ival ;
		    }
		    break ;
	        case 2:
	            rs = cfbinui(cp,cl,&value) ;
	            break ;
	        case 8:
	            rs = cfoctui(cp,cl,&value) ;
	            break ;
	        case 10:
#if	CF_CFDECMFUI
	            rs = cfdecmfui(cp,cl,&value) ;
#else
	            rs = cfdecui(cp,cl,&value) ;
#endif
	            break ;
	        case 16:
	            rs = cfhexui(cp,cl,&value) ;
	            break ;
	        default:
	            rs = SR_INVALID ;
	            break ;
	        } /* end switch */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: default rs=%d v=%08x\n",rs,value) ;
#endif

	    } /* end if (trying bases straight) */

	    if (isNotValid(rs) && (cl > 2) && (cp[0] == '0')) {
		const char	ch1 = MKCHAR(cp[1]) ;

	        if (tolc(ch1) == 'b') {

	            if (cl > 2)
	                rs = cfbinui(cp + 2,cl - 2,&value) ;

	        } else if (tolc(ch1) == 'o') {

	            if (cl > 2)
	                rs = cfoctui(cp + 2,cl - 2,&value) ;

	        } else if (tolc(ch1) == 'd') {

	            if (cl > 2) {
#if	CF_CFDECMFUI
	                rs = cfdecmfui((cp + 2),(cl - 2),&value) ;
#else
	                rs = cfdecui(cp + 2,cl - 2,&value) ;
#endif
	            }

	        } else if (tolc(ch1) == 'x') {

	            if (cl > 2)
	                rs = cfhexui(cp + 2,cl - 2,&value) ;

	        } else if (isdigitlatin(ch1)) {
	            switch (lip->basei) {
	            case 2:
	                rs = cfbinui(cp,cl,&value) ;
	                break ;
	            case 8:
	                rs = cfoctui(cp,cl,&value) ;
	                break ;
	            case 10:
	                rs = cfdecui(cp,cl,&value) ;
	                break ;
	            case 16:
	                rs = cfhexui(cp,cl,&value) ;
	                break ;
	            } /* end switch */
	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process: extra rs=%d v=%08x\n",rs,value) ;
#endif

	    } /* end if (trying '0x' types) */

/* take a modulo operation if requested */

	    if (lip->mod > 0) {
	        value = (value % lip->mod) ;
	        if (lip->f.mnz && (value == 0)) {
	            value = lip->mod ;
		}
	    }

	} /* end if (sfshrink) */

/* format and print the result */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: output rs=%d value=%08x baseo=%u\n",
	        rs,value,lip->baseo) ;
#endif

	if (rs >= 0) {
	    const int	olen = OUTBUFLEN ;
	    int		ol = -1 ;
	    cchar	*obufp = NULL ;
	    char	obuf[OUTBUFLEN + 1] ;
	    obufp = obuf ;
	    switch (lip->baseo) {
	    case 1:
	        rs = ctromani(obuf,olen,value) ;
		ol = rs ;
		break ;
	    case 3:
		{
		    string	s ;
	            if ((rs = ctwords(&s,value)) >= 0) {
			obufp = s.c_str() ;
		    }
		}
		break ;
	    case 2:
	        rs = ctbini(obuf,olen,value) ;
		ol = rs ;
	        break ;
	    case 8:
	        rs = ctocti(obuf,olen,value) ;
		ol = rs ;
	        break ;
	    case 10:
	        rs = ctdecui(obuf,olen,value) ;
		ol = rs ;
	        break ;
	    case 16:
	        if ((rs = cthexi(obuf,olen,value)) > 0) {
		    int	i ;
	            if (lip->f.uppercase) {
	                if ((i = strcspn(obuf,"abcdef")) > 0) {
	                    while (i < 8) {
	                        obuf[i] = touc(obuf[i]) ;
	                        i += 1 ;
	                    }
	                }
	            } else {
	                if ((i = strcspn(obuf,"ABCDEF")) > 0) {
	                    while (i < 8) {
	                        obuf[i] = tolc(obuf[i]) ;
	                        i += 1 ;
	                    }
	                }
	            } /* end if */
		    ol = rs ;
	        } /* end if (case conversion) */
	        break ;
	    } /* end switch */
	    if (rs >= 0) {
	        rs = shio_printf(ofp,"%t\n",obufp,ol) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: result rs=%d obuf=>%t<\n",rs,obuf,rs) ;
#endif

	return rs ;
}
/* end subroutine (procspec) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->basei = DEFBASEI ;
	lip->baseo = DEFBASEO ;
	lip->mod = 0 ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip == NULL)
	    return SR_FAULT ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_base(LOCINFO *lip,cchar *sp,int sl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	cchar		*tp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_numcvt/locinfo_base: b=>%t<\n",sp,sl) ;
#endif

	if ((tp = strnchr(sp,sl,':')) != NULL) {
	    cchar	*cp = sp ;
	    int		cl = (tp-sp) ;
	    if (cl > 0) {
	        rs = findbase(pip,bases,cp,cl) ;
	        lip->basei = rs ;
	    }
	    if (rs >= 0) {
		cp = (tp+1) ;
		cl = ((sp+sl)-(tp+1)) ;
		if (cl > 0) {
	            rs = findbase(pip,bases,cp,cl) ;
	            lip->baseo = rs ;
		}
	    }
	} else {
	    rs = findbase(pip,bases,sp,sl) ;
	    lip->baseo = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("b_numcvt/locinfo_base: ret bi=%u bo=%u\n",
		lip->basei,lip->baseo) ;
	debugprintf("b_numcvt/locinfo_base: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (locinfo_base) */


static int isNotValidBase(int b)
{
	int	f = FALSE ;
	switch (b) {
	case 1:
	case 3:
	case 2:
	case 8:
	case 10:
	case 16:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isNotValidBase) */


