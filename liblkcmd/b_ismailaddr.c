/* b_ismailaddr */

/* SHELL built-in to test for local mail-addresses */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2008-08-11, David A­D­ Morano
	This subroutine (program) was adapted from an old 1999 program (which
	bore little remote similarity) that was used on the old hardware CAD
	systems.  This program is also ready-designed for use as a KSH built-in
	command (if anyone might ever want that for any reason).

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is (or can be) a built-in KSH command for testing whether
	specified mail-addresses are local (local native to the current node)
	or not.  Take a look at this.  Would you have thought that determining
	a local mail-address was this complicated?

	Synopsis:

	$ ismailaddr <mailaddr(s)> [-af <afile>] [-V]


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
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<field.h>
#include	<kvsfile.h>
#include	<nulstr.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<pwi.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_ismailaddr.h"
#include	"defs.h"
#include	"proglog.h"
#include	"address.h"


/* local defines */

#ifndef	VARCLUSTER
#define	VARCLUSTER	"CLUSTER"
#endif

#ifndef	LOCALDOMAINNAME
#define	LOCALDOMAINNAME	"local"
#endif

#ifndef	LOCALHOSTNAME
#define	LOCALHOSTNAME	"localhost"
#endif

#define	CVTBUFLEN	100

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getclustername(cchar *,char *,int,cchar *) ;
extern int	getuid_name(cchar *,int) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		db:1 ;		/* NAMES DB is open */
	uint		rndb:1 ;	/* RN DB is open */
} ;

struct locinfo {
	PROGINFO	*pip ;
	cchar		*rndbfname ;
	VECSTR		names ;
	PWI		rndb ;
	LOCINFO_FL	init, have, f, final ;
	LOCINFO_FL	open ;
	uint		c_total ;
	uint		c_local ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procnames(PROGINFO *,void *,cchar *,int) ;
static int	procname(PROGINFO *,void *,cchar *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_pwi(LOCINFO *,cchar *) ;
static int	locinfo_loadnames(LOCINFO *,cchar *) ;
static int	locinfo_loadone(LOCINFO *,cchar *,int) ;
static int	locinfo_pwilookup(LOCINFO *,cchar *) ;
static int	locinfo_finish(LOCINFO *) ;

static int	initdomain(PROGINFO *) ;
static int	addrcompact(char *,int,cchar *,int) ;


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
	"lf",
	"ln",
	"lnf",
	"rndb",
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
	argopt_lf,
	argopt_ln,
	argopt_lnf,
	argopt_rndb,
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
	{ SR_INVALID, EX_USAGE },
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
	"logging",
	NULL
} ;

enum akonames {
	akoname_logging,
	akoname_overlast
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


/* exported subroutines */


int b_ismailaddr(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_ismailaddr) */


int p_ismailaddr(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_ismailaddr) */


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
	cchar		*lnfname = NULL ;
	cchar		*rndbfname = NULL ;
	cchar		*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("mainsub: starting DFD=%d\n",rs) ;
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
	pip->daytime = time(NULL) ;
	pip->f.logprog = TRUE ;

	pip->lip = lip ;
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

/* log file name */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->lfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* local-hosts file name */
	                case argopt_ln:
	                case argopt_lnf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lnfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lnfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* real-name DB */
	                case argopt_rndb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            rndbfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rndbfname = argp ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
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
	    debugprintf("mainsub: debuglevel=%u\n",pip->debuglevel) ;
#endif

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

/* process program options */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* find and load the "localnames" file DB */

	if ((rs >= 0) && ((lnfname == NULL) || (lnfname[0] == '\0'))) {
	    lnfname = tmpfname ;
	    rs = mkpath2(tmpfname,pip->pr,LNFNAME) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("mainsub: lnfname=%s\n",lnfname) ;
#endif

	if (rs >= 0) {
	    if ((rs = initdomain(pip)) >= 0) {
		if ((rs = locinfo_loadnames(lip,lnfname)) >= 0) {
		    rs = locinfo_pwi(lip,rndbfname) ;
		}
	    }
	}

	if ((pip->debuglevel > 0) && (lip->rndbfname == NULL)) {
	    shio_printf(pip->efp,"%s: PWI DB was not given\n",
	        pip->progname) ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: logging=%u\n",
	        pip->progname,pip->f.logprog) ;
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = proguserlist_begin(pip)) >= 0) {
			    {
				ARGINFO	*aip = &ainfo ;
				BITS	*bop = &pargs ;
	        		cchar	*afn = afname ;
	          		cchar	*ofn = ofname ;
	          		rs = procargs(pip,aip,bop,ofn,afn) ;
			    }
			    rs1 = proguserlist_end(pip) ;
			    if (rs >= 0) rs = rs1 ;
			} /* end if (proguserlist) */
			rs1 = proglog_end(pip) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (proglog) */
		    rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

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
	    if (lip->c_total != lip->c_local) {
	        ex = EX_NOTFOUND ;
	    } else if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	}

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
	    debugprintf("mainsub: final mallout=%u\n",(mo-mo_start)) ;
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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<mailaddr(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-lnf <lnfile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;
	int		opts ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->init.db = TRUE ;

	opts = VECSTR_OSORTED ;
	rs = vecstr_start(&lip->names,10,opts) ;
	lip->f.db = (rs >= 0) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.rndb) {
	    lip->f.rndb = FALSE ;
	    rs1 = pwi_close(&lip->rndb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->rndbfname != NULL) {
	    rs1 = uc_free(lip->rndbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->rndbfname = NULL ;
	}

	if (lip->f.db) {
	    lip->f.db = FALSE ;
	    rs1 = vecstr_finish(&lip->names) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_pwi(LOCINFO *lip,cchar *rndbfname)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (rndbfname != NULL) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(rndbfname,-1,&cp)) >= 0) {
	        lip->rndbfname = cp ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_pwi) */


static int locinfo_pwilookup(LOCINFO *lip,cchar *name)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("locinfo_pwilookup: ent name=%s\n",name) ;
	    debugprintf("locinfo_pwilookup: rndbfname=%s\n",lip->rndbfname) ;
	}
#endif

	if (! lip->init.rndb) {
	    lip->init.rndb = TRUE ;
	    rs1 = pwi_open(&lip->rndb,pip->pr,lip->rndbfname) ;
	    lip->f.rndb = (rs1 >= 0) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("locinfo_pwilookup: pwi_open() rs=%d\n",rs1) ;
#endif
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_pwilookup: mid rs=%d name=%s\n",rs,name) ;
#endif

	if (lip->f.rndb && (name[0] != '\0')) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    rs = pwi_lookup(&lip->rndb,ubuf,ulen,name) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_pwilookup: pwi_lookup() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_pwilookup) */


/* load the various names for this host (order doesn't matter; sorted later) */
static int locinfo_loadnames(LOCINFO *lip,cchar *lnfname)
{
	PROGINFO	*pip = lip->pip ;
	KVSFILE		kv ;
	KVSFILE_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		nbl ;
	int		nlen = MAXHOSTNAMELEN ;
	int		c = 0 ;		/* this count is not really correct */
	cchar		*lhn = LOCALHOSTNAME ;
	cchar		*nn ;
	cchar		*cp ;
	char		nbuf[MAXHOSTNAMELEN + 1] ;

	if (lnfname == NULL) return SR_FAULT ;

	if (lnfname[0] == '\0') return SR_INVALID ;

	nn = pip->nodename ;

/* miscellaneous names (like 'localhost') */

	if (rs >= 0) {
	    rs = locinfo_loadone(lip,lhn,-1) ;
	    c += rs ;
	}

/* load our nodename */

	if (rs >= 0) {
	    rs = locinfo_loadone(lip,nn,-1) ;
	    c += rs ;
	} /* end if (local node) */

/* clustername */

	if (rs >= 0) {

	    cl = -1 ;
	    cp = getourenv(pip->envv,VARCLUSTER) ;

	    if ((cp == NULL) || (cp[0] == '\0')) {

	        rs1 = getclustername(pip->pr,nbuf,NODENAMELEN,nn) ;
	        nbl = rs1 ;
	        if (rs1 >= 0) {
	            cp = nbuf ;
	            cl = nbl ;
	        }

	    } /* end if */

	    if (cp != NULL) {
	        rs = locinfo_loadone(lip,cp,cl) ;
	        c += rs ;
	    }

	} /* end if (cluster) */

/* load the base-machine domain-name */

	if (rs >= 0) {
	    c += 1 ;
	    rs = vecstr_adduniq(&lip->names,pip->domainname,-1) ;
	}

/* continue with our local names */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_loadnames: LOCALNAMES\n") ;
#endif

	if ((rs >= 0) && (kvsfile_open(&kv,10,lnfname) >= 0)) {

	    if ((rs = kvsfile_curbegin(&kv,&cur)) >= 0) {

	        while (rs >= 0) {
	            nbl = kvsfile_fetch(&kv,nn,&cur,nbuf,nlen) ;
	            if (nbl == SR_NOTFOUND) break ;
	            rs = nbl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("locinfo_loadnames: LOCALNAMES n=%t\n",
	                    nbuf,nbl) ;
#endif

	            if (nbl > 0) {

	                if (strnchr(nbuf,nbl,'.') != NULL) {
	                    rs = vecstr_adduniq(&lip->names,nbuf,nbl) ;
	                    c += 1 ;
	                } else {
	                    rs = locinfo_loadone(lip,nbuf,nbl) ;
	                    c += rs ;
	                } /* end if */

	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while */

	        kvsfile_curend(&kv,&cur) ;
	    } /* end if (cursor) */

	    kvsfile_close(&kv) ;
	} /* end if (local-name DB) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	i ;
	    for (i = 0 ; vecstr_get(&lip->names,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;
	        debugprintf("locinfo_loadnames: n=%s\n",cp) ;
	    }
	}
#endif /* CF_DEBUG */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadnames) */


static int locinfo_loadone(LOCINFO *lip,cchar *namep,int namel)
{
	PROGINFO	*pip = lip->pip ;
	NULSTR		s ;
	int		rs ;
	int		c = 0 ;
	cchar		*np ;

	if ((rs = nulstr_start(&s,namep,namel,&np)) >= 0) {
	    const int	nl = rs ;
	    const int	namelen = MAXHOSTNAMELEN ;
	    int		nbl ;
	    cchar	*ldn = LOCALDOMAINNAME ;
	    char	namebuf[MAXHOSTNAMELEN + 1] ;

	    if (rs >= 0) {
	        c += 1 ;
	        rs = vecstr_adduniq(&lip->names,np,nl) ;
	    }

	    if (rs >= 0) {
	        nbl = snsds(namebuf,namelen,np,ldn) ;
	        if (nbl > 0) {
	            c += 1 ;
	            rs = vecstr_adduniq(&lip->names,namebuf,nbl) ;
	        }
	    } /* end if ('name.local') */

	    if (rs >= 0) {
	        nbl = snsds(namebuf,namelen,np,pip->domainname) ;
	        if (nbl > 0) {
	            c += 1 ;
	            rs = vecstr_adduniq(&lip->names,namebuf,nbl) ;
	        }
	    } /* end if ('name.domain') */

	    nulstr_finish(&s) ;
	} /* end if (nulstr) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadone) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
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

	                case akoname_logging:
	                    if (! pip->final.logprog) {
	                        pip->have.logprog = TRUE ;
	                        pip->final.logprog = TRUE ;
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
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


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procuserinfo_logid: rm=%08ß\n",rs) ;
#endif
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            const int	s = rs ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
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
	    int		cl ;
	    cchar	*cp ;

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

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procnames(pip,ofp,cp,cl) ;
	                    }
	                } /* end if (sfskipwhite) */

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
	        fmt = "%s: no mail-addresses given\n" ;
	        shio_printf(pip->efp,fmt,pn) ;
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


static int procnames(PROGINFO *pip,void *ofp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            c += 1 ;
	            rs = procname(pip,ofp,fp,fl) ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


/* process a mail-address */
static int procname(PROGINFO *pip,void *ofp,cchar *namep,int namel)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		type = 0 ;
	int		ans ;
	int		wlen = 0 ;
	int		f = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		mailaddr[MAILADDRLEN + 1] ;
	char		partlocal[MAILADDRLEN + 1] ;
	char		parthost[MAILADDRLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ismailaddr/procname: ent namep=>%t<\n",
	        namep,strnlen(namep,MAX(namel,60))) ;
#endif

	lip->c_total += 1 ;

/* compact the mail-address if needed */

	if ((rs = addrcompact(mailaddr,MAILADDRLEN,namep,namel)) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("ismailaddr/procname: addrcompact() rs=%d\n",rs) ;
	        debugprintf("ismailaddr/procname: m=>%t<\n",mailaddr,rs) ;
	    }
#endif
	    namel = rs ;
	    namep = mailaddr ;
	}

/* parse the mail-address into local and host parts */

	if (rs >= 0) {
	    rs = addressparse(namep,namel,parthost,partlocal) ;
	    type = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("ismailaddr/procname: type=%u\n",type) ;
	    debugprintf("ismailaddr/procname: parthost=%s\n",parthost) ;
	    debugprintf("ismailaddr/procname: partlocal=%s\n",partlocal) ;
	}
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: a=%t type=%u\n",
	        pip->progname,namep,namel,type) ;
	}

/* first check that the host-domain is ours (cheap action) */

	if (rs >= 0) {
	    if (parthost[0] != '\0') {
	        cchar	*localhost = ADDRESS_LOCALHOST ;
	        if (strcmp(parthost,localhost) != 0) {
		    vecstr	*nlp = &lip->names ;
		    const int	nrs = SR_NOTFOUND ;
	            if ((rs = vecstr_search(nlp,parthost,NULL,NULL)) >= 0) {
			f = TRUE ;
		    } else if (rs == nrs) {
			rs = SR_OK ;
		    }
	        }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ismailaddr/procname: ourhost rs=%d\n",rs) ;
#endif

/* now check that the local-part (user) is ours (more expensive action) */

	if ((rs >= 0) && (! f) && (partlocal[0] != 'U')) {
	    if ((partlocal[0] != '\0') && (strchr(partlocal,'.') == NULL)) {
	        if ((rs = getuid_name(partlocal,-1)) >= 0) {
	            f = TRUE ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    }
	    if ((rs >= 0) && (! f)) {
	        if ((rs = locinfo_pwilookup(lip,partlocal)) >= 0) {
	            f = TRUE ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    }
	} /* end if (testing local-part) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ismailaddr/procname: ouruser rs=%d f=%u\n",rs,f) ;
#endif

/* record (account for) the result */

	if ((rs >= 0) && f) {
	    lip->c_local += 1 ;
	}

	ans = (f) ? 'Y' : 'N' ;

/* debugging */

	if (pip->debuglevel > 0) {
	    fmt = "%s: %c %t\n" ;
	    shio_printf(pip->efp,fmt,pn,ans,namep,strnlen(namep,namel)) ;
	} /* end if (debugging) */

/* logging */

	if (pip->open.logprog) {
	    fmt = "%c %t\n" ;
	    logfile_printf(&pip->lh,fmt,ans,namep,strnlen(namep,namel)) ;
	} /* end if (logging) */

/* optionally print result */

	if (pip->verboselevel >= 2) {
	    fmt = "%c %t\n" ;
	    rs = shio_printf(ofp,fmt,ans,namep,strnlen(namep,namel)) ;
	    wlen += rs ;
	} /* end if (verbose) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


static int initdomain(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = TRUE ;

	f = f && (pip->nodename != NULL) ;
	f = f && (pip->domainname != NULL) ;
	f = f && (pip->nodename[0] != '\0') ;
	f = f && (pip->domainname[0] != '\0') ;
	if (! f) {
	    char	nodename[NODENAMELEN + 1] ;
	    char	domainname[MAXHOSTNAMELEN + 1] ;

	    if ((rs = getnodedomain(nodename,domainname)) >= 0) {
	        if ((rs >= 0) && (nodename[0] != '\0')) {
		    cchar	**vpp = &pip->nodename ;
	            proginfo_setentry(pip,vpp,nodename,-1) ;
		}
	        if ((rs >= 0) && (domainname[0] != '\0')) {
		    cchar	**vpp = &pip->domainname ;
	            proginfo_setentry(pip,vpp,domainname,-1) ;
		}
	    } /* end if (getnodedoamin) */

	} /* end if (needed) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (initdomain) */


/* optionally compact (remove white-space) a mail-address as needed */
static int addrcompact(char *rbuf,int rlen,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	cchar		*tp ;

	if (rbuf == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("addrcompact: ent n=>%t<\n",np,nl) ;
#endif

	rbuf[0] = '\0' ;
	if (nl < 0) nl = strlen(np) ;

	while ((tp = strnpbrk(np,nl," \t")) != NULL) {
	    if ((tp-np) > 0) {
	        rs = storebuf_strw(rbuf,rlen,i,np,(tp-np)) ;
	        i += rs ;
	    }
	    nl -= ((tp+1) - np) ;
	    np = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (nl > 0)) {
	    rs = storebuf_strw(rbuf,rlen,i,np,nl) ;
	    i += rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("addrcompact: near rs=%d i=%u\n",rs,i) ;
	debugprintf("addrcompact: r=>%t<\n",rbuf,i) ;
#endif

/* remove the stupid trailing dots! */

	if ((rs >= 0) && (i > 0)) {
	    while ((i > 0) && (rbuf[i - 1] == '.')) i -= 1 ;
	    rbuf[i] = '\0' ;
	} /* end if (dot removal) */

#if	CF_DEBUGS
	debugprintf("addrcompact: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (addrcompact) */


