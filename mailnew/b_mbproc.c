/* b_mbproc */

/* process a MAILBOX (in certain ways) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGMALLER	1		/* print out results */
#define	CF_PROCMAILBOX	1		/* |procmailbox()| */
#define	CF_CONFIG	1		/* config */
#define	CF_CONFIGREAD	1		/* |config_read()| */
#define	CF_LOGID	1		/* |locinfo_logid()| */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a MSU program
	out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child processes to exit explicitly.  Child processes are only
	created when run in "daemon" mode.

	Synopsis:

	$ mailnew [-u <user>]


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
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<vechand.h>
#include	<userinfo.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<storebuf.h>
#include	<mailbox.h>
#include	<mailmsghdrs.h>
#include	<mbcache.h>
#include	<char.h>
#include	<bfile.h>
#include	<tmtime.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_mbproc.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	SUBJBUFLEN
#define	SUBJBUFLEN	(2*COLUMNS)
#endif

#ifndef	NOTEBUFLEN
#define	NOTEBUFLEN	COLUMNS
#endif

#define	MAXOVERLEN	22
#define	MAXFROMLEN	35

#define	DEBUGFNAME	"/tmp/mailnew.deb"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config
#define	CONFIG_MAGIC	0x23FFEEDD

#define	PO_MAILDIRS	"maildirs"
#define	PO_MAILUSERS	"mailusers"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	mksublogid(char *,int,const char *,int) ;
extern int	mkcleanline(char *,int,int) ;
extern int	mkmid(char *,int,const char *,const char *,pid_t,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int prsetfname(cchar *,char *,cchar *,int,int,cchar *,cchar *,cchar *) ;
extern int	tolc(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		mbfnames:1 ;
	uint		md:1 ;
	uint		midstore:1 ;
	uint		midtrack:1 ;
	uint		sort:1 ;
	uint		sortrev:1 ;
	uint		nshow:1 ;
	uint		clen:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, final, changed ;
	LOCINFO_FL	open ;
	VECSTR		stores ;
	VECPSTR		mbfnames ;
	STRPACK		midstore ;
	HDB		midtrack ;
	bfile		yfile ;
	const char	*yearbase ;
	const char	**yearfnames ;
	time_t		*yeartimes ;
	time_t		*maxtimes ;
	time_t		maxouttime ;
	int		nyrs ;
	int		yearbegin ;		/* 19xx */
	int		yearcurrent ;		/* 19xx */
	int		nshow ;
	int		pagesize ;
	int		serial ;		/* serial number */
} ;

struct config {
	uint		magic ;
	PROGINFO	*pip ;
	PARAMOPT	*app ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *,PARAMOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *,const char *) ;
static int	procourconf_end(PROGINFO *) ;

static int	procmailbox(PROGINFO *,SHIO *,const char *,int) ;
static int	procmailmsg(PROGINFO *,SHIO *,MBCACHE *,int,int,char *) ;

static int	procout(PROGINFO *,SHIO *,int,char *,
			MBCACHE_SCAN *,offset_t,int) ;
static int	procouter(PROGINFO *,int,SHIO *,char *,offset_t,int) ;
static int	procouthdrs(PROGINFO *,SHIO *,MBCACHE_SCAN *) ;
static int	procouthdrstatus(PROGINFO *,SHIO *,MBCACHE_SCAN *) ;
static int	procouthdrmid(PROGINFO *,SHIO *,MBCACHE_SCAN *) ;

static int	procbase(PROGINFO *,int,char *,MBCACHE_SCAN *,offset_t,int) ;
static int	procbaser(PROGINFO *,int,bfile *,char *,offset_t,int) ;
static int	procbasehdrs(PROGINFO *,bfile *,MBCACHE_SCAN *) ;
static int	procbasehdrstatus(PROGINFO *,bfile *,MBCACHE_SCAN *) ;
static int	procbasehdrmid(PROGINFO *,bfile *,MBCACHE_SCAN *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_midseen(LOCINFO *,const char *,int) ;
static int	locinfo_yearbase(LOCINFO *,const char *,int) ;
static int	locinfo_yearfins(LOCINFO *) ;
static int	locinfo_yfile(LOCINFO *,time_t) ;
static int	locinfo_yclose(LOCINFO *,int) ;
static int	locinfo_nshow(LOCINFO *,const char *) ;

static int	config_start(CONFIG *,PROGINFO *,PARAMOPT *,cchar *) ;
static int	config_findfile(CONFIG *,char *,const char *) ;
static int	config_cookbegin(CONFIG *) ;
static int	config_cookend(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_reader(CONFIG *) ;
static int	config_finish(CONFIG *) ;

#ifdef	COMMENT
static int	config_check(CONFIG *) ;
#endif /* COMMENT */


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"LOGFILE",
	"md",
	"sn",
	"af",
	"ef",
	"of",
	"cf",
	"lf",
	"year",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_logfile,
	argopt_md,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cf,
	argopt_lf,
	argopt_year,
	argopt_overlast
} ;

static const char	*csched[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"maildir",
	"logsize",
	"logfile",
	NULL
} ;

enum cparams {
	cparam_maildir,
	cparam_logsize,
	cparam_logfile,
	cparam_overlast
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
	"md",
	"sort",
	"nshow",
	"clen",
	NULL
} ;

enum akonames {
	akoname_md,
	akoname_sort,
	akoname_nshow,
	akoname_clen,
	akoname_overlast
} ;

#ifdef	COMMENT
static const char	*varmaildirs[] = {
	VARMAILDNAMESP,
	VARMAILDNAMES,
	VARMAILDNAME,
	NULL
} ;
#endif /* COMMENT */

#ifdef	COMMENT
static const char	*varmailusers[] = {
	VARMAILUSERSP,
	VARMAILUSERS,
	NULL
} ;
#endif /* COMMENT */


/* exported subroutines */


int b_mbproc(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_mbproc) */


int p_mbproc(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mbproc) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
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
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*cfname = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*yrbase = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_mbproc: starting DFD=%d\n",rs) ;
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

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

	                case argopt_logfile:
	                    pip->have.logprog = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.logprog = TRUE ;
	                            pip->lfname = avp ;
	                        }
	                    }
	                    break ;

/* mail directory */
	                case argopt_md:
	                    cp = NULL ;
	                    cl = -1 ;
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
	                        const char	*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(&aparams,po,cp,cl) ;
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

/* output file-name */
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

/* configuration file */
	                case argopt_cf:
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                        }
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
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->have.cfname = TRUE ;
	                        pip->final.cfname = TRUE ;
	                        cfname = argp ;
	                    }
	                    break ;

/* log file name */
	                case argopt_lf:
	                    cp = NULL ;
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
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->have.logprog = TRUE ;
	                        pip->lfname = cp ;
	                    }
	                    break ;

/* year-base */
	                case argopt_year:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            yrbase = argp ;
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

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                cfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
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

/* alternate users */
	                    case 'u':
	                        cp = NULL ;
	                        cl = 0 ;
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
	                            const char	*po = PO_MAILUSERS ;
	                            rs = paramopt_loads(&aparams,po,cp,cl) ;
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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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
	    shio_printf(pip->efp, "%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp, "%s: sn=%s\n",pip->progname,pip->searchname) ;
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

	if (f_help || f_usage || f_version)
	    goto retearly ;


	ex = EX_OK ;

/* cotinue intialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (argval == NULL) argval = getourenv(pip->envv,VARNSHOW) ;

	if (rs >= 0) {
	    if ((rs = locinfo_nshow(lip,argval)) >= 0) {
	        rs = procopts(pip,&akopts,&aparams) ;
	    }
	}

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (cfname == NULL) cfname = getourenv(pip->envv,VARCFNAME) ;
	if (cfname == NULL) cfname = CONFIGFNAME ;
	if (cfname != NULL) pip->final.cfname = TRUE ;

	if (pip->lfname == NULL) pip->lfname = getourenv(pip->envv,VARLFNAME) ;

/* OK, we finally do our thing */

	if ((rs >= 0) && (yrbase != NULL)) {
	    rs = locinfo_yearbase(lip,yrbase,-1) ;
	}

/* argument information */

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
		    cchar	*cfn = cfname ;
	    	    if (cfname != NULL) {
	        	if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	        	if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	    	    }
	            if ((rs = procourconf_begin(pip,&aparams,cfn)) >= 0) {
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
	                } /* end if (proglogfile) */
	                rs1 = procourconf_end(pip) ;
		        if (rs >= 0) rs = rs1 ;
	            } /* end if (procourconf) */
	            rs1 = procuserinfo_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        ex = EX_NOUSER ;
	        fmt = "%s: userinfo failure (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_USAGE ;
	    fmt = "%s: invalid argument (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
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

/* early return thing */
retearly:
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mbproc: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    paramopt_finish(&aparams) ;
	    pip->open.aparams = FALSE ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	rs1 = proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugprintf("main/proginfo_finish: ret rs=%d\n",rs1) ;
#endif

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if ((mdiff > 0) || CF_DEBUGMALLER) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
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

	fmt = "%s: USAGE> %s [<mailbox(es)>] [-o <opt(s)>] [-year <base>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop,PARAMOPT *app)
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
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_md:
	                    if (vl > 0) {
	                        cchar	*po = PO_MAILDIRS ;
	                        lip->have.md = TRUE ;
	                        rs = paramopt_loads(app,po,vp,vl) ;
	                    }
	                    break ;

	                case akoname_sort:
	                    if (! lip->final.sort) {
	                        lip->final.sort = TRUE ;
	                        lip->have.sort = TRUE ;
	                        lip->f.sort = TRUE ;
	                        if (vl > 0) {
	                            int	ch = tolc(*vp & 0xff) ;
	                            switch (ch) {
	                            case '0':
	                            case 'n':
	                                lip->f.sort = FALSE ;
	                                break ;
	                            case 'r':
	                                lip->f.sortrev = TRUE ;
	                                break ;
	                            } /* end switch */
	                        }
	                    }
	                    break ;

	                case akoname_nshow:
	                    if (! lip->final.nshow) {
	                        if (vl) {
	                            lip->final.nshow = TRUE ;
	                            lip->have.nshow = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            lip->nshow = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_clen:
	                    if (! lip->final.clen) {
	                        if (vl) {
	                            lip->final.clen = TRUE ;
	                            lip->have.clen = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            lip->f.clen = (rs > 0) ;
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
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        const char	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

#if	CF_LOGID
	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */
#endif /* CF_LOGID */

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
	            const char	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    const char	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int procourconf_begin(PROGINFO *pip,PARAMOPT *app,cchar cfname[])
{
	const int	csize = sizeof(CONFIG) ;
	int		rs = SR_OK ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mbproc/procourconf_begin: ent\n") ;
#endif

#if	CF_CONFIG
	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    pip->config = csp ;
	    if ((rs = config_start(csp,pip,app,cfname)) >= 0) {
#if	CF_CONFIGREAD
	        if ((rs = config_read(csp)) >= 0) {
	            rs = 1 ;
	        }
#endif
	        if (rs < 0)
	            config_finish(csp) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->config = NULL ;
	    }
	} /* end if (memory-allocation) */
#endif /* CF_CONFIG */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mbproc/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mbproc/procourconf_end: config=%u\n",
	        (pip->config != NULL)) ;
#endif

	if (pip->config != NULL) {
	    CONFIG	*csp = pip->config ;
	    rs1 = config_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}

	return rs ;
}
/* end subroutine (procourconf_end) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn,cchar *ofn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	int		f_special = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-')) {
	    f_special = TRUE ;
	    ofn = STDOUTFNAME ;
	}

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
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
			if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procmailbox(pip,ofp,cp,-1) ;
			    wlen += rs ;
			}
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (looping through positional arguments) */
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
	                        rs = procmailbox(pip,ofp,cp,cl) ;
			    	wlen += rs ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while */

	            rs1 = shio_close(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (afile arguments) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;

	    if ((rs >= 0) && (ofn[0] != '*') && (! f_special)) {
	        struct utimbuf	ut ;
	        LOCINFO		*lip = pip->lip ;
	        time_t		mt = lip->maxouttime ;
	        ut.actime = mt ;
	        ut.modtime = mt ;
	        if (mt > 0) {
	            utime(ofn,&ut) ;
		}
	    }

	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	} /* end if (output file open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procargs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return rs ;
}
/* end subroutine (procargs) */


static int procmailbox(PROGINFO *pip,SHIO *ofp,cchar *mbp,int mbl)
{
	LOCINFO		*lip = pip->lip ;
	MAILBOX		mb ;
	int		rs ;
	int		rs1 ;
	int		mbopts = MAILBOX_ORDONLY ;
	int		wlen = 0 ;
	char		mbfname[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procmailbox: mb=%t\n",mbp,mbl) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: mailbox=%t\n",pip->progname,mbp,mbl) ;
	}

	strdcpy1w(mbfname,MAXPATHLEN,mbp,mbl) ;

	if (! lip->f.clen) mbopts |= MAILBOX_ONOCLEN ;

	if ((rs = mailbox_open(&mb,mbfname,mbopts)) >= 0) {
	    MBCACHE	mc ;
	    if ((rs = mbcache_start(&mc,mbfname,0,&mb)) >= 0) {
	        if ((rs = mbcache_count(&mc)) >= 0) {
	            const int	mn = rs ;
	            if ((rs = mbcache_sort(&mc)) >= 0) {
	                if ((rs = uc_open(mbfname,O_RDONLY,0666)) >= 0) {
	                    const int	ps = lip->pagesize ;
	                    const int	mfd = rs ;
	                    char	*mbuf ;
	                    if ((rs = uc_malloc(ps,&mbuf)) >= 0) {
	                        int	mi ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("b_mbproc/procmailbox: "
	                                "mn=%u\n",mn) ;
#endif

	                        for (mi = 0 ; mi < mn ; mi += 1) {

	                            rs = procmailmsg(pip,ofp,&mc,mi,mfd,mbuf) ;
	                            wlen += rs ;

	                            if (rs < 0) break ;
	                        } /* end for */

	                        uc_free(mbuf) ;
	                    } /* end if (memory-allocation-free) */
	                    u_close(mfd) ;
	                } /* end if (file) */
	            } /* end if (sort) */
	        } /* end if (n-msgs) */
	        rs1 = mbcache_finish(&mc) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mbcache) */
	    rs1 = mailbox_close(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailbox-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procmailbox: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmailbox) */


static int procmailmsg(pip,ofp,mcp,mi,mfd,mbuf)
PROGINFO	*pip ;
SHIO		*ofp ;
MBCACHE		*mcp ;
int		mi ;
int		mfd ;
char		*mbuf ;
{
	LOCINFO		*lip = pip->lip ;
	MBCACHE_SCAN	*msp ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procmailmsg: mi=%u\n",mi) ;
#endif

	if ((rs = mbcache_msginfo(mcp,mi,&msp)) >= 0) {
	    const char	*mid = msp->vs[mbcachemf_hdrmid] ;
	    if ((rs = locinfo_midseen(lip,mid,-1)) == 0) {
	        offset_t	mo ;

	        if ((rs = mbcache_msgoff(mcp,mi,&mo)) >= 0) {
	            const int	ml = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_mbproc/procmailmsg: "
	                    "mi=%u mo=%lld ml=%u\n",mi,mo,ml) ;
#endif

	            if (lip->yeartimes != NULL) {
	                rs = procbase(pip,mfd,mbuf,msp,mo,ml) ;
	                wlen += rs ;
	            } else {
	                rs = procout(pip,ofp,mfd,mbuf,msp,mo,ml) ;
	                wlen += rs ;
	            }

	        } /* end if (mbcache_msgoff) */

	    } /* end if (locinfo-midseen) */
	} /* end if (mbcache_msginfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procmailmsg: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmailmsg) */


static int procout(pip,ofp,mfd,mbuf,msp,mo,ml)
PROGINFO	*pip ;
SHIO		*ofp ;
int		mfd ;
char		mbuf[] ;
MBCACHE_SCAN	*msp ;
offset_t	mo ;
int		ml ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (rs >= 0) {
	    const char	*stap = msp->vs[mbcachemf_hdrstatus] ;
	    const char	*midp = msp->vs[mbcachemf_hdrmid] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        if (stap != NULL)
	            debugprintf("b_mbproc/procout: status=>%t<\n",stap,-1) ;
	        if (midp != NULL)
	            debugprintf("b_mbproc/procout: mid=>%t<\n",midp,-1) ;
	    }
#endif

	    if ((stap == NULL) || (midp == NULL)) {
	        const offset_t	ho = msp->hoff ;
	        const offset_t	bo = msp->boff ;
	        const int	hl = msp->hlen ;
	        const int	bl = msp->blen ;
	        int		wl = ((ho+hl)-mo) ;
	        if ((rs = procouter(pip,mfd,ofp,mbuf,mo,wl)) >= 0) {
	            wlen += rs ;
	            if ((rs = procouthdrs(pip,ofp,msp)) >= 0) {
	                wlen += rs ;
	                if ((rs = procouter(pip,mfd,ofp,mbuf,bo,bl)) >= 0) {
	                    wlen += rs ;
	                }
	            }
	        }
	    } else {
	        rs = procouter(pip,mfd,ofp,mbuf,mo,ml) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        time_t	t = msp->htime ;
	        if (t == 0) t = msp->etime ;
	        if (t > lip->maxouttime) lip->maxouttime = t ;
	    }

	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procout: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procouter(pip,mfd,ofp,mbuf,mo,ml)
PROGINFO	*pip ;
int		mfd ;
SHIO		*ofp ;
char		mbuf[] ;
offset_t	mo ;
int		ml ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		ps ;
	int		minlen ;
	int		wlen = 0 ;

	ps = lip->pagesize ;
	while ((rs >= 0) && (ml > 0)) {
	    minlen = MIN(ml,ps) ;
	    rs = u_pread(mfd,mbuf,minlen,mo) ;
	    if (rs <= 0) break ;
	    mo += rs ;
	    ml -= rs ;
	    rs = shio_write(ofp,mbuf,rs) ;
	    wlen += rs ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouter) */


static int procouthdrs(pip,ofp,msp)
PROGINFO	*pip ;
SHIO		*ofp ;
MBCACHE_SCAN	*msp ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*stap = msp->vs[mbcachemf_hdrstatus] ;
	const char	*midp = msp->vs[mbcachemf_hdrmid] ;

	if ((rs >= 0) && (stap == NULL)) {
	    rs = procouthdrstatus(pip,ofp,msp) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && (midp == NULL)) {
	    rs = procouthdrmid(pip,ofp,msp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = shio_putc(ofp,'\n') ; /* EOH */
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouthdrs) */


static int procouthdrstatus(pip,ofp,msp)
PROGINFO	*pip ;
SHIO		*ofp ;
MBCACHE_SCAN	*msp ;
{
	int		rs ;
	int		bl ;
	int		wlen = 0 ;
	const char	*hdr = HN_STATUS ;
	char		bbuf[10+1] ;

	bl = strwset(bbuf,' ',10) - bbuf ;
	rs = shio_printf(ofp,"%s: %t\n",hdr,bbuf,bl) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouthdrstatus) */


static int procouthdrmid(pip,ofp,msp)
PROGINFO	*pip ;
SHIO		*ofp ;
MBCACHE_SCAN	*msp ;
{
	LOCINFO		*lip = pip->lip ;
	pid_t		pid = pip->pid ;
	const int	midlen = LINEBUFLEN ;
	int		serial ;
	int		rs ;
	int		wlen = 0 ;
	const char	*hdr = HN_MESSAGEID ;
	const char	*dn = pip->domainname ;
	const char	*nn = pip->nodename ;
	char		midbuf[LINEBUFLEN+1] ;

	serial = lip->serial++ ;
	if ((rs = mkmid(midbuf,midlen,dn,nn,pid,serial)) >= 0) {
	    rs = shio_printf(ofp,"%s: <%t>\n",hdr,midbuf,rs) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouthdrmid) */


static int procbase(pip,mfd,mbuf,msp,mo,ml)
PROGINFO	*pip ;
int		mfd ;
char		mbuf[] ;
MBCACHE_SCAN	*msp ;
offset_t	mo ;
int		ml ;
{
	LOCINFO		*lip = pip->lip ;
	time_t		t = msp->htime ;
	int		rs ;
	int		wlen = 0 ;

	if (t == 0) t = msp->etime ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    char	tbuf[TIMEBUFLEN+1] ;
	    debugprintf("b_mbproc/procbase: mo=%lld\n",mo) ;
	    debugprintf("b_mbproc/procbase: t=%s\n",
	        timestr_log(t,tbuf)) ;
	}
#endif

	if ((rs = locinfo_yfile(lip,t)) >= 0) {
	    bfile	*yfp = &lip->yfile ;
	    const char	*stap = msp->vs[mbcachemf_hdrstatus] ;
	    const char	*midp = msp->vs[mbcachemf_hdrmid] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        if (stap != NULL)
	            debugprintf("b_mbproc/procbase: status=>%t<\n",stap,-1) ;
	        if (midp != NULL)
	            debugprintf("b_mbproc/procbase: mid=>%t<\n",midp,-1) ;
	    }
#endif

	    if ((stap == NULL) || (midp == NULL)) {
	        const offset_t	ho = msp->hoff ;
	        const offset_t	bo = msp->boff ;
	        const int	hl = msp->hlen ;
	        const int	bl = msp->blen ;
	        int		wl ;
	        wl = ((ho+hl)-mo) ;
	        if ((rs = procbaser(pip,mfd,yfp,mbuf,mo,wl)) >= 0) {
	            wlen += rs ;
	            if ((rs = procbasehdrs(pip,yfp,msp)) >= 0) {
	                wlen += rs ;
	                if ((rs = procbaser(pip,mfd,yfp,mbuf,bo,bl)) >= 0) {
	                    wlen += rs ;
	                }
	            }
	        }
	    } else {
	        rs = procbaser(pip,mfd,yfp,mbuf,mo,ml) ;
	        wlen += rs ;
	    }

	} /* end if (year-file-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procbase: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procbase) */


static int procbaser(pip,mfd,yfp,mbuf,mo,ml)
PROGINFO	*pip ;
int		mfd ;
bfile		*yfp ;
char		mbuf[] ;
offset_t	mo ;
int		ml ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		ps ;
	int		minlen ;
	int		wlen = 0 ;

	ps = lip->pagesize ;
	while ((rs >= 0) && (ml > 0)) {
	    minlen = MIN(ml,ps) ;
	    rs = u_pread(mfd,mbuf,minlen,mo) ;
	    if (rs <= 0) break ;
	    mo += rs ;
	    ml -= rs ;
	    rs = bwrite(yfp,mbuf,rs) ;
	    wlen += rs ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mbproc/procbaser: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procbaser) */


static int procbasehdrs(pip,yfp,msp)
PROGINFO	*pip ;
bfile		*yfp ;
MBCACHE_SCAN	*msp ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*stap = msp->vs[mbcachemf_hdrstatus] ;
	const char	*midp = msp->vs[mbcachemf_hdrmid] ;

	if ((rs >= 0) && (stap == NULL)) {
	    rs = procbasehdrstatus(pip,yfp,msp) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && (midp == NULL)) {
	    rs = procbasehdrmid(pip,yfp,msp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bputc(yfp,'\n') ; /* EOH */
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procbasehdrs) */


static int procbasehdrstatus(pip,yfp,msp)
PROGINFO	*pip ;
bfile		*yfp ;
MBCACHE_SCAN	*msp ;
{
	int		rs ;
	int		bl ;
	int		wlen = 0 ;
	const char	*hdr = HN_STATUS ;
	char		bbuf[10+1] ;

	bl = strwset(bbuf,' ',10) - bbuf ;
	rs = bprintf(yfp,"%s: %t\n",hdr,bbuf,bl) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procbasehdrstatus) */


static int procbasehdrmid(pip,yfp,msp)
PROGINFO	*pip ;
bfile		*yfp ;
MBCACHE_SCAN	*msp ;
{
	LOCINFO		*lip = pip->lip ;
	pid_t		pid = pip->pid ;
	const int	midlen = LINEBUFLEN ;
	int		serial ;
	int		rs ;
	int		wlen = 0 ;
	const char	*hdr = HN_MESSAGEID ;
	const char	*dn = pip->domainname ;
	const char	*nn = pip->nodename ;
	char		midbuf[LINEBUFLEN+1] ;

	serial = lip->serial++ ;
	if ((rs = mkmid(midbuf,midlen,dn,nn,pid,serial)) >= 0) {
	    rs = bprintf(yfp,"%s: <%t>\n",hdr,midbuf,rs) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procbasehdrmid) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	const int	n = 1000 ;
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->pagesize = getpagesize() ;
	lip->f.clen = TRUE ;

	if ((rs = vecstr_start(&lip->stores,0,0)) >= 0) {
	    VECPSTR	*flp = &lip->mbfnames ;
	    lip->open.stores = TRUE ;
	    if ((rs = vecpstr_start(flp,n,0,0)) >= 0) {
	        const int	csize = (n * 60) ;
	        lip->open.mbfnames = TRUE ;
	        if ((rs = strpack_start(&lip->midstore,csize)) >= 0) {
	            HDB		*hdp = &lip->midtrack ;
	            lip->open.midstore = TRUE ;
	            if ((rs = hdb_start(hdp,n,1,NULL,NULL)) >= 0) {
	                lip->open.midtrack = TRUE ;
	            }
	            if (rs < 0) {
	                lip->open.midstore = FALSE ;
	                strpack_finish(&lip->midstore) ;
	            }
	        } /* end if (strpack) */
	        if (rs < 0) {
	            lip->open.mbfnames = FALSE ;
	            vecpstr_finish(flp) ;
	        }
	    } /* end if (mb-file-names) */
	    if (rs < 0) {
	        lip->open.stores = FALSE ;
	        vecstr_finish(&lip->stores) ;
	    }
	} /* end if (vecstr-stores) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) rs = SR_FAULT ;

	if (lip->yearcurrent > 0) {
	    const int	i = (lip->yearcurrent - lip->yearbegin) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/locinfo_finish: yearcurrent\n") ;
#endif
	    rs1 = locinfo_yclose(lip,i) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->yearcurrent = 0 ;
	}

	if (lip->yearbase != NULL) {
	    rs1 = locinfo_yearfins(lip) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.midtrack) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/locinfo_finish: midtrack\n") ;
#endif
	    lip->open.midtrack = FALSE ;
	    rs1 = hdb_finish(&lip->midtrack) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.midstore) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/locinfo_finish: midstore\n") ;
#endif
	    lip->open.midstore = FALSE ;
	    rs1 = strpack_finish(&lip->midstore) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.mbfnames) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/locinfo_finish: mbfnames\n") ;
#endif
	    lip->open.mbfnames = FALSE ;
	    rs1 = vecpstr_finish(&lip->mbfnames) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.stores) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/locinfo_finish: stores\n") ;
#endif
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_yearfins(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) rs = SR_FAULT ;
	if (lip->yearbase != NULL) {

	    if (lip->yearfnames != NULL) {
	        const int	n = lip->nyrs ;
	        int		i ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
		    debugprintf("main/locinfo_finish: yearfnames\n") ;
#endif
	        for (i = 0 ; i < n ; i += 1) {
	            if (lip->yearfnames[i] != NULL) {
	                rs1 = uc_free(lip->yearfnames[i]) ;
	                if (rs >= 0) rs = rs1 ;
	            }
	        } /* end for */
	        rs1 = uc_free(lip->yearfnames) ;
	        if (rs >= 0) rs = rs1 ;
	        lip->yearfnames = NULL ;
	    }

	    if (lip->yeartimes != NULL) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
		    debugprintf("main/locinfo_finish: yeartimes\n") ;
#endif
	        rs1 = uc_free(lip->yeartimes) ;
	        if (rs >= 0) rs = rs1 ;
	        lip->yeartimes = NULL ;
	    }

	} /* end if (initialized) */
	return rs ;
}
/* end subroutine (locinfo_yearfins) */


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


static int locinfo_midseen(LOCINFO *lip,cchar mp[],int ml)
{
	HDB_DATUM	hk, hv ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (ml < 0) {
	    ml = (mp != NULL) ? strlen(mp) : 0 ;
	}

	if ((mp != NULL) && (ml > 0)) {
	    hk.buf = mp ;
	    hk.len = ml ;
	    if ((rs = hdb_fetch(&lip->midtrack,hk,NULL,&hv)) >= 0) {
	        f = TRUE ;
	    } else if (rs == SR_NOTFOUND) {
	        cchar	*cp ;
	        if ((rs = strpack_store(&lip->midstore,mp,ml,&cp)) >= 0) {
	            hk.buf = cp ;
	            hv.buf = cp ;
	            hv.len = ml ;
	            rs = hdb_store(&lip->midtrack,hk,hv) ;
	        } /* end if (strpack_store) */
	    } /* end if */
	} /* end if (no message-id) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_midseen) */


static int locinfo_yearbase(LOCINFO *lip,cchar *sp,int sl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (sp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_yearbase: yb=%t\n",sp,sl) ;
#endif

	if (lip->yearbase == NULL) {
	    if (sl < 0) sl = strlen(sp) ;
	    if (sp[0] != '-') {
	        cchar	**vpp = &lip->yearbase ;
	        if ((rs = locinfo_setentry(lip,vpp,sp,sl)) >= 0) {
	            TMTIME	ts ;
	            const int	yrbegin = YEARTIMEBASE ;
	            if ((rs = tmtime_localtime(&ts,pip->daytime)) >= 0) {
	                const int	yrend = (TM_YEAR_BASE+ts.year+1) ;
	                int		nyrs ;
	                int		ysize ;
	                void		*p ;
    
	                nyrs = (yrend-yrbegin) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("locinfo_yearbase: "
				"yrb=%u yre=%u nyrs=%u\n",
	                        yrbegin,yrend,nyrs) ;
#endif
	                ysize = ((nyrs+1)*2*sizeof(time_t)) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("locinfo_yearbase: ysize=%u\n",ysize) ;
#endif
	                if ((rs = uc_malloc(ysize,&p)) >= 0) {
	                    int		yr = (yrbegin - TM_YEAR_BASE) ;
	                    int		i ;
	                    lip->yeartimes = p ;
	                    lip->maxtimes = (lip->yeartimes + (nyrs+1)) ;
	                    lip->yearbegin = yrbegin ;
	                    lip->nyrs = nyrs ;
	                    ts.sec = 0 ;
	                    ts.min = 0 ;
	                    ts.hour = 0 ;
	                    ts.mday = 1 ;
	                    ts.mon = 0 ;
	                    ts.isdst = 0 ;
	                    for (i = 0 ; (rs >= 0) && (i < nyrs) ; i += 1)  {
	                        time_t	t ;
	                        ts.year = yr++ ;
	                        if ((rs = tmtime_mktime(&ts,&t)) >= 0) {
	                            lip->yeartimes[i] = t ;
#if	CF_DEBUG
	                            if (DEBUGLEVEL(3)) {
	                                char	tbuf[TIMEBUFLEN+1] ;
	                                debugprintf("locinfo_yearbase: "
					    "i=%u t=%s\n",
	                                    i,timestr_log(t,tbuf)) ;
	                            }
#endif
	                        } /* end if (tmtime_mktime) */
	                    } /* end for */
	                    lip->yeartimes[i] = 0 ;
	                    if (rs >= 0) {
	                        const int s = ((nyrs+1)*sizeof(time_t)) ;
	                        memset(lip->maxtimes,0,s) ;
	                    }
	                    if (rs >= 0) {
	                        const int s = ((nyrs+1)*sizeof(char *)) ;
	                        if ((rs = uc_malloc(s,&p)) >= 0) {
				    memset(p,0,s) ;
	                            lip->yearfnames = p ;
				}
	                    }
	                    if (rs < 0) {
	                        uc_free(lip->yeartimes) ;
	                        lip->yeartimes = NULL ;
	                    }
	                } /* end if (memory-allocation) */
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("locinfo_yearbase: "
				"uc_malloc-out rs=%d\n", rs) ;
#endif
	            } /* end if (tmtime-localtime) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("locinfo_yearbase: "
	                    "tmtime_localtime-out rs=%d\n",rs) ;
#endif
	        } /* end if (locinfo_setentry) */
	    } else
		rs = SR_INVALID ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("locinfo_yearbase: _setentry-out rs=%d\n",rs) ;
#endif
	} /* end if (yearbase was given) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("locinfo_yearbase: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_yearbase) */


static int locinfo_yfile(LOCINFO *lip,time_t t)
{
	PROGINFO	*pip = lip->pip ;
	const int	nyrs = lip->nyrs ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    char	tbuf[TIMEBUFLEN+1] ;
	    debugprintf("locinfo_yfile: nyrs=%u t=%s\n",
	        nyrs,timestr_log(t,tbuf)) ;
	}
#endif

	for (i = (nyrs-1) ; i >= 0 ; i -= 1) {
	    f = (t >= lip->yeartimes[i]) ;
	    if (f) break ;
	} /* end for */
	if (! f) i = nyrs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_yfile: yc=%u f=%u i=%u\n",
	        lip->yearcurrent,f,i) ;
#endif

	if (lip->yearcurrent > 0) {
	    int		yr = (lip->yearbegin + i) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_yfile: yc=%d yr=%d\n",
	            lip->yearcurrent,yr) ;
#endif
	    if (yr != lip->yearcurrent) {
	        const int	ii = (lip->yearcurrent - lip->yearbegin) ;
	        rs = locinfo_yclose(lip,ii) ;
	    }
	}

	if (lip->yearcurrent <= 0) {
	    const int	plen = MAXPATHLEN ;
	    int		yr = (lip->yearbegin + i) ;
	    int		pl = 0 ;
	    char	mbfname[MAXPATHLEN+1] ;
	    if (f) {
		const int	dlen = DIGBUFLEN ;
	        char		dbuf[DIGBUFLEN+1] ;
	        if ((rs = ctdeci(dbuf,dlen,yr)) >= 0) {
	            rs = sncpy2(mbfname,plen,lip->yearbase,dbuf) ;
	            pl = rs ;
	        }
	    } else {
	        rs = sncpy2(mbfname,plen,lip->yearbase,"extra") ;
	        pl = rs ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("locinfo_yfile: rs=%d mbfname=%s\n",rs,mbfname) ;
	        debugprintf("locinfo_yfile: i=%u yr=%u\n",i,yr) ;
	    }
#endif
	    if (rs >= 0) {
	        bfile	*yfp = &lip->yfile ;
	        if ((rs = bopen(yfp,mbfname,"wc",0666)) >= 0) {
	            const int	ps = getpagesize() ;
	            int		bs ;
	            bs = (32*ps) ;
	            if ((rs = bcontrol(yfp,BC_BUFSIZE,bs)) >= 0) {
	                const char	*cp ;
	                lip->yearcurrent = yr ;
	                if ((rs = uc_mallocstrw(mbfname,pl,&cp)) >= 0) {
	                    lip->yearfnames[i] = cp ;
	                }
	            } /* end if (set-buffer-size) */
	            if (rs < 0) {
	                lip->yearcurrent = 0 ;
	                bclose(yfp) ;
	            }
	        } /* end if (file-open) */
	    } /* end if (ok) */
	} /* end if (needed to open file) */

	if ((rs >= 0) && (lip->yearcurrent > 0)) {
	    if (t > lip->maxtimes[i]) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            char	tbuf[TIMEBUFLEN+1] ;
	            timestr_log(t,tbuf) ;
	            debugprintf("locinfo_yfile: new maxtime=%s\n",tbuf) ;
	        }
#endif
	        lip->maxtimes[i] = t ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_yfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_yfile) */


static int locinfo_yclose(LOCINFO *lip,int i)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) rs = SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("locinfo_yclose: ent i=%d\n",i) ;
	    debugprintf("locinfo_yclose: fn=%s\n",
	        lip->yearfnames[i]) ;
	}
#endif

	lip->yearcurrent = 0 ;
	rs1 = bclose(&lip->yfile) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_yclose: bclose() rs=%d\n",rs) ;
#endif

	if (lip->yearfnames[i] != NULL) {
	    struct utimbuf	ut ;
	    time_t		mt = lip->maxtimes[i] ;
	    ut.actime = mt ;
	    ut.modtime = mt ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("locinfo_yclose: utime()\n") ;
	    }
#endif
	    if (mt > 0) {
	        rs1 = utime(lip->yearfnames[i],&ut) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_log(mt,tbuf) ;
	        debugprintf("locinfo_yclose: fn=%s\n",lip->yearfnames[i]) ;
	        debugprintf("locinfo_yfile: final maxtime=%s\n",tbuf) ;
	        debugprintf("locinfo_yclose: utime() rs=%d\n",rs1) ;
	    }
#endif
	    rs1 = uc_free(lip->yearfnames[i]) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->yearfnames[i] = NULL ;
	} /* end if (used) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("locinfo_yclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_yclose) */


static int locinfo_nshow(LOCINFO *lip,cchar *sp)
{
	int		rs = SR_OK ;

	if (! lip->final.nshow) {
	    if (sp != NULL) {
	        int	v ;
	        lip->final.nshow = TRUE ;
	        lip->have.nshow = TRUE ;
	        rs = cfdeci(sp,-1,&v) ;
	        lip->nshow = v ;
	    }
	} /* end if (needed 'nshow') */

	return rs ;
}
/* end subroutine (locinfo_nshow) */


/* configuration maintenance */
static int config_start(CONFIG *csp,PROGINFO *pip,PARAMOPT *app,cchar *cfn)
{
	int		rs = SR_OK ;
	char		tmpfname[MAXPATHLEN+1] = { 0 } ;

	if (csp == NULL) return SR_FAULT ;
	if (cfn == NULL) return SR_FAULT ;

	memset(csp,0,sizeof(CONFIG)) ;
	csp->pip = pip ;
	csp->app = app ;

	if (strchr(cfn,'/') == NULL) {
	    rs = config_findfile(csp,tmpfname,cfn) ;
	    if (rs > 0) cfn = tmpfname ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/config_start: mid rs=%d cfn=%s\n",rs,cfn) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: conf=%s\n",
	        pip->progname,cfn) ;
	}

	if (rs >= 0) {
	    const char	**envv = pip->envv ;
	    if ((rs = paramfile_open(&csp->p,envv,cfn)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = TRUE ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

	if (rs >= 0) csp->magic = CONFIG_MAGIC ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/config_start: ret rs=%d f=%u\n",rs,csp->f_p) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_finish(CONFIG *csp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;
	pip = csp->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/config_finish: ent\n") ;
#endif

	if (csp->f_p) {

	    rs1 = config_cookend(csp) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/config_finish: ret rs=%d f=%u\n",rs,csp->f_p) ;
#endif

	return rs ;
}
/* end subroutine (config_finish) */


static int config_findfile(CONFIG *csp,char tbuf[],cchar cfname[])
{
	PROGINFO	*pip = csp->pip ;
	VECSTR		sv ;
	int		rs ;
	int		pl = 0 ;

	tbuf[0] = '\0' ;
	if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	    const int	tlen = MAXPATHLEN ;

	    vecstr_envset(&sv,"p",pip->pr,-1) ;

	    vecstr_envset(&sv,"e","etc",-1) ;

	    vecstr_envset(&sv,"n",pip->searchname,-1) ;

	    rs = permsched(csched,&sv,tbuf,tlen,cfname,R_OK) ;
	    pl = rs ;

	    vecstr_finish(&sv) ;
	} /* end if (finding file) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (config_findfile) */


static int config_cookbegin(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	int		rs ;

	if ((rs = expcook_start(&csp->cooks)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    const char	*ks = "PSNDHRU" ;
	    const char	*vp ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    char	kbuf[2] ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'P':
	            vp = pip->progname ;
	            break ;
	        case 'S':
	            vp = pip->searchname ;
	            break ;
	        case 'N':
	            vp = pip->nodename ;
	            break ;
	        case 'D':
	            vp = pip->domainname ;
	            break ;
	        case 'H':
	            {
	                const char	*nn = pip->nodename ;
	                const char	*dn = pip->domainname ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = pip->pr ;
	            break ;
	        case 'U':
	            vp = pip->username ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(&csp->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        csp->f_cooks = TRUE ;
	    } else
	        expcook_finish(&csp->cooks) ;
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->f_cooks) {
	    csp->f_cooks = FALSE ;
	    rs1 = expcook_finish(&csp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


#ifdef	COMMENT
static int config_check(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {
	    time_t	dt = pip->daytime ;
	    if ((rs = paramfile_check(&csp->p,dt)) > 0)
	        rs = config_read(csp) ;
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* COMMENT */


static int config_read(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	LOCINFO		*lip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",csp->f_p) ;
#endif

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

	if (csp->f_p) {
	    rs = config_reader(csp) ;
	}

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *csp)
{
	PROGINFO	*pip = csp->pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp = &csp->p ;
	PARAMFILE_CUR	cur ;
	const int	vlen = VBUFLEN ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		vl, el ;
	int		v ;
	int		i ;
	int		ml ;
	cchar		*pr = pip->pr ;
	cchar		*sn = pip->searchname ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",csp->f_p) ;
#endif

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;
	for (i = 0 ; cparams[i] != NULL ; i += 1) {

	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = paramfile_fetch(pfp,cparams[i],&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: vbuf=>%t<\n",vbuf,vl) ;
#endif

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {
	                el = expcook_exp(&csp->cooks,0,ebuf,elen,vbuf,vl) ;
	                if (el >= 0) ebuf[el] = '\0' ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	            if (el > 0) {
	                char	tbuf[MAXPATHLEN + 1] ;

	                switch (i) {

	                case cparam_logsize:
	                    if ((rs = cfdecmfi(ebuf,el,&v)) >= 0) {
	                        if (v >= 0) {
	                            switch (i) {
	                            case cparam_logsize:
	                                pip->logsize = v ;
	                                break ;
	                            } /* end switch */
	                        }
	                    } /* end if (valid number) */
	                    break ;

#ifdef	COMMENT
	                case cparam_maildir:
	                    {
	                        int	ml ;
	                        ml = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                            NULL,MAILDNAME,"") ;
	                        if (ml > 0)
	                            rs = procmaildir(pip,tbuf,ml) ;
	                    }
	                    break ;
#endif /* COMMENT */

	                case cparam_logfile:
	                    if (! pip->final.lfname) {
	                        const char	*lfn = pip->lfname ;
	                        const char	*tfn = tbuf ;
	                        pip->final.lfname = TRUE ;
	                        pip->have.lfname = TRUE ;
	                        ml = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                            LOGCNAME,sn,"") ;
	                        if ((lfn == NULL) || 
	                            (strcmp(lfn,tfn) != 0)) {
	                            cchar	**vpp = &pip->lfname ;
	                            cchar	*tp = tbuf ;
	                            pip->changed.lfname = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,tp,ml) ;
	                        }
	                    }
	                    break ;

	                } /* end switch */

	            } /* end if (got one) */

	        } /* end while (fetching) */

	        paramfile_curend(pfp,&cur) ;
	    } /* end if (cursor) */

	    if (rs < 0) break ;
	} /* end for (parameters) */

	return rs ;
}
/* end subroutine (config_reader) */


