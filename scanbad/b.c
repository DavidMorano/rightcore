/* b_scanbad */

/* program to return a user's home login directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_BADBLOCK	0		/* try some bad-block handling */
#define	CF_LOCSETENT	0		/* "locinfo_setentry()| */


/* revision history:

	= 2004-07-14, David A­D­ Morano
	This was written inspired by a previous program that did about the same
	function as this.  But this was written a fresh as a KSH builtin.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ scanbad [file(s) ...]


*******************************************************************************/


#include	<envstandards.h>

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
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_scanbad.h"
#include	"defs.h"


/* local defines */

#ifndef	UBLOCKLEN
#define	UBLOCKLEN	512		/* UNIX® block-size */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROCDATA	struct procdata


/* external subroutines */

extern uint	uceil(uint,int) ;

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecmfll(cchar *,int,LONG *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	msleep(int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isStrEmpty(cchar *,int) ;

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
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		mapfile:1 ;
	uint		nice:1 ;
	uint		scanoff:1 ;
	uint		scanlen:1 ;
	uint		gap:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	char		*rdata ;
	offset_t	scanoff ;
	offset_t	scanlen ;
	size_t		rsize ;
	uint		blocksize ;
	int		pagesize ;
	int		nice ;
	int		gap ;
} ;

struct procdata {
	int		blocklen ;
	int		maxbad ;
	int		nblocks ;
	int		nbad ;
} ;


/* forward references */

static int	mainsub(int,cchar *[],cchar *[],void *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_allocbegin(LOCINFO *,int) ;
static int	locinfo_allocend(LOCINFO *) ;
static int	locinfo_scanspec(LOCINFO *,cchar *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,PROCDATA *,
			cchar *,cchar *) ;
static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procfile(PROGINFO *,PROCDATA *,void *,cchar *) ;


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
	"bs",
	"nice",
	"gap",
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
	argopt_bs,
	argopt_nice,
	argopt_gap,
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

static const char	*progopts[] = {
	"nice",
	"scanoff",
	"scanlen",
	"gap",
	NULL
} ;

enum progopts {
	progopt_nice,
	progopt_scanoff,
	progopt_scanlen,
	progopt_gap,
	progopt_overlast
} ;


/* exported subroutines */


int b_scanbad(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_scanbad) */


int p_scanbad(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_scanbad) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	PROCDATA	pd ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v ;
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
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_scanbad: starting DFD=%d\n",rs) ;
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

	memset(&pd,0,sizeof(PROCDATA)) ;
	pd.blocklen = BLOCKLEN ;

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

	                case argopt_bs:
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        rs = cfdecmfi(cp,cl,&v) ;
	                        pd.blocklen = v ;
	                    }
	                    break ;

	                case argopt_nice:
	                    lip->final.nice = TRUE ;
	                    lip->have.nice = TRUE ;
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->nice = rs ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_gap:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            int		v ;
	                            lip->final.gap = TRUE ;
	                            lip->have.gap = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            lip->gap = v ;
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

	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pd.maxbad = rs ;
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

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_scanspec(lip,argp,argl) ;
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

	if (rs < 0) goto badarg ;

	if (pip->debuglevel == 0) {
	    if ((cp = getourenv(envv,VARDEBUGLEVEL)) != NULL) {
	        if (hasnonwhite(cp,-1)) {
		    rs = optvalue(cp,-1) ;
		    pip->debuglevel = rs ;
	        }
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arhuments */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (pd.blocklen < UBLOCKLEN) pd.blocklen = UBLOCKLEN ;

	if ((rs >= 0) && (lip->scanlen == 0)) {
	    if ((cp = getourenv(envv,VARSCANSPEC)) != NULL) {
	        rs = locinfo_scanspec(lip,cp,-1) ;
	    }
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp, "%s: blocksize=%u\n",pn,pd.blocklen) ;
	    if (lip->gap > 0) {
	        shio_printf(pip->efp, "%s: gap=%d\n",pn,lip->gap) ;
	    }
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    LONG	som = (lip->scanoff / (1024*1024)) ;
	    LONG	slm = (lip->scanlen / (1024*1024)) ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: so=%lldm sl=%lldm\n" ;
	    if (lip->scanlen == LONG64_MAX) {
	        fmt = "%s: so=%lldm sl=Inf\n" ;
	    }
	    rs = shio_printf(pip->efp,fmt,pn,som,slm) ;
	} /* end if (debugging information) */

	if ((rs >= 0) && (lip->nice > 0)) {
	    rs = u_nice(lip->nice) ;
	}

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = locinfo_allocbegin(lip,pd.blocklen)) >= 0) {
	        {
		    ARGINFO	*aip = &ainfo ;
		    BITS	*bop = &pargs ;
	            cchar	*ofn = ofname ;
	            cchar	*afn = afname ;
	            rs = procargs(pip,aip,bop,&pd,ofn,afn) ;
	        }
	        rs1 = locinfo_allocend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (locinfo-alloc) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
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

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_scanbad: exiting ex=%u (%d)\n",ex,rs) ;
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
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_scanbad: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,PROCDATA *pdp,
		cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    cchar	*cp ;

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
	                    rs = procfile(pip,pdp,ofp,cp) ;
			}
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */
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

	                if (len > 0) {
	                    pan += 1 ;
	                    rs = procfile(pip,pdp,ofp,lbuf) ;
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

	    if ((rs >= 0) && (pan == 0)) {
	        rs = SR_INVALID ;
	        shio_printf(pip->efp,"%s: no files specified\n",pn) ;
	    }

	    if (pip->verboselevel >= 2) {
	        shio_printf(ofp,"files scanned=%u\n",pan) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return rs ;
}
/* end subroutine (procargs) */


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

	            if ((oi = matostr(progopts,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case progopt_nice:
	                    if (! lip->final.nice) {
	                        if (vl > 0) {
	                            lip->final.nice = TRUE ;
	                            lip->have.nice = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            lip->nice = rs ;
	                        }
	                    }
	                    break ;

	                case progopt_scanoff:
	                    if ( ! lip->final.scanoff) {
	                        if (vl > 0) {
	                            LONG	lv ;
	                            lip->final.scanoff = TRUE ;
	                            lip->have.scanoff = TRUE ;
	                            rs = cfdecmfll(vp,vl,&lv) ;
	                            lip->scanoff = lv ;
	                        }
	                    }
	                    break ;

	                case progopt_scanlen:
	                    if ( ! lip->final.scanlen) {
	                        if (vl > 0) {
	                            LONG	lv ;
	                            lip->final.scanlen = TRUE ;
	                            lip->have.scanlen = TRUE ;
	                            rs = cfdecmfll(vp,vl,&lv) ;
	                            lip->scanlen = lv ;
	                        }
	                    }
	                    break ;

	                case progopt_gap:
	                    if (! lip->final.gap) {
	                        if (vl > 0) {
	                            int		v ;
	                            lip->final.gap = TRUE ;
	                            lip->have.gap = TRUE ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->gap = v ;
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


/* process a file */
static int procfile(PROGINFO *pip,PROCDATA *pdp,void *ofp,cchar *ifname)
{
	LOCINFO		*lip = pip->lip ;
	offset_t	scanoff, scanlen ;
	offset_t	scanmax = LONG64_MAX ;
	offset_t	roff = 0 ;
	offset_t	tlen = 0 ;
	int		rs ;
	int		blocklen = pdp->blocklen ;
	int		nblocks = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		*blockbuf = lip->rdata ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfile: ifname=%s\n",ifname) ;
#endif

/* offset and length of scan */

	scanoff = lip->scanoff ;
	scanlen = lip->scanlen ;
	if (scanlen > 0) scanmax = (scanoff + scanlen) ;

/* go w/ file open */

	if ((ifname[0] == '\0') || (strcmp(ifname,"-") == 0))
	    return SR_INVALID ;

	if ((rs = u_open(ifname,O_RDONLY,0666)) >= 0) {
	    const int	fd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procfile: opened\n") ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procfile: seeking\n") ;
#endif

	    if (scanoff > 0) {
	        roff = scanoff ;
	        rs = u_seek(fd,scanoff,SEEK_SET) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("main/procfile: start roff=%lld rlen=%lld\n",
	            roff,scanlen) ;
	        debugprintf("main/procfile: start scanmax=%lld\n",
	            scanmax) ;
	    }
#endif

/* top of loop */

	    if (rs >= 0) {
	        int	rlen = 0 ;

	        while ((rs >= 0) && (roff < scanmax)) {
	            rs = u_read(fd,blockbuf,blocklen) ;
	            rlen = rs ;
	            if (rs <= 0) break ;
	            roff += rlen ;
	            tlen += rlen ;
	            nblocks += 1 ;
	            if (lip->gap > 0) rs = msleep(lip->gap) ;
	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	        } /* end while */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("main/procfile: read-out rlen=%d\n",rlen) ;
	            debugprintf("main/procfile: nblocks=%u\n",nblocks) ;
	            debugprintf("main/procfile: tlen=%llu\n",tlen) ;
		}
#endif

	    } /* end if (ok) */

	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfile: open-out rs=%d tlen=%lldm\n",
	        rs,(tlen/(1024*1024))) ;
#endif

	if (pip->verboselevel > 1) {
	    shio_printf(ofp,"file=%s (%d) blocks=%u\n",
	        ifname,rs,nblocks) ;
	}

	if ((rs < 0) && (! pip->f.quiet)) {
	    shio_printf(pip->efp,"%s: error (%d) file=%s\n",
	        pip->progname,rs,ifname) ;
	} else if (pip->debuglevel > 0) {
	    if (rs >= 0) {
	        LONG	som = (scanoff / (1024*1024)) ;
		LONG	slm = (tlen / (1024*1024)) ;
	        fmt = "%s: file=%s so=%lldm sl=%lldm\n" ;
	        shio_printf(pip->efp,fmt,pn,ifname,som,slm) ;
	    } else {
	        fmt = "%s: file=%s (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,ifname,rs) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    LONG	som = (scanoff / (1024*1024)) ;
	    LONG	slm = ((roff-scanoff) / (1024*1024)) ;
	    cchar	*pn = "main/procfile" ;
	    cchar	*fmt = "%s: file=%s so=%lldm sl=%lldm\n" ;
	    if (scanlen == 0) {
	        fmt = "%s: file=%s so=%lldm sl=Inf\n" ;
	    }
	    debugprintf(fmt,pn,ifname,som,slm) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procfile: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? nblocks : rs ;
}
/* end subroutine (procfile) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->pagesize = getpagesize() ;

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


static int locinfo_allocbegin(LOCINFO *lip,int blocklen)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if (blocklen <= 0) return SR_INVALID ;

	if (blocklen <= lip->pagesize) {
	    char	*p ;
	    if (blocklen < lip->pagesize) blocklen = lip->pagesize ;
	    rs = uc_valloc(blocklen,&p) ;
	    if (rs >= 0) lip->rdata = p ;
	} else {
	    size_t	ms = uceil(blocklen,lip->pagesize) ;
	    const int	mp = (PROT_READ|PROT_WRITE) ;
	    const int	mf = (MAP_PRIVATE|MAP_ANON) ;
	    void	*md ;
	    if ((rs = u_mmap(NULL,ms,mp,mf,-1,0L,&md)) >= 0) {
	        lip->rdata = md ;
	        lip->rsize = ms ;
	        lip->f.mapfile = TRUE ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_allocbegin) */


static int locinfo_allocend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->rdata != NULL) {
	    if (lip->rsize > 0) {
	        rs1 = u_munmap(lip->rdata,lip->rsize) ;
	        if (rs >= 0) rs = rs1 ;
	        lip->rsize = 0 ;
	    } else {
	        rs1 = uc_free(lip->rdata) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    lip->rdata = 0 ;
	}

	return rs ;
}
/* end subroutine (locinfo_allocend) */


static int locinfo_scanspec(LOCINFO *lip,cchar *sp,int sl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl > 0) {
	    offset_t	scanoff, scanlen ;
	    int		cl ;
	    cchar	*cp ;
	    cchar	*tp ;
	    if ((tp = strnchr(sp,sl,':')) != NULL) {
	        cp = (tp+1) ;
	        cl = ((sp+sl)-cp) ;
	        rs = cfdecmfll(cp,cl,&scanlen) ;
	        lip->scanlen = scanlen ;
	        sl = (tp-sp) ;
	    }
	    if (rs >= 0) {
	        rs = cfdecmfll(sp,sl,&scanoff) ;
	        lip->scanoff = scanoff ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_scanbad/locinfo_scanspec: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_scanspec) */


