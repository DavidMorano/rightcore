/* b_loghist */

/* SHELL built-in similar to 'who(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FULLNAME	0		/* use fullname? */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet.

	This built-in is pretty straight forward.  We used the supplied UNIX®
	System subroutines for accessing UTMPX because it is more portable and
	speed is not paramount, like it would be if we were continuously
	scanning the UTMPX DB for changed events.  In short, the system UTMPX
	subroutines are fast enough for a one-shot scan through the UTMPX DB
	like we are doing here.

	One interesting thing to note is that we do maintain a cache of the
	username-to-realname translations.  Even though we are only scanning
	the UTMPX DB once, there may be several repeat logins by the same
	username.  Using the cache, we bypass asking the system for every
	username for those usernames that we've seen before.  I think that the
	cost of maintaining the cache is not as bad as asking the system for
	every username.  Your mileage may vary!

	Synopsis:

	$ loghost [-l]


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
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<field.h>
#include	<vecobj.h>
#include	<tmpx.h>
#include	<realname.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_loghist.h"
#include	"defs.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	100		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		dotusers:1 ;
	uint		hdr:1 ;
	uint		self:1 ;
} ;

struct locinfo {
	const char	*wtmpfname ;
	LOCINFO_FL	have, f, changed, final ;
	int		records ;
	char		username[USERNAMELEN + 1] ;
} ;

struct user {
	time_t		date ;
	uint		count ;
	char		name[TMPX_LUSER + 1] ;
} ;

struct dead {
	uint		ei ;
} ;


/* forward references */

static int	mainsub(int,const char **,const char **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,vecstr *,cchar *) ;
static int	procloadnames(PROGINFO *,VECSTR *,const char *,int) ;
static int	procloadname(PROGINFO *,vecstr *,const char *,int) ;
static int	procout(PROGINFO *,const char *,vecstr *) ;
static int	process(PROGINFO *,SHIO *,vecstr *) ;
static int	procthemout(PROGINFO *,void *,vecobj *) ;
static int	getrealname(char *,int,const char *) ;

static int	cmpname(struct user **,struct user **) ;
static int	cmpdate(struct user **,struct user **) ;
static int	cmpei(struct dead **,struct dead **) ;
static int	utmatch(TMPX_ENT *,TMPX_ENT *) ;

#if	CF_DEBUGS || CF_DEBUG
static int debugprintrecord(const char *, TMPX_ENT *) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"sn",
	"af",
	"ef",
	"of",
	"utf",
	"db",
	"nh",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_utf,
	argopt_db,
	argopt_nh,
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
	"dotusers",
	"hdr",
	"header",
	NULL
} ;

enum akonames {
	akoname_dotusers,
	akoname_hdr,
	akoname_header,
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


int b_loghist(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_loghist) */


int p_loghist(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_loghist) */


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

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_loghist: starting DFD=%d\n",rs) ;
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

	pip->lip = &li ;
	memset(&li,0,sizeof(LOCINFO)) ;

	pip->verboselevel = 1 ;

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

	                case argopt_nh:
	                    lip->f.hdr = FALSE ;
	                    lip->final.hdr = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.hdr = (rs == 0) ;
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

/* output name */
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

/* WTMP filename */
	                case argopt_utf:
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->wtmpfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->wtmpfname = argp ;
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

/* print header */
	                    case 'H':
	                        lip->f.hdr = TRUE ;
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

/* WTMP file */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->wtmpfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* print header */
	                    case 'h':
	                        lip->final.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.hdr = (rs > 0) ;
	                            }
	                        }
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

/* show records? */
	                    case 'r':
	                        lip->records = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->records = rs ;
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_loghist: debuglevel=%u\n",pip->debuglevel) ;
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
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (lip->wtmpfname == NULL) lip->wtmpfname = WTMPFNAME ;

	if ((cp = getourenv(envv,VARUSERNAME)) != NULL) {
	    strwcpy(lip->username,cp,USERNAMELEN) ;
	}

/* OK, do the thing */

	        memset(&ainfo,0,sizeof(ARGINFO)) ;
	        ainfo.argc = argc ;
	        ainfo.ai = ai ;
	        ainfo.argv = argv ;
	        ainfo.ai_max = ai_max ;
	        ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    vecstr	names ;
	    if ((rs = vecstr_start(&names,10,0)) >= 0) {
	        const char	*ofn = ofname ;
	        const char	*afn = afname ;
	        if ((rs = procargs(pip,&ainfo,&pargs,&names,afn)) >= 0) {
	            rs = procout(pip,ofn,&names) ;
	        }
	        rs1 = vecstr_finish(&names) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (names) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
	        shio_printf(pip->efp,
	            "%s: could not perform function (%d)\n",
	            pip->progname,rs) ;
	    }
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
	    debugprintf("b_loghist: exiting ex=%u (%d)\n",ex,rs) ;
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
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_loghist: final mallout=%u\n",(mo-mo_start)) ;
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
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-h] [<name(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-db <wtmpx>] [-r]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_dotusers:
	                    lip->have.dotusers = TRUE ;
	                    lip->f.dotusers = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.dotusers = (rs > 0) ;
	                    }
	                    break ;

	                case akoname_header:
	                case akoname_hdr:
	                    if (! lip->final.hdr) {
	                        lip->final.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.hdr = (rs > 0) ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,vecstr *nlp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
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
	                rs = procloadname(pip,nlp,cp,-1) ;
	                c += rs ;
	            }
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
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = procloadnames(pip,nlp,cp,cl) ;
	                    c += rs ;
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

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: specified=%u\n",pn,c) ;
	}

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procloadnames(PROGINFO *pip,VECSTR *nlp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procloadname(pip,nlp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadnames) */


static int procloadname(PROGINFO *pip,vecstr *nlp,cchar *cp,int cl)
{
	LOCINFO		*lip = pip->lip ;
	const int	nch = MKCHAR(cp[0]) ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if ((nch == '-') || (nch == '+')) {
	    cp = lip->username ;
	    cl = -1 ;
	    if (lip->username[0] == '\0') {
	        rs = getusername(lip->username,USERNAMELEN,-1) ;
	        cl = rs ;
	    }
	    if (nch == '+') {
	        lip->f.self = TRUE ;
	    }
	}

	if (rs >= 0) {
	    rs = vecstr_adduniq(nlp,cp,cl) ;
	    if (rs < INT_MAX) c = 1 ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadname) */


static int procout(PROGINFO *pip,cchar *ofn,vecstr *nlp)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    rs = process(pip,ofp,nlp) ;
	    wlen += rs ;
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n",
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    fmt = "%s: offile=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end if (procout) */


/* process the names that we have */
static int process(PROGINFO *pip,SHIO *ofp,vecstr *aup)
{
	struct user	u, *rp ;
	LOCINFO		*lip = pip->lip ;
	TMPX		wt ;
	TMPX_CUR	cur, fcur ;
	TMPX_ENT	e, fe, *up = &e ;
	vecobj		users, deads ;
	int		rs, rs1 ;
	int		ei ;
	int		size ;
	int		wlen = 0 ;
	int		f_argusers = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_loghist/process: ent\n") ;
#endif

#ifdef	COMMENT
	vecstr_sort(aup,NULL) ;
#endif

	if ((rs = vecstr_count(aup)) > 0) {
	    f_argusers = TRUE ;
	} else if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_loghist/process: n=%u\n",rs) ;
#endif

	size = sizeof(struct user) ;
	rs = vecobj_start(&users,size,20,0) ;
	if (rs < 0)
	    goto ret0 ;

	size = sizeof(struct dead) ;
	rs = vecobj_start(&deads,size,20,VECOBJ_PSWAP) ;
	if (rs < 0)
	    goto ret1 ;

/* loop through session records looking for user-type records */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_loghist/process: TMPX\n") ;
#endif

	if ((rs = tmpx_open(&wt,lip->wtmpfname,O_RDONLY)) >= 0) {
	    if ((rs = tmpx_curbegin(&wt,&cur)) >= 0) {
	        while ((ei = tmpx_enum(&wt,&cur,&e)) >= 0) {

	            if (f_argusers) {
	                char	userbuf[LOGNAMELEN + 1] ;

	                strwcpy(userbuf,e.ut_user,
	                    MIN(LOGNAMELEN,TMPX_LUSER)) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_loghist/process: userbuf=%s\n",
	                        userbuf) ;
#endif

	                rs1 = vecstr_search(aup,userbuf,NULL,NULL) ;
	                if (rs1 == SR_NOTFOUND) continue ;

	            } /* end if (argusers) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_loghist/process: records=%u\n",
	                    lip->records) ;
#endif

	            if (lip->records) {
	                rs = shio_printf(ofp,
	                    "t=%u i=%-4t u=%-12t l=%-12t p=%6u e=%2d %s\n",
	                    up->ut_type,
	                    up->ut_id,strlinelen(up->ut_id,TMPX_LID,4),
	                    up->ut_user,strlinelen(up->ut_user,TMPX_LUSER,12),
	                    up->ut_line,strlinelen(up->ut_line,TMPX_LLINE,12),
	                    up->ut_pid,
	                    up->ut_exit.e_exit,
	                    timestr_log(up->ut_tv.tv_sec,timebuf)) ;

	                wlen += rs ;
	            }

	            if (((up->ut_type == TMPX_TUSERPROC) ||
	                (up->ut_type == TMPX_TDEADPROC)) &&
	                (up->ut_user[0] != '\0')) {

	                struct dead	d ;
	                time_t		date = up->ut_tv.tv_sec ;
	                int		f_skip = FALSE ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_loghist/process: "
	                        "searching for dead\n") ;
#endif

/* as necessary, search forward for a possible matching "dead" record */

	                if (up->ut_type == TMPX_TUSERPROC) {
	                    if ((rs = tmpx_curbegin(&wt,&fcur)) >= 0) {

	                        fcur = cur ;	/* from previous position */
	                        while (rs >= 0) {
	                            const char	*un = up->ut_user ;
	                            ei = tmpx_fetchuser(&wt,&fcur,&fe,un) ;
	                            if (ei == SR_NOTFOUND) break ;

	                            if ((fe.ut_type == TMPX_TDEADPROC) &&
	                                utmatch(&fe,up)) {

	                                date = fe.ut_tv.tv_sec ;
	                                d.ei = ei ;
	                                rs = vecobj_add(&deads,&d) ;
	                                break ;
	                            }

	                        } /* end while */

	                        tmpx_curend(&wt,&fcur) ;
	                    } /* end if (tmpx-cur) */
	                } /* end if (searching for matching dead record) */

/* skip any dead records that we've already seen a login for */

	                if ((rs >= 0) && (up->ut_type == TMPX_TDEADPROC)) {

	                    d.ei = ei ;
	                    rs1 = vecobj_search(&deads,&d,cmpei,NULL) ;

	                    if (rs1 >= 0) {

	                        f_skip = TRUE ;
	                        vecobj_del(&deads,rs1) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintrecord("b_loghist: live",up) ;
#endif

	                    } /* end if */

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4) && (rs1 == SR_NOTFOUND))
	                        debugprintrecord("b_loghist: dead",up) ;
#endif

	                } /* end if (dead record) */

/* continue with those records that we want to consider */

	                if ((rs >= 0) && (! f_skip)) {

	                    strncpy(u.name,up->ut_user,TMPX_LUSER) ;

	                    rs1 = vecobj_search(&users,&u,cmpname,&rp) ;

	                    if (rs1 == SR_NOTFOUND) {

	                        u.count = 1 ;
	                        u.date = date ;
	                        rs1 = vecobj_add(&users,&u) ;

	                    } else if (rs1 >= 0) {

	                        rp->count += 1 ;
	                        if (date > rp->date)
	                            rp->date = date ;

	                    } /* end if */

	                } /* end if (not skipped) */

	            } /* end if (got one) */

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (looping through records) */
	        tmpx_curend(&wt,&cur) ;
	    } /* end if (cursor) */
	    tmpx_close(&wt) ;
	} /* end if (tmpx) */

/* print out the report */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs = procthemout(pip,ofp,&users) ;
	    wlen += rs ;
	} /* end if (verbose output) */

	vecobj_finish(&deads) ;

ret1:
	vecobj_finish(&users) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_loghist/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procthemout(PROGINFO *pip,void *ofp,vecobj *ulp)
{
	struct passwd	pw ;
	LOCINFO		*lip = pip->lip ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*fmt ;
	char		*pwbuf ;

	vecobj_sort(ulp,cmpdate) ;

	    if (lip->f.hdr) {
	        fmt = "LOGNAME    NUMBER DATE                    REALNAME\n" ;
	        shio_printf(ofp,fmt) ;
	    }

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    struct user	*rp ;
	    const int	nrs = SR_NOTFOUND ;
	    const int	rlen = REALNAMELEN ;
	    int		i ;
	    char	rbuf[REALNAMELEN+1] ;
	    for (i = 0 ; vecobj_get(ulp,i,&rp) >= 0 ; i += 1) {
	        if (rp == NULL) continue ;

	        if ((! lip->f.dotusers) && (rp->name[0] == '.'))
	            continue ;		/* ignore fake (dot) users */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_loghist: name=%s\n",rp->name) ;
#endif

	        if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rp->name)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(6))
	                debugprintf("b_loghist: gecos=>%s<\n",pw.pw_gecos) ;
#endif

	            if ((rs = getrealname(rbuf,rlen,pw.pw_gecos)) == nrs) {
	                rs = sncpy1(rbuf,rlen,"* not found *") ;
		    }

	        } else if (isNotPresent(rs)) {
	            rs = sncpy1(rbuf,rlen,"* system record *") ;
	        }

	        if (rs >= 0) {
		    const int	nl = strnlen(rp->name,8) ;
		    cchar	*np = rp->name ;
	            char	tbuf[TIMEBUFLEN+1] ;
		    fmt = "%-8t %8u %-23s %s\n" ;
	            timestr_log(rp->date,tbuf),
	            rs = shio_printf(ofp,fmt,np,nl,rp->count,tbuf,rbuf) ;
	            wlen += rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_loghist: shio_printf() rs=%d\n",rs) ;
#endif


	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end for (looping through users) */
	    rs1 = uc_free(pwbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procthemout) */


static int getrealname(char *nbuf,int nlen,cchar *gecosname)
{
	const int	glen = GNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;
	char		gbuf[GNAMELEN + 1] ;

	if (gecosname == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;

	if (nlen < 0) nlen = REALNAMELEN ;

	if ((rs = mkgecosname(gbuf,glen,gecosname)) >= 0) {
	    REALNAME	rn ;
	    int		gl = rs ;
	    if ((rs = realname_start(&rn,gbuf,gl)) >= 0) {
	        nbuf[0] = '\0' ;

#if	CF_FULLNAME
	        rl = realname_fullname(&rm,nbuf,nlen) ;
#else
	        rl = realname_name(&rn,nbuf,nlen) ;
#endif

	        if (isNotPresent(rl)) {
	            rs = snwcpy(nbuf,nlen,gbuf,gl) ;
	            rl = rs ;
	        }

	        rs1 = realname_finish(&rn) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (realname processing) */
	} /* end if */

	return (rs >= 0) ? rl : 0 ;
}
/* end subroutine (getrealname) */


/* compare user names */
static int cmpname(struct user **e1pp,struct user **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strncmp((*e1pp)->name,(*e2pp)->name,TMPX_LUSER) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpname) */


/* compare record dates */
static int cmpdate(struct user **e1pp,struct user **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
		if (*e2pp != NULL) {
		    rc = ((*e1pp)->date - (*e2pp)->date) ;
		} else
		    rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpdate) */


/* compare dead record entry indices */
static int cmpei(struct dead **e1pp,struct dead **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = ((*e1pp)->ei - (*e2pp)->ei) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpei) */


/* compare two records */
static int utmatch(TMPX_ENT *u1p,TMPX_ENT *u2p)
{
	int		f = TRUE ;
	f = f && (u1p->ut_pid == u2p->ut_pid) ;
	f = f && (strncmp(u1p->ut_user,u2p->ut_user,TMPX_LUSER) == 0) ;
	f = f && (strncmp(u1p->ut_id,u2p->ut_id,TMPX_LID) == 0) ;
	f = f && (strncmp(u1p->ut_line,u2p->ut_line,TMPX_LLINE) == 0) ;
	return f ;
}
/* end subroutine (utmatch) */


#if	CF_DEBUGS || CF_DEBUG
static int debugprintrecord(cchar *s,TMPX_ENT *up)
{
	int		rs ;
	rs = debugprintf("%s t=%u id=%-4t u=%-12t l=%-12t p=6u e=%2d\n",
	    s,
	    up->ut_type,
	    up->ut_id,strnlen(up->ut_id,TMPX_LID),
	    up->ut_user,strnlen(up->ut_user,TMPX_LUSER),
	    up->ut_line,strnlen(up->ut_line,TMPX_LLINE),
	    up->ut_pid,
	    up->ut_exit.e_exit) ;
	return rs ;
}
#endif /* CF_DEBUG */


