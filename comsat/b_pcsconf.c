/* b_pcsconf (PCSCONF) */

/* PCS Configuration */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CHECKONC	0		/* check ONC */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program is used either by programs or a user to retrieve the
	current PCS configuration settings from a PCS configuration file in the
	PCS distribution directory tree.

	Environment variables:

	PCS		root of program files

	Synopsis:

	$ pcsconf [-ROOT program_root] [-C conf] [keyword] [-V?]

	Notes:

	Note the subtle differences between queries:
		org
		pcsdeforg
		pcsorg
	Look at the code below (carefully) for the answer!


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
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<ascii.h>
#include	<field.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<pcsconf.h>
#include	<pcspoll.h>
#include	<pcsns.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_pcsconf.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#ifndef	INET4DOTDECLEN
#define	INET4DOTDECLEN	16
#endif

#define	CONFBUFLEN	(6 * 1024)

#define	LOCALHOST	"localhost"
#define	LOCALHOSTADDR	0x7f000002

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 12)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


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
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	getnodeinfo(cchar *,char *,char *,vecstr *,cchar *) ;
extern int	getclustername(cchar *,char *,int,cchar *) ;
extern int	getlogname(char *,int) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	nisdomainname(char *,int) ;
#if	CF_CHECKONC
extern int	checkonc(cchar *,cchar *,cchar *,cchar *) ;
#endif
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	pcsgetorg(cchar *,char *,int,cchar *) ;
extern int	pcsgetfacility(cchar *,char *,int) ;
extern int	nchr(cchar *,int,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isStrEmpty(cchar *,int) ;

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
extern char	*strnrchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		altuser:1 ;
	uint		squery:1 ;
	uint		list:1 ;
	uint		onckey:1 ;
	uint		prlocal:1 ;
	uint		clustername:1 ;
	uint		nisdomain:1 ;
	uint		org:1 ;
	uint		pcsorg:1 ;
	uint		pcsdeforg:1 ;
	uint		pcsusername:1 ;
	uint		name:1 ;
	uint		fullname:1 ;
	uint		ema:1 ;
	uint		facility:1 ;
	uint		to:1 ;
	uint		ns:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	PCSNS		ns ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*prlocal ;
	cchar		*pr_pcs ;
	cchar		*pcsusername ;
	cchar		*nisdomain ;
	cchar		*systemname ;
	cchar		*clustername ;
	cchar		*nodename ;
	cchar		*org ;
	cchar		*pcsorg ;
	cchar		*pcsdeforg ;
	cchar		*name ;
	cchar		*fullname ;
	cchar		*ema ;
	cchar		*facility ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	int		to ;		/* time-out */
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procpcsdump(PROGINFO *,cchar *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	proclist(PROGINFO *,SHIO *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,SHIO *,cchar *) ;
static int	procqueries(PROGINFO *,void *,cchar *,int) ;
static int	procquery(PROGINFO *,void *,cchar *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procpcsconf_begin(PROGINFO *,PCSCONF *) ;
static int	procpcsconf_end(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_prlocal(LOCINFO *) ;
static int	locinfo_nisdomain(LOCINFO *) ;
static int	locinfo_systemname(LOCINFO *) ;
static int	locinfo_clustername(LOCINFO *) ;
static int	locinfo_nodename(LOCINFO *) ;
static int	locinfo_org(LOCINFO *) ;
static int	locinfo_pcsorg(LOCINFO *) ;
static int	locinfo_pcsdeforg(LOCINFO *) ;
static int	locinfo_name(LOCINFO *) ;
static int	locinfo_fullname(LOCINFO *) ;
static int	locinfo_ema(LOCINFO *) ;
static int	locinfo_pcsids(LOCINFO *) ;
static int	locinfo_pcsusername(LOCINFO *) ;
static int	locinfo_facility(LOCINFO *) ;
static int	locinfo_prpcs(LOCINFO *) ;
static int	locinfo_pcsns(LOCINFO *) ;
static int	locinfo_pcsnsget(LOCINFO *,char *,int,cchar *,int) ;

static int	mkpresent(char *,int,int,int,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"CONFIG",
	"LOGFILE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	"cf",
	"df",
	"dump",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_config,
	argopt_logfile,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
	argopt_cf,
	argopt_df,
	argopt_dump,
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
	"squery",
	NULL
} ;

enum akonames {
	akoname_squery,
	akoname_overlast
} ;

/* define the configuration keywords */
static const char	*qopts[] = {
	"username",
	"nodename",
	"domainname",
	"nisdomain",
	"systemname",
	"clustername",
	"hostname",
	"pcsuid",
	"pcsgid",
	"pcsusername",
	"pcsdeforg",
	"pcsorg",
	"gecosname",
	"realname",
	"name",
	"pcsname",
	"fullname",
	"pcsfullname",
	"mailname",
	"org",
	"ema",
	"logname",
	"facility",
	"pr",
	NULL
} ;

enum qopts {
	qopt_username,
	qopt_nodename,
	qopt_domainname,
	qopt_nisdomain,
	qopt_systemname,
	qopt_clustername,
	qopt_hostname,
	qopt_pcsuid,
	qopt_pcsgid,
	qopt_pcsusername,
	qopt_pcsdeforg,
	qopt_pcsorg,
	qopt_gecosname,
	qopt_realname,
	qopt_name,
	qopt_pcsname,
	qopt_fullname,
	qopt_pcsfullname,
	qopt_mailname,
	qopt_org,
	qopt_ema,
	qopt_logname,
	qopt_facility,
	qopt_pr,
	qopt_overlast
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


int b_pcsconf(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_pcsconf) */


int p_pcsconf(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_pcsconf) */


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
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_USAGE ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*hfname = NULL ;
	cchar		*dfname = NULL ;
	cchar		*un = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_pcsconf: starting DFD=%d\n",rs) ;
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

	            argval = (argp + 1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
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

/* configuration file */
	                case argopt_config:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log file */
	                case argopt_logfile:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->lfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* helpfile */
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

/* configuration file */
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* dump file */
	                case argopt_df:
	                case argopt_dump:
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    f_usage = TRUE ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* list mode */
	                    case 'l':
	                        lip->f.list = TRUE ;
	                        lip->have.list = TRUE ;
	                        lip->final.list = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(argp,argl) ;
	                                lip->f.list = (rs > 0) ;
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

/* only special-queries */
	                    case 's':
	                        lip->f.squery = TRUE ;
	                        lip->final.squery = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.squery = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* alternative username */
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

	        } /* end if (digits or progopts) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: args rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = getourenv(envv,VARERRORFNAME) ;
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
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_pcsconf: debuglevel=%u\n",pip->debuglevel) ;
	    debugprintf("b_pcsconf: sn=%s\n",sn) ;
	}
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program root */

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
	    debugprintf("b_pcsconf: pr=%s\n",pip->pr) ;
	    debugprintf("b_pcsconf: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* user wants help? */

	if (f_help) {
	    if ((hfname == NULL) || (hfname[0] == '\0')) {
	        hfname = HELPFNAME ;
	    }
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,hfname) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,hfname) ;
#endif
	} /* end if (help file) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* continue */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (cfname == NULL) afname = getourenv(pip->envv,VARCFNAME) ;
	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: cfname=%s\n",cfname) ;
#endif

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* before we even try to find out our username, try to get our ONC key */

#if	CF_CHECKONC

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: checkonc() \n") ;
#endif

	rs1 = checkonc(pip->pr,NULL,NULL,NULL) ;
	lip->f.onckey = (rs1 >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: checkonc() rs=%d\n",rs) ;
#endif

#endif /* CF_CHECKONC */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: un=%s\n",un) ;
#endif

	if ((un != NULL) && (un[0] != '-') && (un[0] != '\0')) {
	    lip->f.altuser = TRUE ;
	}

/* go */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: go\n") ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = userinfo_start(&u,un)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            PCSCONF	pc, *pcp = &pc ;
	            cchar	*pr = pip->pr ;
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	                logfile_printf(&pip->lh,"conf=%s",cfname) ;
	                if (pip->debuglevel > 0) {
	                    shio_printf(pip->efp,"%s: conf=%s\n",pn,cfname) ;
	                }
	            }
	            if ((rs = pcsconf_start(pcp,pr,envv,cfname)) >= 0) {
	                pip->pcsconf = pcp ;
	                pip->open.pcsconf = TRUE ;
	                if ((rs = procpcsconf_begin(pip,pcp)) >= 0) {
	                    PCSPOLL	poll ;
	                    cchar	*sn = pip->searchname ;
	                    if ((rs = pcspoll_start(&poll,pcp,sn)) >= 0) {
	                        if ((rs = proglog_begin(pip,&u)) >= 0) {
	                            if ((rs = proguserlist_begin(pip)) >= 0) {
	                                cchar	*afn = afname ;
	                                cchar	*ofn = ofname ;

	                                if (dfname != NULL) {
	                                    rs = procpcsdump(pip,dfname) ;
	                                }

	                                if (rs >= 0) {
	                                    ARGINFO	*aip = &ainfo ;
	                                    BITS	*bop = &pargs ;
	                                    rs = process(pip,aip,bop,ofn,afn) ;
	                                }

#if	CF_DEBUG
	                                if (DEBUGLEVEL(2))
	                                    debugprintf("b_pcsconf: "
	                                        "procargs() rs=%d\n", rs) ;
#endif

	                                rs1 = proguserlist_end(pip) ;
	                                if (rs >= 0) rs = rs1 ;
	                            } /* end if (proguserlist) */
	                            rs1 = proglog_end(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (proglog) */
	                        rs1 = pcspoll_finish(&poll) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (pcspoll) */
	                    rs1 = procpcsconf_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (procpcsconf) */
	                pip->open.pcsconf = FALSE ;
	                pip->pcsconf = NULL ;
	                rs1 = pcsconf_finish(pcp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (pcsconf) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_pcsconf: pcsconf_start() rs=%d\n",rs) ;
#endif

	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if */
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
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
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
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,
	        "%s: exiting ex=%u (%d)\n",pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_pcsconf: exiting ex=%u (%d)\n",ex,rs) ;
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
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("b_pcsconf: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad argument */
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
	int		i ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<keyword(s)> ... ] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s] [-u <altuser>] [-cf <cfile>] [-df <dfile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-lf <lfile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: special built-in query keys are:\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	for (i = 0 ; (rs >= 0) && (qopts[i] != NULL) ; i += 1) {

	    if ((i % USAGECOLS) == 0) {
	        rs = shio_printf(pip->efp,"%s: \t",pip->progname) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;
	        wlen += rs ;
	        if ((rs >= 0) && ((i % USAGECOLS) == 3)) {
	            rs = shio_printf(pip->efp,"\n") ;
	            wlen += rs ;
	        }
	    }

	} /* end for */

	if ((rs >= 0) && ((i % USAGECOLS) != 0)) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutines (usage) */


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
	                case akoname_squery:
	                    if (! lip->final.squery) {
	                        lip->have.squery = TRUE ;
	                        lip->final.squery = TRUE ;
	                        lip->f.squery = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.squery = (rs > 0) ;
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/process: ent ofn=%s\n",ofn) ;
	    debugprintf("main/process: f_list=%u\n",lip->f.list) ;
	}
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    if (lip->f.list) {
	        rs = proclist(pip,ofp) ;
	    } else {
	        rs = procargs(pip,aip,bop,ofp,afn) ;
	    }
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


static int proclist(PROGINFO *pip,SHIO *ofp)
{
	PCSCONF		*pcp = pip->pcsconf ;
	PCSCONF_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/proclist: ent\n") ;
#endif

	if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	    const int	klen = KBUFLEN ;
	    const int	vlen = VBUFLEN ;
	    int		vl ;
	    int		nfs ;
	    char	kbuf[KBUFLEN+1] ;
	    char	vbuf[VBUFLEN+1] ;
	    while (rs >= 0) {
	        vl = pcsconf_enum(pcp,&cur,kbuf,klen,vbuf,vlen) ;
	        if (vl == SR_NOTFOUND) break ;
	        if ((nfs = nchr(vbuf,vl,CH_FS)) > 0) {
	            rs = mkpresent(vbuf,vlen,vl,CH_FS,nfs) ;
	            vl = rs ;
	        }
		if (rs >= 0) {
	            rs = shio_printf(ofp,"%s=%t\n",kbuf,vbuf,vl) ;
	            wlen += rs ;
	        }
	    } /* end while */
	    rs1 = pcsconf_curend(pcp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pcsconf-cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/proclist: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proclist) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,SHIO *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procquery(pip,ofp,cp,-1) ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (loading positional arguments) */
	} /* end if */

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
	                    rs = procqueries(pip,ofp,cp,cl) ;
	                }
	            } /* end if (sfskipwhite) */

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

	return rs ;
}
/* end subroutine (proargs) */


static int procqueries(PROGINFO *pip,void *ofp,cchar *ap,int al)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,ap,al)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procquery(pip,ofp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procqueries) */


static int procquery(PROGINFO *pip,void *ofp,cchar *qp,int ql)
{
	LOCINFO		*lip = pip->lip ;
	PCSCONF		*pcp = pip->pcsconf ;
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl = -1 ;
	int		qi ;
	int		c = 0 ;
	int		wlen = 0 ;
	char		vbuf[VBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procquery: query=%t\n",qp,ql) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: query=%t\n" ;
	    shio_printf(pip->efp,fmt,pn,qp,ql) ;
	}

	if (! lip->f.squery) {
	    PCSCONF_CUR	cur ;
	    int		nfs ;
	    if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	        while ((vl = pcsconf_fetch(pcp,qp,ql,&cur,vbuf,vlen)) >= 0) {
	            c += 1 ;
	            if ((nfs = nchr(vbuf,vl,CH_FS)) > 0) {
	                rs = mkpresent(vbuf,vlen,vl,CH_FS,nfs) ;
	                vl = rs ;
	            }
	            if (rs >= 0) {
	                rs = shio_print(ofp,vbuf,vl) ;
	                wlen += rs ;
	            } /* end if */
	            if (rs < 0) break ;
	        } /* end while (PCS-conf fetch) */
	        rs1 = pcsconf_curend(pcp,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	} /* end if (special-only query) */

	if ((rs >= 0) && (c == 0)) {
	    if ((qi = matstr(qopts,qp,ql)) >= 0) {
	        cchar	*vp = NULL ;
	        int	vl = -1 ;
	        int	v ;

	        switch (qi) {
	        case qopt_nisdomain:
	            if ((rs = locinfo_nisdomain(lip)) >= 0) {
	                vp = lip->nisdomain ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_systemname:
	            if ((rs = locinfo_systemname(lip)) >= 0) {
	                vp = lip->systemname ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_clustername:
	            if ((rs = locinfo_clustername(lip)) >= 0) {
	                vp = lip->clustername ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_pcsuid:
	        case qopt_pcsgid:
	            if ((rs = locinfo_pcsids(lip)) >= 0) {
	                switch (qi) {
	                case qopt_pcsuid:
	                    v = lip->uid_pcs ;
	                    break ;
	                case qopt_pcsgid:
	                    v = lip->gid_pcs ;
	                    break ;
	                }
	                rs = ctdeci(vbuf,vlen,v) ;
			vl = rs ;
	                vp = vbuf ;
	            } /* end if (locinfo-pcsids) */
	            break ;
	        case qopt_pcsusername:
	            if ((rs = locinfo_pcsusername(lip)) >= 0) {
	                vp = lip->pcsusername ;
	                vl = rs ;
	            } /* end if (locinfo-pcsusername) */
	            break ;
	        case qopt_pcsdeforg:
	            if ((rs = locinfo_pcsdeforg(lip)) >= 0) {
	                vp = lip->pcsdeforg ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_pcsorg:
	            if ((rs = locinfo_pcsorg(lip)) >= 0) {
	                vp = lip->pcsorg ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_username:
	            vp = pip->username ;
	            break ;
	        case qopt_nodename:
	            vp = pip->nodename ;
	            break ;
	        case qopt_domainname:
	            vp = pip->domainname ;
	            break ;
	        case qopt_gecosname:
	            vp = pip->gecosname ;
	            break ;
	        case qopt_realname:
	            vp = pip->realname ;
	            break ;
	        case qopt_name:
	        case qopt_pcsname:
	            if ((rs = locinfo_name(lip)) >= 0) {
	                vp = lip->name ;
			vl = rs ;
	            }
	            break ;
	        case qopt_fullname:
	        case qopt_pcsfullname:
	            if ((rs = locinfo_fullname(lip)) >= 0) {
	                vp = lip->fullname ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_mailname:
	            vp = pip->mailname ;
	            break ;
	        case qopt_org:
	            if ((rs = locinfo_org(lip)) >= 0) {
	                vp = lip->org ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_logname:
	            if ((rs = getlogname(vbuf,vlen)) >= 0) {
	                vp = vbuf ;
			vl = rs ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }
	            break ;
	        case qopt_hostname:
	            if (pip->nodename) {
	                cchar	*nn = pip->nodename ;
	                cchar	*dn = pip->domainname ;
	                if (dn != NULL) {
	                    rs = snsds(vbuf,vlen,nn,dn) ;
			    vl = rs ;
	                    vp = vbuf ;
	                }
	            } /* end if (nodename) */
	            break ;
	        case qopt_ema:
	            if (lip->ema == NULL) rs = locinfo_ema(lip) ;
	            vp = lip->ema ;
	            break ;
	        case qopt_facility:
	            if ((rs = locinfo_facility(lip)) >= 0) {
	                vp = lip->facility ;
	                vl = rs ;
	            }
	            break ;
	        case qopt_pr:
	            vp = pip->pr ;
	            break ;
	        } /* end switch */

	        if ((rs >= 0) && (vp != NULL)) {
	            c += 1 ;
	            rs = shio_print(ofp,vp,vl) ;
	            wlen += rs ;
	        }

	    } /* end if (matstr) */
	} /* end if (not found yet) */

	if ((rs >= 0) && (c == 0)) {
	    rs = shio_print(ofp,"*",1) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procquery: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procquery) */


static int procpcsdump(PROGINFO *pip,cchar *dfname)
{
	SHIO		dfile, *dfp = &dfile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((dfname != NULL) && ((dfname[0] == '\0') || (dfname[0] == '-')))
	    dfname = STDOUTFNAME ;

	if (pip->open.pcsconf && (dfname != NULL)) {
	    PCSCONF	*pcp = pip->pcsconf ;
	    if ((rs = shio_open(dfp,dfname,"wct",0666)) >= 0) {
	        PCSCONF_CUR	cur ;
	        if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	            const int	klen = KBUFLEN ;
	            const int	vlen = VBUFLEN ;
	            int		vl ;
	            int		nfs ;
	            char	kbuf[KBUFLEN+1] ;
	            char	vbuf[VBUFLEN+1] ;
	            while (rs >= 0) {
	                vl = pcsconf_enum(pcp,&cur,kbuf,klen,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                if ((nfs = nchr(vbuf,vl,CH_FS)) > 0) {
	                    rs = mkpresent(vbuf,vlen,vl,CH_FS,nfs) ;
	                    vl = rs ;
	                }
			if (rs >= 0) {
	                    rs = shio_printf(dfp,"%s=%t\n",kbuf,vbuf,vl) ;
	                    wlen += rs ;
			}
	            } /* end while */
	            rs1 = pcsconf_curend(pcp,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (cursor) */
	        rs1 = shio_close(dfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: inaccessible dump-file (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: dfile=%s\n",pn,dfname) ;
	    } /* end if (opened-file) */
	} /* end if (PCSCONF opened) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procpcsdump) */


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
	} /* end if (ok) */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*un = pip->username ;
	    shio_printf(pip->efp,"%s: user=%s\n",pn,un) ;
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


static int procpcsconf_begin(PROGINFO *pip,PCSCONF *pcp)
{
	int		rs = SR_OK ;

	if (pip->open.pcsconf) {
	    rs = (pcp != NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        PCSCONF_CUR	cur ;
	        if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	            const int	klen = KBUFLEN ;
	            const int	vlen = VBUFLEN ;
	            int		vl ;
	            char	kbuf[KBUFLEN+1] ;
	            char	vbuf[VBUFLEN+1] ;
	            while (rs >= 0) {
	                vl = pcsconf_enum(pcp,&cur,kbuf,klen,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                debugprintf("main/procpcsconf: pair> %s=%t\n",
	                    kbuf,vbuf,vl) ;
	            } /* end while */
	            pcsconf_curend(pcp,&cur) ;
	        } /* end if (cursor) */
	    }
#endif /* CF_DEBUG */

	} /* end if (configured) */

	return rs ;
}
/* end subroutine (procpcsconf_begin) */


static int procpcsconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procpcsconf_end) */


static int mkpresent(char *vbuf,int vlen,int vl,int sch,int sn)
{
	const int	tlen = (vlen+(3*sn)) ;
	int		rs ;
	char		*tbuf ;

	if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
	    SBUF	b ;
	    if ((rs = sbuf_start(&b,tbuf,tlen)) >= 0) {
	        int	sl = vl ;
	        cchar	*tp ;
	        cchar	*sp = vbuf ;
	        while ((tp = strnchr(sp,sl,sch)) != NULL) {
	            rs = sbuf_strw(&b,sp,(tp-sp)) ;
	            if (rs >= 0) rs = sbuf_strw(&b," ­ ",3) ;
	            sl -= ((tp+1)-sp) ;
	            sp = (tp+1) ;
	            if (rs < 0) break ;
	        } /* end while */
	        if ((rs >= 0) && (sl > 0)) {
	            rs = sbuf_strw(&b,sp,sl) ;
	        }
	        vl = sbuf_finish(&b) ;
	        if (vl >= 0) rs = vl ;
	    } /* end if (sbuf) */
	    if (rs >= 0) {
	        rs = snwcpy(vbuf,vlen,tbuf,vl) ;
	    }
	    uc_free(tbuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mkpresent) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->uid_pcs = -1 ;
	lip->gid_pcs = -1 ;
	lip->to = -1 ;

	return rs ;
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


static int locinfo_prlocal(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.prlocal) {
	    cchar	*var = VARPRLOCAL ;
	    cchar	*dn = pip->domainname ;
	    char	tmpdname[MAXPATHLEN + 1] ;
	    lip->f.prlocal = TRUE ;
	    if ((rs = mkpr(tmpdname,MAXPATHLEN,var,dn)) >= 0) {
	        cchar	**vpp = &lip->prlocal ;
	        rs = locinfo_setentry(lip,vpp,tmpdname,rs) ;
	    }
	} else if (lip->prlocal != NULL) {
	    rs = strlen(lip->prlocal) ;
	}

	return rs ;
}
/* end subroutine (locinfo_prlocal) */


static int locinfo_nisdomain(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (! lip->f.nisdomain) {
	    const int	dlen = MAXHOSTNAMELEN ;
	    int		dl = -1 ;
	    cchar	*dp = NULL ;
	    char	dbuf[MAXHOSTNAMELEN+ 1] ;
	    lip->f.nisdomain = TRUE ;

	    if (! lip->f.altuser) {
	        dp = getourenv(pip->envv,VARNISDOMAIN) ;
	    }

	    if (dp == NULL) {
	        if ((rs1 = nisdomainname(dbuf,dlen)) >= 0) {
	            dl = rs1 ;
	            dp = dbuf ;
	        } else if (pip->open.logprog) {
	            logfile_printf(&pip->lh,"no NIS domain name (%d)",rs1) ;
	        }
	    }

	    if ((rs >= 0) && (dp != NULL)) {
	        cchar	**vpp = &lip->nisdomain ;
	        rs = locinfo_setentry(lip,vpp,dp,dl) ;
	    }

	} else if (lip->nisdomain != NULL) {
	    rs = strlen(lip->nisdomain) ;
	} /* end if (needed NIS domain) */

	return rs ;
}
/* end subroutine (locinfo_nisdomain) */


static int locinfo_systemname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip->systemname == NULL) {
	    if ((rs = locinfo_nodename(lip)) >= 0) {
	        if ((rs = locinfo_prlocal(lip)) >= 0) {
	            cchar	*nn = pip->nodename ;
	            cchar	*prlocal = lip->prlocal ;
	            char	cbuf[NODENAMELEN+1] ;
	            char	sbuf[NODENAMELEN+1] ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("pcsconf/locinfo_systemname: "
	                    "prlocal=%s nn=%s\n",prlocal,nn) ;
#endif
	            if ((rs = getnodeinfo(prlocal,cbuf,sbuf,NULL,nn)) >= 0) {
	                cchar	**vpp = &lip->systemname ;
	                if ((rs = locinfo_setentry(lip,vpp,sbuf,-1)) >= 0) {
			    cchar	*cn = lip->clustername ;
	                    len = rs ;
	                    if (( cn == NULL) && (cbuf[0] != '\0')) {
	                        cchar	**vpp = &lip->clustername ;
	                        rs = locinfo_setentry(lip,vpp,cbuf,-1) ;
	                    }
	                }
	            } /* end if (getnodeinfo) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("pcsconf/locinfo_systemname: "
	                    "getnodeindo-out rs=%d\n",rs) ;
#endif
	        } /* end if (locinfo_prlocal) */
	    } /* end if (locinfo_nodename) */
	} else {
	    len = strlen(lip->systemname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsconf/locinfo_systemname: ret rs=%d len=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_systemname) */


static int locinfo_clustername(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (! lip->f.clustername) {
	    const int	nlen = NODENAMELEN ;
	    int		nl = -1 ;
	    cchar	*np = NULL ;
	    char	nbuf[NODENAMELEN+ 1] ;
	    lip->f.clustername = TRUE ;

	    if (! lip->f.altuser) {
	        np  = getourenv(pip->envv,VARCLUSTER) ;
	    }

	    if (np == NULL) {
	        if ((rs = locinfo_prlocal(lip)) >= 0) {
	            cchar	*nn = pip->nodename ;
	            cchar	*prlocal = lip->prlocal ;
	            rs1 = SR_NOENT ;
	            if ((prlocal != NULL) && (prlocal[0] != '\0')) {
	                rs1 = getclustername(prlocal,nbuf,nlen,nn) ;
	                if (rs1 >= 0) {
	                    nl = rs1 ;
	                    np = nbuf ;
	                } else if (pip->open.logprog) {
			    cchar	*fmt = "no cluster-name (%d)" ;
	                    logfile_printf(&pip->lh,fmt,rs1) ;
	                }
	            }
	        }
	    }

	    if ((rs >= 0) && (np != NULL)) {
	        cchar	**vpp = &lip->clustername ;
	        rs = locinfo_setentry(lip,vpp,np,nl) ;
	    }

	} else if (lip->clustername != NULL) {
	    rs = strlen(lip->clustername) ;
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_clustername) */


static int locinfo_nodename(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (lip->nodename == NULL) {
	    lip->nodename = pip->nodename ;
	    rs = strlen(lip->nodename) ;
	} else {
	    rs = strlen(lip->nodename) ;
	}
	return rs ;
}
/* end subroutine (locinfo_nodename) */


static int locinfo_org(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_pcsconf/locinfo_org: ent\n") ;
#endif

	if (! lip->f.org) {
	    const int	rlen = ORGLEN ;
	    int		rl = -1 ;
	    cchar	*rp = pip->org ;
	    char	rbuf[ORGLEN+ 1] ;
	    lip->f.org = TRUE ;

	    if ((rp == NULL) || (rp[0] == '\0')) {
	        const int	w = pcsnsreq_pcsorg ;
	        cchar		*un = pip->username ;
	        if ((rs = locinfo_pcsnsget(lip,rbuf,rlen,un,w)) > 0) {
	            rl = rs ;
	            rp = rbuf ;
	        } else {
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,"no ORG (%d)",rs) ;
	            }
	            if (isNotPresent(rs)) rs = SR_OK ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(5)) {
	            debugprintf("b_pcsconf/locinfo_org: pcsnsget() rs=%d\n",
	                rs) ;
	            debugprintf("b_pcsconf/locinfo_org: r=>%t<\n",rp,rl) ;
	        }
#endif /* CF_DEBUG */
	    } /* end if */

	    if ((rs >= 0) && (rp != NULL)) {
	        cchar	**vpp = &lip->org ;
	        rs = locinfo_setentry(lip,vpp,rp,rl) ;
	    }

	} else if (lip->org != NULL) {
	    rs = strlen(lip->org) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("b_pcsconf/locinfo_org: ret rs=%d\n",rs) ;
	    debugprintf("b_pcsconf/locinfo_org: org=>%s<\n",lip->org) ;
	}
#endif

	return rs ;
}
/* end subroutine (locinfo_org) */


static int locinfo_pcsorg(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.pcsorg) {
	    const int	rlen = ORGLEN ;
	    int		rl = -1 ;
	    cchar	*rp = NULL ;
	    char	rbuf[ORGLEN+ 1] ;
	    lip->f.pcsorg = TRUE ;

	    if (! lip->f.altuser) {
	        rp = getourenv(pip->envv,VARPCSORG) ;
	    }

	    if (rp == NULL) {
	        const int	w = pcsnsreq_pcsorg ;
	        cchar		*un = pip->username ;
	        if ((rs = locinfo_pcsnsget(lip,rbuf,rlen,un,w)) > 0) {
	            rl = rs ;
	            rp = rbuf ;
	        } else {
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,"no PCSORG (%d)",rs) ;
	            }
	            if (isNotPresent(rs)) rs = SR_OK ;
	        }
	    }

	    if ((rs >= 0) && (rp != NULL)) {
	        cchar	**vpp = &lip->pcsorg ;
	        rs = locinfo_setentry(lip,vpp,rp,rl) ;
	    }

	} else if (lip->pcsorg != NULL) {
	    rs = strlen(lip->pcsorg) ;
	} /* end if (needed PCSUSERORG) */

	return rs ;
}
/* end subroutine (locinfo_pcsorg) */


static int locinfo_pcsdeforg(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.pcsdeforg) {
	    const int	rlen = ORGLEN ;
	    int		rl = -1 ;
	    cchar	*rp = NULL ;
	    char	rbuf[ORGLEN+ 1] ;
	    lip->f.pcsdeforg = TRUE ;

	    if (rp == NULL) {
	        cchar	*un = pip->username ;
	        cchar	*pr = pip->pr ;
	        if ((rs = pcsgetorg(pr,rbuf,rlen,un)) >= 0) {
	            rl = rs ;
	            rp = rbuf ;
	        } else if (isNotPresent(rs)) {
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,"no PCSDEFORG (%d)",rs) ;
	            }
	            rs = SR_OK ;
	        }
	    }

	    if ((rs >= 0) && (rp != NULL)) {
	        cchar	**vpp = &lip->pcsdeforg ;
	        rs = locinfo_setentry(lip,vpp,rp,rl) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("pcsconf/locinfo_pcsdeforg: "
	                "locinfo_setentry() rs=%d\n",rs) ;
#endif
	    }

	} else if (lip->pcsdeforg != NULL) {
	    rs = strlen(lip->pcsdeforg) ;
	} /* end if (needed PCSDEFORG) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsconf/locinfo_pcsdeforg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_pcsdeforg) */


static int locinfo_name(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.name) {
	    const int	rlen = REALNAMELEN ;
	    int		rl = -1 ;
	    cchar	*rp = NULL ;
	    char	rbuf[REALNAMELEN+1] ;
	    lip->f.name = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("pcsconf/locinfo_name: un=%s alt=%u\n",
	            pip->username,lip->f.altuser) ;
#endif

	    if (! lip->f.altuser) {
	        rp = getourenv(pip->envv,VARNAME) ;
	    }

	    if (rp == NULL) {
	        const int	w = pcsnsreq_pcsname ;
	        cchar		*un = pip->username ;
	        if ((rs = locinfo_pcsnsget(lip,rbuf,rlen,un,w)) > 0) {
	            rl = rs ;
	            rp = rbuf ;
	        } else {
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,"no PCS-name (%d)",rs) ;
	            }
	            if (isNotPresent(rs)) rs = SR_OK ;
	        }
	    }

	    if ((rs >= 0) && (rp != NULL)) {
	        cchar	**vpp = &lip->name ;
	        rs = locinfo_setentry(lip,vpp,rp,rl) ;
	    }

	} else if (lip->name != NULL) {
	    rs = strlen(lip->name) ;
	} /* end if (needed NAME) */

	return rs ;
}
/* end subroutine (locinfo_name) */


static int locinfo_fullname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.fullname) {
	    const int	rlen = REALNAMELEN ;
	    int		rl = -1 ;
	    cchar	*rp = NULL ;
	    char	rbuf[REALNAMELEN+1] ;
	    lip->f.fullname = TRUE ;

	    if (! lip->f.altuser) {
	        rp = getourenv(pip->envv,VARFULLNAME) ;
	    }

	    if (rp == NULL) {
	        const int	w = pcsnsreq_fullname ;
	        cchar		*un = pip->username ;
	        if ((rs = locinfo_pcsnsget(lip,rbuf,rlen,un,w)) > 0) {
	            rl = rs ;
	            rp = rbuf ;
	        } else {
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,"no PCS-fullname (%d)",rs) ;
	            }
	            if (isNotPresent(rs)) rs = SR_OK ;
	        }
	    }

	    if ((rs >= 0) && (rp != NULL)) {
	        cchar	**vpp = &lip->fullname ;
	        rs = locinfo_setentry(lip,vpp,rp,rl) ;
	    }

	} else if (lip->fullname != NULL) {
	    rs = strlen(lip->fullname) ;
	} /* end if (needed FULLNAME) */

	return rs ;
}
/* end subroutine (locinfo_fullname) */


static int locinfo_ema(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.ema) {
	    const int	dlen = MAXHOSTNAMELEN ;
	    int		dl = -1 ;
	    cchar	*dp = NULL ;
	    char	dbuf[MAXHOSTNAMELEN+ 1] ;
	    lip->f.ema = TRUE ;

	    if (dp == NULL) {
	        if ((rs = locinfo_clustername(lip)) >= 0) {
		    if (lip->clustername != NULL) {
	                cchar	*cn = lip->clustername ;
	                cchar	*un = pip->username ;
	                rs = sncpy3(dbuf,dlen,cn,"!",un) ;
	                dl = rs ;
	                dp = dbuf ;
		    }
	        }
	    }

	    if ((rs >= 0) && (dp == NULL)) {
	        cchar	*nn = pip->nodename ;
	        cchar	*un = pip->username ;
	        rs = sncpy3(dbuf,dlen,nn,"!",un) ;
	        dl = rs ;
	        dp = dbuf ;
	    }

	    if ((rs >= 0) && (dp != NULL)) {
	        cchar	**vpp = &lip->ema ;
	        rs = locinfo_setentry(lip,vpp,dp,dl) ;
	    }

	} else if (lip->ema != NULL) {
	    rs = strlen(lip->ema) ;
	} /* end if (needed EMA) */

	return rs ;
}
/* end subroutine (locinfo_ema) */


static int locinfo_pcsids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->uid_pcs < 0) {
	    PCSCONF	*pcp = pip->pcsconf ;
	    if ((rs = pcsconf_getpcsuid(pcp)) >= 0) {
	        lip->uid_pcs = rs ;
	        if ((rs = pcsconf_getpcsgid(pcp)) >= 0) {
	            lip->gid_pcs = rs ;
	        }
	    }
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_pcsids) */


static int locinfo_pcsusername(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! lip->f.pcsusername) {
	    PCSCONF	*pcp = pip->pcsconf ;
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    lip->f.pcsusername = TRUE ;
	    if ((rs = pcsconf_getpcsusername(pcp,ubuf,ulen)) >= 0) {
	        cchar	**vpp = &lip->pcsusername ;
	        rs = locinfo_setentry(lip,vpp,ubuf,rs) ;
	    } /* end if (pcsconf-getpcsusername) */
	} else if (lip->pcsusername != NULL) {
	    rs = strlen(lip->pcsusername) ;
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_pcsusername) */


static int locinfo_facility(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (! lip->f.facility) {
	    PROGINFO	*pip = lip->pip ;
	    const int	flen = MAXNAMELEN ;
	    char	fbuf[MAXNAMELEN+1] ;
	    lip->f.facility = TRUE ;
	    if ((rs = pcsgetfacility(pip->pr,fbuf,flen)) >= 0) {
	        cchar	**vpp = &lip->facility ;
	        rs = locinfo_setentry(lip,vpp,fbuf,rs) ;
	    }
	} else if (lip->facility != NULL) {
	    rs = strlen(lip->facility) ;
	}

	return rs ;
}
/* end subsubroutine (locinfo_facility) */


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
	} /* end if (needed PCSNS open) */
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


