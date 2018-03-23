/* b_username */

/* SHELL built-in: return various user information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_LOCSETENT	1		/* |locinfo_setentry()| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ username [<username>|-] 


*******************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

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
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<osetstr.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<field.h>
#include	<pcsns.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<sysusernames.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_username.h"
#include	"defs.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARUSER
#define	VARUSER		"USER"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_GMCUR	struct locinfo_gmcur
#define	LOCINFO_RNCUR	struct locinfo_rncur


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getgecosname(cchar *,int,cchar **) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	mkrealname(char *,int,cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
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
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_gmcur {
	GRMEMS_CUR	gmcur ;
} ;

struct locinfo_rncur {
	SYSREALNAME_CUR	rncur ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		nouser:1 ;
	uint		linebuf:1 ;
	uint		all:1 ;
	uint		realname:1 ;
	uint		name:1 ;		/* mode */
	uint		fullname:1 ;		/* mode */
	uint		org:1 ;			/* mode */
	uint		projinfo:1 ;		/* mode */
	uint		gm:1 ;
	uint		rn:1 ;
	uint		sysuser:1 ;
	uint		reguser:1 ;
	uint		speuser:1 ;
	uint		ns:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	vecstr		stores ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	PCSNS		ns ;
	cchar		*pr_pcs ;
	char		unbuf[USERNAMELEN + 1] ;
	char		gnbuf[GROUPNAMELEN+ 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_username(LOCINFO *) ;
static int	locinfo_groupname(LOCINFO *) ;
static int	locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,cchar *,int) ;
static int	locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;
static int	locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
static int	locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
static int	locinfo_prpcs(LOCINFO *) ;
static int	locinfo_pcsns(LOCINFO *) ;
static int	locinfo_pcsnsget(LOCINFO *,char *,int,cchar *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procall(PROGINFO *,ARGINFO *,BITS *,void *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,void *,cchar *) ;
static int	procloadnames(PROGINFO *,OSETSTR *,cchar *,int) ;
static int	procloadname(PROGINFO *,OSETSTR *,cchar *,int) ;
static int	procquery(PROGINFO *,SHIO *,cchar *) ;
static int	procselect(PROGINFO *,struct passwd *) ;
static int	procout(PROGINFO *,SHIO *,struct passwd *,cchar *) ;
static int	procgetns(PROGINFO *,char *,int,cchar *,int) ;
static int	getname(PROGINFO *,struct passwd *,char *,int,cchar *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
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

static const char	*progmodes[] = {
	"username",
	"userhome",
	"userdir",
	"logdir",
	NULL
} ;

enum progmodes {
	progmode_username,
	progmode_userhome,
	progmode_userdir,
	progmode_logdir,
	progmode_overlast
} ;

static const char	*akonames[] = {
	"linebuf",
	"all",
	"realname",
	"pcsname",
	"name",
	"fullname",
	"org",
	"projinfo",
	"sysuser",
	"reguser",
	"speuser",
	NULL
} ;

enum akonames {
	akoname_linebuf,
	akoname_all,
	akoname_realname,
	akoname_pcsname,
	akoname_name,
	akoname_fullname,
	akoname_org,
	akoname_projinfo,
	akoname_sysuser,
	akoname_reguser,
	akoname_speuser,
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

static const char	*specials[] = {
	"noaccess",
	"nobody",
	"nobody4",
	NULL
} ;


/* exported subroutines */


int p_username(int argc,cchar *argv[],cchar *envv[],void *contextp)
{

	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_username) */


int p_userhome(int argc,cchar *argv[],cchar *envv[],void *contextp)
{

	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_userhome) */


int p_userdir(int argc,cchar *argv[],cchar *envv[],void *contextp)
{

	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_userdir) */


int p_logdir(int argc,cchar *argv[],cchar *envv[],void *contextp)
{

	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_logdir) */


int b_username(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_username) */


int b_userhome(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_userhome) */


int b_userdir(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_userdir) */


int b_logdir(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_logdir) */


/* local subroutines */


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
	cchar		*pm = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_userxxx: starting DFD=%d\n",rs) ;
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

	pip->verboselevel = 1 ;

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
	                        pip->debuglevel = 2 ;
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

	                    case 'a':
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'n':
	                        lip->f.realname = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.realname = (rs > 0) ;
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

#if	CF_DEBUGS
	pip->debuglevel = 5 ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_userxxx: pn=%s\n",pip->progname) ;
	    debugprintf("b_userxxx: debuglevel=%u\n",pip->debuglevel) ;
	}
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(pip->efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(pip->efp,fmt,pn,pm) ;
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
	}

/* set program-root */

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
	    debugprintf("b_userxxx: pr=%s\n",pip->pr) ;
	    debugprintf("b_userxxx: sn=%s\n",pip->searchname) ;
	}
#endif

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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some preliminary initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* OK, we finally do our thing */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    rs = process(pip,&ainfo,&pargs,ofn,afn) ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_userxxx: process() rs=%d nouser=%u\n",
	        rs,lip->f.nouser) ;
#endif

	if ((rs >= 0) && lip->f.nouser) {
	    rs = SR_SRCH ;
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
	    case SR_SRCH:
	        ex = EX_NOUSER ;
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
	    debugprintf("b_userxxx: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("b_userxxx: final mallout=%u\n",(mo-mo_start)) ;
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
	cchar		*pn = progmodes[pip->progmode] ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<username(s)>|-] [-q]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
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
	                case akoname_linebuf:
	                    if (! lip->final.linebuf) {
	                        lip->have.linebuf = TRUE ;
	                        lip->final.linebuf = TRUE ;
	                        lip->f.linebuf = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.linebuf = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_all:
	                    if (! lip->final.all) {
	                        lip->have.all = TRUE ;
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.all = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_realname:
	                    if (! lip->final.realname) {
	                        lip->have.realname = TRUE ;
	                        lip->final.realname = TRUE ;
	                        lip->f.realname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.realname = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_pcsname:
	                case akoname_name:
	                    if (! lip->final.name) {
	                        lip->have.name = TRUE ;
	                        lip->final.name = TRUE ;
	                        lip->f.name = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.name = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_fullname:
	                    if (! lip->final.fullname) {
	                        lip->have.fullname = TRUE ;
	                        lip->final.fullname = TRUE ;
	                        lip->f.fullname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fullname = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_org:
	                    if (! lip->final.org) {
	                        lip->have.org = TRUE ;
	                        lip->final.org = TRUE ;
	                        lip->f.org = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.org = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_projinfo:
	                    if (! lip->final.projinfo) {
	                        lip->have.projinfo = TRUE ;
	                        lip->final.projinfo = TRUE ;
	                        lip->f.projinfo = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.projinfo = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_sysuser:
	                    if (! lip->final.sysuser) {
	                        lip->have.sysuser = TRUE ;
	                        lip->final.sysuser = TRUE ;
	                        lip->f.sysuser = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.sysuser = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_reguser:
	                    if (! lip->final.reguser) {
	                        lip->have.reguser = TRUE ;
	                        lip->final.reguser = TRUE ;
	                        lip->f.reguser = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.reguser = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_speuser:
	                    if (! lip->final.speuser) {
	                        lip->have.speuser = TRUE ;
	                        lip->final.speuser = TRUE ;
	                        lip->f.speuser = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.speuser = (rs > 0) ;
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {

	    if (lip->f.linebuf)
	        rs = shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;

	    if (rs >= 0) {
	        if (lip->f.all) {
	            rs = procall(pip,aip,bop,ofp) ;
	        } else {
	            rs = procargs(pip,aip,bop,ofp,afn) ;
	        }
	    } /* end if (ok) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp,cchar *afn)
{
	OSETSTR		ss ;
	const int	n = 20 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = osetstr_start(&ss,n)) >= 0) {
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
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procloadname(pip,&ss,cp,-1) ;
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

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procloadnames(pip,&ss,cp,cl) ;
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

	    if ((rs >= 0) && (pan == 0)) {
	        cp = "-" ;
	        pan += 1 ;
	        rs = procloadname(pip,&ss,cp,-1) ;
	    } /* end if (default) */

	    if (rs >= 0) {
	        OSETSTR_CUR	cur ;
	        if ((rs = osetstr_curbegin(&ss,&cur)) >= 0) {
	            while ((rs1 = osetstr_enum(&ss,&cur,&cp)) >= 0) {
	                if (strcmp(cp,"--") != 0) {
	                    rs = procquery(pip,ofp,cp) ;
	                    wlen += rs ;
	                }
	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	            rs1 = osetstr_curend(&ss,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (osetstr-cursor) */
	    } /* end if (ok) */

	    rs1 = osetstr_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (osetstr) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procloadnames(PROGINFO *pip,OSETSTR *nlp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;

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


static int procloadname(PROGINFO *pip,OSETSTR *nlp,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ent rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || hasMeAlone(np,nl)) {
	    if ((rs = locinfo_username(lip)) >= 0) {
	        np = lip->unbuf ;
	        nl = rs ;
	        rs = osetstr_add(nlp,np,nl) ;
	        c += rs ;
	    } /* end if (locinfo_username) */
	} else {
	    const int	nch = MKCHAR(np[0]) ;
	    cchar	*tp ;
	    if ((tp = strnchr(np,nl,'+')) != NULL) {
	        nl = (tp-np) ;
	    }
	    if (strnchr(np,nl,'.') != NULL) {
	        LOCINFO_RNCUR	rnc ;
	        if ((rs = locinfo_rncurbegin(lip,&rnc)) >= 0) {
	            if ((rs = locinfo_rnlook(lip,&rnc,np,nl)) > 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_rnread(lip,&rnc,ub,ul)) > 0) {
	                    rs = osetstr_add(nlp,ub,rs) ;
	                    c += rs ;
	                    if (rs < 0) break ;
	                } /* end while (reading entries) */
	            } /* end if (locinfo_rnlook) */
	            rs1 = locinfo_rncurend(lip,&rnc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (srncursor) */
	    } else if (nch == MKCHAR('¡')) {
	        LOCINFO_GMCUR	gc ;
	        cchar		*gnp = (np+1) ;
	        int		gnl = (nl-1) ;
		if (gnl == 0) {
		    rs = locinfo_groupname(lip) ;
		    gnl = rs ;
		    gnp = lip->gnbuf ;
		}
		if (rs >= 0) {
	            if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	                if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) > 0) {
	                    const int	ul = USERNAMELEN ;
	                    char	ub[USERNAMELEN+1] ;
	                    while ((rs = locinfo_gmread(lip,&gc,ub,ul)) > 0) {
	                        rs = osetstr_add(nlp,ub,rs) ;
	                        c += rs ;
	                        if (rs < 0) break ;
	                    } /* end while */
	                } /* end if */
	                rs1 = locinfo_gmcurend(lip,&gc) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (gmcursor) */
		} /* end if (ok) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
		if (nl == 0) {
	    	    if ((rs = locinfo_username(lip)) >= 0) {
	        	np = lip->unbuf ;
			nl = rs ;
		    }
		}
	        if ((rs >= 0) && (nl > 0)) {
	            rs = osetstr_add(nlp,np,nl) ;
	            c += rs ;
		}
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadname) */


static int procall(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp)
{
	SYSUSERNAMES	su ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = sysusernames_open(&su,NULL)) >= 0) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;

	    while ((rs = sysusernames_readent(&su,ubuf,ulen)) > 0) {

	        rs = procquery(pip,ofp,ubuf) ;
	        wlen += rs ;

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end while (reading user-names) */

	    rs1 = sysusernames_close(&su) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sysusernames) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procall) */


/* process a name */
static int procquery(PROGINFO *pip,SHIO *ofp,cchar name[])
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (name == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_userxxx/procquery: q=>%s<\n",name) ;
#endif

	if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = rs ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getname(pip,&pw,pwbuf,pwlen,name)) >= 0) {
	            cchar	*pp = NULL ;
	            if ((rs = procselect(pip,&pw)) > 0) {
	                switch (pip->progmode) {
	                case progmode_username:
	                default:
	                    pp = pw.pw_name ;
	                    break ;
	                case progmode_userhome:
	                case progmode_userdir:
	                case progmode_logdir:
	                    pp = pw.pw_dir ;
	                    break ;
	                } /* end switch */
	                if ((rs >= 0) && (pip->verboselevel > 0)) {
		            rs = procout(pip,ofp,&pw,pp) ;
		            wlen += rs ;
	                } /* end if (printing) */
	            } /* end if (procselect) */
	        } /* end if (getname) */
	        rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	} /* end if (getbufsize) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_userxxx/procquery: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int procselect(PROGINFO *pip,struct passwd *pwp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f = TRUE ;
	if (lip->f.all) {
	    if (matstr(specials,pwp->pw_name,-1) >= 0) {
	        f = lip->f.speuser ;
	    } else {
	        if (lip->f.sysuser) f = f && (pwp->pw_uid < SYSUID) ;
	        if (lip->f.reguser) f = f && (pwp->pw_uid >= SYSUID) ;
	        if (lip->f.speuser) f = FALSE ;
	    }
	} /* end if (type of user) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procselect) */


static int procout(PROGINFO *pip,SHIO *ofp,struct passwd *pwp,cchar *pp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f = FALSE ;
	cchar		*fmt ;
	f = f || lip->f.realname ;
	f = f || lip->f.name ;
	f = f || lip->f.fullname ;
	if (f) {
	    const int	nlen = REALNAMELEN ;
	    int		nl = 0 ;
	    cchar	*un = pwp->pw_name ;
	    char	nbuf[REALNAMELEN+1] ;
	    fmt = "%-16s %s\n" ;
	    if (lip->f.realname) {
	        cchar	*gecos = pwp->pw_gecos ;
	        cchar	*gp ;
	        if ((rs = getgecosname(gecos,-1,&gp)) > 0) {
	            if ((rs = mkrealname(nbuf,nlen,gp,rs)) > 0) {
	                nl = rs ;
	            } /* end if (mkrealname) */
	        } /* end if (getgecosname) */
	    } else if (lip->f.name || lip->f.fullname) {
	        int 	w = pcsnsreq_fullname ;
	        if (lip->f.name) w = pcsnsreq_pcsname ;
	        if ((rs = locinfo_prpcs(lip)) >= 0) {
	            rs = procgetns(pip,nbuf,nlen,un,w) ;
	            nl = rs ;
	        }
	    }
	    if (rs >= 0) {
	        rs = shio_printf(ofp,fmt,un,nbuf,nl) ;
	        wlen += rs ;
	    }
	} else if (lip->f.org || lip->f.projinfo) {
	    const int	nlen = REALNAMELEN ;
	    int		nl = 0 ;
	    cchar	*un = pwp->pw_name ;
	    char	nbuf[REALNAMELEN+1] ;
	    fmt = "%-16s %s\n" ;
	    int 	w = pcsnsreq_pcsorg ;
	    if (lip->f.projinfo) w = pcsnsreq_projinfo ;
	    if ((rs = locinfo_prpcs(lip)) >= 0) {
	        if ((rs = procgetns(pip,nbuf,nlen,un,w)) >= 0) {
	            nl = rs ;
	            rs = shio_printf(ofp,fmt,un,nbuf,nl) ;
	            wlen += rs ;
		}
	    }
	} else {
	    cchar	*un = ((pp != NULL) ? pp : "") ;
	    rs = shio_print(ofp,un,-1) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procgetns(PROGINFO *pip,char *nbuf,int nlen,cchar *un,int w)
{
	LOCINFO		*lip = pip->lip ;
	return locinfo_pcsnsget(lip,nbuf,nlen,un,w) ;
}
/* end subroutine (procgetns) */


static int getname(pip,pwp,pwbuf,pwlen,name)
PROGINFO	*pip ;
struct passwd	*pwp ;
char		pwbuf[] ;
int		pwlen ;
cchar		name[] ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((strcmp(name,"-") == 0) || (name[0] == '\0')) {
	    rs = getpwusername(pwp,pwbuf,pwlen,-1) ;
	} else {
	    rs = GETPW_NAME(pwp,pwbuf,pwlen,name) ;
	    if ((rs == SR_NOTFOUND) && hasalldig(name,-1)) {
	        int	v ;
	        if ((rs = cfdeci(name,-1,&v)) >= 0) {
	            uid_t	uid = v ;
	            rs = getpwusername(pwp,pwbuf,pwlen,uid) ;
	        }
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_username/getname: getpw() rs=%d\n",rs) ;
#endif

	if ((rs == SR_SRCH) || (rs == SR_NOENT)) {
	    lip->f.nouser = TRUE ;
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (getname) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.ns) {
	    lip->open.ns = FALSE ;
	    rs1 = pcsns_close(&lip->ns) ;
	    if (rs >= 0) rs = rs1 ;
	}

	lip->unbuf[0] = '\0' ;
	if (lip->open.rn) {
	    lip->open.rn = FALSE ;
	    rs1 = sysrealname_close(&lip->rn) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.gm) {
	    lip->open.gm = FALSE ;
	    rs1 = grmems_finish(&lip->gm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
static int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
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


static int locinfo_username(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->unbuf[0] == '\0') {
	    rs = getusername(lip->unbuf,USERNAMELEN,-1) ;
	} else {
	    rs = strlen(lip->unbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_username) */


static int locinfo_groupname(LOCINFO *lip)
{
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->gnbuf[0] == '\0') {
	    rs = getgroupname(lip->gnbuf,GROUPNAMELEN,-1) ;
	} else {
	    rs = strlen(lip->gnbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_groupname) */


int locinfo_gmcurbegin(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.gm) {
	    const int	max = 20 ;
	    const int	ttl = (12*3600) ;
	    rs = grmems_start(&lip->gm,max,ttl) ;
	    lip->open.gm = (rs >= 0) ;
	}

	if (rs >= 0)
	    rs = grmems_curbegin(&lip->gm,&curp->gmcur) ;

	return rs ;
}
/* end subroutine (locinfo_gmcurbegin) */


int locinfo_gmcurend(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = grmems_curend(&lip->gm,&curp->gmcur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_gmcurend) */


int locinfo_gmlook(LOCINFO *lip,LOCINFO_GMCUR *curp,cchar *gnp,int gnl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if ((rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char *ubuf,int ulen)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if ((rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen)) == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmread) */


int locinfo_rncurbegin(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ent\n") ;
#endif
	if (! lip->open.rn) {
	    rs = sysrealname_open(&lip->rn,NULL) ;
	    lip->open.rn = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = sysrealname_curbegin(&lip->rn,&curp->rncur) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_rncurbegin) */


int locinfo_rncurend(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = sysrealname_curend(&lip->rn,&curp->rncur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_rncurend) */


int locinfo_rnlook(LOCINFO *lip,LOCINFO_RNCUR *curp,cchar *gnp,int gnl)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	const int	fo = 0 ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if ((rs = sysrealname_look(&lip->rn,&curp->rncur,fo,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnlook: sysrealname_look() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnlook) */


int locinfo_rnread(LOCINFO *lip,LOCINFO_RNCUR *curp,char ubuf[],int ulen)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((ulen >= 0) && (ulen < USERNAMELEN)) return SR_OVERFLOW ;

	if ((rs = sysrealname_lookread(&lip->rn,&curp->rncur,ubuf)) == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnread: sysrealname_lookread() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnread) */


static int locinfo_prpcs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (lip->pr_pcs == NULL) {
	    const int	plen = MAXPATHLEN ;
	    cchar	*dn = pip->domainname ;
	    char	pbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(pbuf,plen,VARPRPCS,dn)) >= 0) {
	        cchar	**vpp = &lip->pr_pcs ;
	        rs = locinfo_setentry(lip,vpp,pbuf,rs) ;
	    }
	} else {
	    rs = strlen(lip->pr_pcs) ;
	}
	return rs ;
}
/* end subroutine (locinfo_prpcs) */


static int locinfo_pcsns(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.ns) {
	    if ((rs = locinfo_prpcs(lip)) >= 0) {
		cchar	*pr_pcs = lip->pr_pcs ;
	        if ((rs = pcsns_open(&lip->ns,pr_pcs)) >= 0) {
		    lip->open.ns = TRUE ;
		}
	    }
	} /* end if (needed initialization) */
	return rs ;
}
/* end subroutine (locinfo_pcsns) */


static int locinfo_pcsnsget(LOCINFO *lip,char *rbuf,int rlen,cchar *un,int w)
{
	int		rs ;
	if ((rs = locinfo_pcsns(lip)) >= 0) {
	    rs = pcsns_get(&lip->ns,rbuf,rlen,un,w) ;
	}
	return rs ;
}
/* end subroutine (locinfo_pcsnsget) */


