/* b_la */

/* SHELL built-in to return load averages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_PERCACHE	1		/* use persistent cache */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ la <spec(s)>

	Special note:

	Just so an observer (like myself later on) won't go too crazy trying to
	understand what is going on with the PERCAHE local data, it
	is a persistent data structure.  This program can operate as both a
	regular program (is flushed from system memory when it exits) or it can
	be operated as sort of a terminate-stay-resident (TSR) program (its
	data is not flushed when it exists).  We detect which it is (which mode
	we are executing in) dynamically.  We do this by simply looking at the
	persistent data and seeing if elements of it are non-zero.  Any
	non-zero data indicates that we have already been executed in the
	past.  This data is allocated in the BSS section of our process memory
	map so it is initialized to all-zero on program-load (a UNIX® standard
	now for? over twenty years!).

	Hopefully, everything else now makes sense upon inspection with this
	understanding.

	Why do this?  Because it speeds things up.  Everything in this program
	is already quite fast, but we have the chance of reducing some
	file-access work with the introduction of a persistent data cache.  It
	is hard to get faster than a single file-access (like a shared-memory
	cache), so anything worth doing has to be a good bit faster than that.
	Hence, pretty much only TSR behavior can beat a single file access.

	Parallelism?  There isn't any, so we don't have to worry about using
	mutexes or semaphores.  Maybe someday we will have to think about
	parallelism, but probably not any time soon!

	OK, now for some of the unpleasantness.  We have to release persistent
	data that was dynamically allocated with the PERCACHE facility.  We do
	this by calling |percache_fini()| on our particular data handle.  But
	this should be done at module unload time, so we need to register a
	subroutine to do this that is called at module-unload time.  That
	subroutine will be |ourfini()|.  The registration is only done when we
	are *exiting* this command.  This is done so that we do not perform the
	registration if the PERCACHE facility was never used.  The cache
	facility keeps track within itself whether it was used or not (among
	all possible simultaneous users also).  We ask its opinion on that and
	act accordingly.  Only one "user" (command) within the same module will
	be given notice to perform a registration.  However if multiple users
	(commands) do register a |fini()| (needlessly) it is dealt with without
	problems (extras are ignored as expected).  Welcome to TSR management
	when in a very dynamic execution environment!


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
#include	<sys/loadavg.h>
#include	<sys/statvfs.h>
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<utmpacc.h>
#include	<ctdec.h>
#include	<ctdecf.h>
#include	<cthex.h>
#include	<field.h>
#include	<uinfo.h>
#include	<nulstr.h>
#include	<sysmemutil.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_la.h"
#include	"defs.h"
#include	"percache.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CVTBUFLEN	100


/* external subroutines */

extern int	snfsflags(char *,int,ulong) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	statvfsdir(cchar *,struct statvfs *) ;
extern int	getnprocessors(cchar **,int) ;
extern int	getsysdomain(char *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	nusers(cchar *) ;
extern int	isdigitlatin(int) ;
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
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */

#if	CF_PERCACHE
extern PERCACHE	pc ;			/* unitialized it stays in BSS */
#endif


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		percache:1 ;
	uint		init:1 ;
	uint		nocache:1 ;
	uint		utfname:1 ;
	uint		uname:1 ;
	uint		uaux:1 ;
	uint		fla:1 ;
	uint		ncpus:1 ;
	uint		nprocs:1 ;
	uint		btime:1 ;
	uint		runlevel:1 ;
	uint		nusers:1 ;
	uint		rnum:1 ;
	uint		mem:1 ;
	uint		to:1 ;
	uint		hostid:1 ;
	uint		allocfname:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*utfname ;
	cchar		*fname ;
	cchar		*sysdomain ;
	UINFO_NAME	uname ;
	UINFO_AUX	uaux ;
	double		fla[3] ;	/* floating load-averages */
	time_t		btime ;		/* machine boot-time */
	time_t		ti_mem ;
	time_t		ti_la ;
	time_t		ti_ncpus ;
	time_t		ti_nprocs[4] ;
	time_t		ti_btime ;
	time_t		ti_runlevel ;
	time_t		ti_nusers ;
	time_t		ti_hostid ;
	uint		nprocs[4] ;
	uint		ncpus ;
	uint		nusers ;
	uint		rnum ;
	uint		pmt ;		/* physical-memory-total */
	uint		pma ;		/* physical-memory-avail */
	uint		pmu ;		/* physical-memory-usage */
	uint		hostid ;
	int		to ;		/* time-out */
	int		pagesize ;
	int		runlevel ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,void *,cchar *,int) ;
static int	procspec(PROGINFO *,void *, cchar *,int) ;
static int	procfs(PROGINFO *,char *,int,int,cchar *,int) ;
static int	procla(PROGINFO *,SHIO *,char *,int,int) ;
static int	procout(PROGINFO *,SHIO *,cchar *,int) ;

static int	getla(PROGINFO *) ;
static int	getsysmisc(PROGINFO *) ;
static int	getnusers(PROGINFO *) ;
static int	getnprocs(PROGINFO *,int) ;
static int	getnprocs_all(PROGINFO *) ;
static int	getncpus(PROGINFO *) ;
static int	getbtime(PROGINFO *) ;
static int	getrnum(PROGINFO *) ;
static int	getmem(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_utfname(LOCINFO *,cchar *) ;
static int	locinfo_flags(LOCINFO *,int,int) ;
static int	locinfo_to(LOCINFO *,int) ;
static int	locinfo_defaults(LOCINFO *) ;
static int	locinfo_uname(LOCINFO *) ;
static int	locinfo_uaux(LOCINFO *) ;
static int	locinfo_sysdomain(LOCINFO *) ;
static int	locinfo_fsdir(LOCINFO *) ;
static int	locinfo_hostid(LOCINFO *) ;
static int	locinfo_pagesize(LOCINFO *) ;

#if	CF_PERCACHE
static void	ourfini() ;
#endif /* CF_PERCACHE */


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
	"utf",
	"db",
	"nocache",
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
	argopt_utf,
	argopt_db,
	argopt_nocache,
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
	"utf",
	"db",
	NULL
} ;

enum akonames {
	akoname_utf,
	akoname_db,
	akoname_overlast
} ;

/* define the configuration keywords */
static const char	*qopts[] = {
	"sysname",
	"nodename",
	"release",
	"version",
	"machine",
	"architecture",
	"platform",
	"provider",
	"hwserial",
	"nisdomain",
	"domainname",
	"la1min",
	"la5min",
	"la15min",
	"nusers",
	"nprocs",
	"naprocs",
	"nsprocs",
	"nuprocs",
	"ntprocs",
	"ncpus",
	"btime",
	"ctime",
	"utime",
	"rnum",
	"pmtotal",
	"pmavail",
	"pmu",
	"mtotal",
	"mavail",
	"mu",
	"lax",
	"fsbs",
	"fspbs",
	"fstotal",
	"fsavail",
	"fsfree",
	"fsused",
	"fsutil",
	"fstype",
	"fsstr",
	"fsid",
	"fsflags",
	"hostid",
	"romserial",
	NULL
} ;

enum qopts {
	qopt_sysname,
	qopt_nodename,
	qopt_release,
	qopt_version,
	qopt_machine,
	qopt_architecture,
	qopt_platform,
	qopt_provider,
	qopt_hwserial,
	qopt_nisdomain,
	qopt_domainname,
	qopt_la1min,
	qopt_la5min,
	qopt_la15min,
	qopt_nusers,
	qopt_nprocs,
	qopt_naprocs,
	qopt_nsprocs,
	qopt_nuprocs,
	qopt_ntprocs,
	qopt_ncpus,
	qopt_btime,
	qopt_ctime,
	qopt_utime,
	qopt_rnum,
	qopt_pmtotal,
	qopt_pmavail,
	qopt_pmu,
	qopt_mtotal,
	qopt_mavail,
	qopt_mu,
	qopt_lax,
	qopt_fsbs,
	qopt_fspbs,
	qopt_fstotal,
	qopt_fsavail,
	qopt_fsfree,
	qopt_fsused,
	qopt_fsutil,
	qopt_fstype,
	qopt_fsstr,
	qopt_fsid,
	qopt_fsflags,
	qopt_hostid,
	qopt_romserial,
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


int b_la(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_la) */


int p_la(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_la) */


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
	int		to = -1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_init = FALSE ;
	int		f_nocache = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*utfname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_la: starting DFD=%d\n",rs) ;
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

/* UTMP file */
	                case argopt_utf:
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            utfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                utfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_nocache:
	                    f_nocache = TRUE ;
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

/* file-name (for FS operations) */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->fname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* special initialization for persistent cache */
	                    case 'i':
	                        f_init = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                f_init = (rs > 0) ;
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

/* time-out */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                to = v ;
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_la: debuglevel=%u\n",pip->debuglevel) ;
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
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    rs = printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (rs >= 0) {
	    if ((rs = locinfo_utfname(lip,utfname)) >= 0) {
	        if ((rs = locinfo_flags(lip,f_init,f_nocache)) >= 0) {
	    	    if ((rs = locinfo_to(lip,to)) >= 0) {
	    		if ((rs = procopts(pip,&akopts)) >= 0) {
	    		    rs = locinfo_defaults(lip) ;
			}
		    }
		}
	    }
	} /* end if (ok) */

/* OK, we finally do our thing */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
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
	    debugprintf("b_la: final mallout=%u\n",(mo-mo_start)) ;
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


#if	CF_PERCACHE
/* execute this on module (shared-object) un-load */
void ourfini()
{
	percache_fini(&pc) ;
}
/* end subroutine (ourfini) */
#endif /* CF_PERCACHE */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (rs >= 0) {
	    fmt = "%s: USAGE> %s [<spec(s)> ...] [-af <afile>] [-f <file>]\n" ;
	    rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:  [-utf <utmpx>]\n" ;
	    rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	    rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:   possible specifications are: \n",
	    rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	for (i = 0 ; (rs >= 0) && (qopts[i] != NULL) ; i += 1) {

	    if ((rs >= 0) && ((i % USAGECOLS) == 0)) {
	        rs = shio_printf(pip->efp,"%s: \t",pn) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && ((i % USAGECOLS) == 3)) {
	        rs = shio_printf(pip->efp,"\n") ;
	        wlen += rs ;
	    }

	} /* end for */

	if ((rs >= 0) && ((i % USAGECOLS) != 0)) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

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

	                case akoname_utf:
	                case akoname_db:
	                    if (! lip->final.utfname) {
	                        lip->have.utfname = TRUE ;
	                        lip->final.utfname = TRUE ;
	                        if (vl > 0) {
	                            cchar	**vpp = &lip->utfname ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
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
	int		wlen = 0 ;
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
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procspec(pip,ofp,cp,-1) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */

	        if ((pip->verboselevel > 0) && (pan > 0)) {
	            rs1 = shio_putc(ofp,'\n') ;
	            if (rs >= 0) rs = rs1 ;
	            wlen += rs1 ;
	        }

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
	                        if ((rs = procspecs(pip,ofp,cp,cl)) >= 0) {
	                            wlen += rs ;
	            	    	    rs = shio_putc(ofp,'\n') ;
	            	    	    wlen += rs ;
				}
	                    }
	                }

	                pip->daytime = time(NULL) ;
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
	        shio_printf(pip->efp,"%s: no specifications given\n",pn) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,void *ofp,cchar *lbuf,int len)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


/* process a specification name */
static int procspec(PROGINFO *pip,void *ofp,cchar rp[],int rl)
{
	LOCINFO		*lip = pip->lip ;
	const int	vlen = CVTBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ri ;
	int		vl = 0 ;
	int		cbl = -1 ;
	int		wlen = 0 ;
	cchar		*tp ;
	cchar		*vp = NULL ;
	cchar		*cbp = NULL ;
	char		vbuf[CVTBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_la/procspec: req=>%s<\n",req) ;
#endif

	if (rl < 0) rl = strlen(rp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysval/procspec: req=>%t<\n",rp,rl) ;
#endif

	if ((tp = strnchr(rp,rl,'=')) != NULL) {
	    vl = ((rp+rl)-(tp+1)) ;
	    if (vl) vp = (tp+1) ;
	    rl = (tp-rp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_la/procspec: rk=%t\n",rp,rl) ;
	    if (vp != NULL)
	        debugprintf("b_la/procspec: rv=%t\n",vp,vl) ;
	}
#endif

	vbuf[0] = '\0' ;
	wlen = 0 ;
	ri = matostr(qopts,2,rp,rl) ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: spec=%t (%d)\n",
	        pip->progname,rp,rl,ri) ;
	}

	switch (ri) {
	case qopt_sysname:
	case qopt_nodename:
	case qopt_release:
	case qopt_version:
	case qopt_machine:
	    if ((rs = locinfo_uname(lip)) >= 0) {
	        switch (ri) {
	        case qopt_sysname:
	            cbp = lip->uname.sysname ;
	            break ;
	        case qopt_nodename:
	            cbp = lip->uname.nodename ;
	            break ;
	        case qopt_release:
	            cbp = lip->uname.release ;
	            break ;
	        case qopt_version:
	            cbp = lip->uname.version ;
	            break ;
	        case qopt_machine:
	            cbp = lip->uname.machine ;
	            break ;
	        } /* end switch */
	    } /* end if (uname) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_sysval/procspec: locinfo_uname() rs=%d\n",rs) ;
#endif
	    break ;
	case qopt_architecture:
	case qopt_platform:
	case qopt_provider:
	case qopt_hwserial:
	case qopt_nisdomain:
	    if ((rs = locinfo_uaux(lip)) >= 0) {
	        switch (ri) {
	        case qopt_architecture:
	            cbp = lip->uaux.architecture ;
	            break ;
	        case qopt_platform:
	            cbp = lip->uaux.platform ;
	            break ;
	        case qopt_provider:
	            cbp = lip->uaux.provider ;
	            break ;
	        case qopt_hwserial:
	            cbp = lip->uaux.hwserial ;
	            break ;
	        case qopt_nisdomain:
	            cbp = lip->uaux.nisdomain ;
	            break ;
	        } /* end switch */
	    } /* end if (uaux) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_sysval/procspec: locinfo_uaux() rs=%d\n",rs) ;
#endif
	    break ;
	case qopt_domainname:
	    if ((rs = locinfo_sysdomain(lip)) >= 0) {
	        cbp = lip->sysdomain ;
	        cbl = rs ;
	    }
	    break ;
	case qopt_la1min:
	case qopt_la5min:
	case qopt_la15min:
	    cbp = vbuf ;
	    rs = procla(pip,ofp,vbuf,vlen,ri) ;
	    wlen += rs ;
	    vbuf[0] = '\0' ;
	    break ;
	case qopt_lax:
	    cbp = vbuf ;
	    {
	        int	i ;
	        int	ris[3] = { qopt_la1min, qopt_la5min, qopt_la15min } ; 
		for (i = 0 ; i < 3 ; i += 1) {
	            ri = ris[i] ;
	            rs = procla(pip,ofp,vbuf,vlen,ri) ;
	            wlen += rs ;
	        } /* end for */
	    }
	    vbuf[0] = '\0' ;
	    break ;
	case qopt_nusers:
	    if ((rs = getnusers(pip)) >= 0) {
	        rs = ctdecui(vbuf,vlen,(uint) rs) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_nprocs:
	case qopt_naprocs:
	    if ((rs = getnprocs(pip,0)) >= 0) {
	        uint	v = rs ;
	        rs = ctdecui(vbuf,vlen,v) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_nsprocs:
	    if ((rs = getnprocs(pip,1)) >= 0) {
	        uint	v = rs ;
	        rs = ctdecui(vbuf,vlen,v) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_nuprocs:
	    if ((rs = getnprocs(pip,2)) >= 0) {
	        uint	v = rs ;
	        rs = ctdecui(vbuf,vlen,v) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_ntprocs:
	    if ((rs = getnprocs(pip,3)) >= 0) {
	        uint	v = rs ;
	        rs = ctdecui(vbuf,vlen,v) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_ncpus:
	    if ((rs = getncpus(pip)) >= 0) {
	        rs = ctdecui(vbuf,vlen,(uint) lip->ncpus) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_btime:
	    if ((rs = getbtime(pip)) >= 0) {
	        rs = ctdecul(vbuf,vlen,(ulong) lip->btime) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_ctime:
	case qopt_utime:
	    rs = ctdecul(vbuf,vlen,(ulong) pip->daytime) ;
	    cbp = vbuf ;
	    break ;
	case qopt_rnum:
	    if ((rs = getrnum(pip)) >= 0) {
	        rs = ctdecul(vbuf,vlen,(ulong) lip->rnum) ;
	        cbp = vbuf ;
	    }
	    break ;
	case qopt_pmtotal:
	case qopt_pmavail:
	case qopt_pmu:
	case qopt_mtotal:
	case qopt_mavail:
	case qopt_mu:
	    if ((rs = getmem(pip)) >= 0) {
	        int	v = -1 ;
	        cbp = vbuf ;
	        switch (ri) {
	        case qopt_pmtotal:
	        case qopt_mtotal:
	            v = lip->pmt ;
	            break ;
	        case qopt_pmavail:
	        case qopt_mavail:
	            v = lip->pma ;
	            break ;
	        case qopt_pmu:
	        case qopt_mu:
	            if ((rs = ctdecui(vbuf,vlen,lip->pmu)) >= 0) {
	                cbl = rs ;
	                vbuf[cbl++] = '%' ;
	                vbuf[cbl] = '\0' ;
			v = -1 ;
	            }
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (v >= 0)) {
	            rs = ctdeci(vbuf,vlen,v) ;
		    cbl = rs ;
		}
	    } /* end if */
	    break ;
	case qopt_fsbs:
	case qopt_fspbs:
	case qopt_fstotal:
	case qopt_fsavail:
	case qopt_fsfree:
	case qopt_fsused:
	case qopt_fsutil:
	case qopt_fstype:
	case qopt_fsstr:
	case qopt_fsid:
	case qopt_fsflags:
	    rs = procfs(pip,vbuf,vlen,ri,vp,vl) ;
	    cbp = vbuf ;
	    cbl = rs ;
	    break ;
	case qopt_hostid:
	case qopt_romserial:
	    if ((rs = locinfo_hostid(lip)) >= 0) {
	        uint	id = lip->hostid ;
	        cbp = vbuf ;
	        if (ri == qopt_romserial) {
	            id &= 0x00FFFFFF ;
	            rs = ctdecui(vbuf,vlen,id) ;
	        } else {
	            rs = cthexui(vbuf,vlen,id) ;
	        }
	    } /* end if */
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs1 = procout(pip,ofp,cbp,cbl) ;
	    wlen += rs1 ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_la/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procfs(PROGINFO *pip,char vbuf[],int vlen,int ri,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cbl = 0 ;
	if ((sp == NULL) || (sl == 0)) {
	    rs = locinfo_fsdir(lip) ;
	    sl = rs ;
	    sp = lip->fname ;
	}
	if (rs >= 0) {
	    NULSTR	vs ;
	    cchar	*vname ;
	    if ((rs = nulstr_start(&vs,sp,sl,&vname)) >= 0) {
	        struct statvfs	fi ;
	        if ((rs1 = statvfsdir(vname,&fi)) >= 0) {
	            LONG	vt ;
	            LONG	v = -1 ;
	            switch (ri) {
	            case qopt_fsbs:
	                v = fi.f_frsize ;
	                break ;
	            case qopt_fspbs:
	                v = fi.f_bsize ;
	                break ;
	            case qopt_fstotal:
	                vt = fi.f_blocks * fi.f_frsize ;
	                v = vt / 1024 ;
	                break ;
	            case qopt_fsavail:
	                vt = fi.f_bavail * fi.f_frsize ;
	                v = vt / 1024 ;
	                break ;
	            case qopt_fsused:
	                vt = (fi.f_blocks - fi.f_bfree) * fi.f_frsize ;
	                v = vt / 1024 ;
	                break ;
	            case qopt_fsfree:
	                vt = fi.f_bfree * fi.f_frsize ;
	                v = vt / 1024 ;
	                break ;
	            case qopt_fsutil:
	                {
	                    LONG	f_bused = fi.f_blocks - fi.f_bavail ;
	                    if (fi.f_blocks != 0) {
	                        int per = +(f_bused * 100) / fi.f_blocks ;
	                        int	cvl = vlen ;
	                        char	*cvp = vbuf ;
	                        if ((rs = ctdecui(cvp,cvl,per)) >= 0) {
	                            cbl = rs ;
	                            vbuf[cbl++] = '%' ;
	                        }
	                    } else {
	                        rs = sncpy1(vbuf,vlen,"na") ;
	                        cbl = rs ;
	                    }
	                }
	                break ;
	            case qopt_fstype:
	                rs = snwcpy(vbuf,vlen,fi.f_basetype,FSTYPSZ) ;
	                cbl = rs ;
	                break ;
	            case qopt_fsstr:
	                rs = snwcpy(vbuf,vlen,fi.f_fstr,32) ;
	                cbl = rs ;
	                break ;
	            case qopt_fsid:
	                rs = ctdecul(vbuf,vlen,fi.f_fsid) ;
	                cbl = rs ;
	                break ;
	            case qopt_fsflags:
	                rs = snfsflags(vbuf,vlen,fi.f_flag) ;
	                cbl = rs ;
	                break ;
	            } /* end switch */
	            if ((rs >= 0) && (v >= 0)) {
	                rs = ctdecull(vbuf,vlen,v) ;
	                cbl = rs ;
	            }
	        } /* end if (statvfsdir) */
	        rs1 = nulstr_finish(&vs) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (nulstr) */
	} /* end if (ok) */
	return (rs >= 0) ? cbl : rs ;
}
/* end subroutine (procfs) */


static int procla(PROGINFO *pip,SHIO *ofp,char vbuf[],int vlen,int ri)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

	vbuf[0] = '\0' ;
	if ((rs = getla(pip)) >= 0) {
	    double	v = -1.0 ;

	    switch (ri) {
	    case qopt_la1min:
	        v = lip->fla[LOADAVG_1MIN] ;
	        break ;
	    case qopt_la5min:
	        v = lip->fla[LOADAVG_5MIN] ;
	        break ;
	    case qopt_la15min:
	        v = lip->fla[LOADAVG_15MIN] ;
	        break ;
	    } /* end switch */

	    if (v > -0.5) {
	        if ((rs = ctdecf(vbuf,vlen,v,'f',7,3,-1)) >= 0) {
		    const int	vl = rs ;
		    if (pip->verboselevel > 0) {
	    		rs = procout(pip,ofp,vbuf,vl) ;
	    		wlen += rs ;
		    }
		}
	    }

	} /* end if (getla) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procla) */


static int procout(PROGINFO *pip,SHIO *ofp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->verboselevel > 0) {
	    if (sp == NULL) {
	        sp = "*" ;
	        sl = 1 ;
	    }
	    rs = shio_printf(ofp," %t",sp,sl) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

	lip->f.percache = TRUE ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

#if	CF_PERCACHE /* register |ourfini()| for mod-unload */
	if (lip->f.percache) {
	    if ((rs1 = percache_finireg(&pc)) > 0) { /* need registration? */
	        rs1 = uc_atexit(ourfini) ;
	    }
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_PERCACHE */

	if ((lip->fname != NULL) && lip->f.allocfname) {
	    rs1 = uc_free(lip->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->fname = NULL ;
	    lip->f.allocfname = FALSE ;
	}

	if (lip->sysdomain != NULL) {
	    rs1 = uc_free(lip->sysdomain) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->sysdomain = NULL ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


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


static int locinfo_utfname(LOCINFO *lip,cchar *utfname)
{

	if (lip == NULL) return SR_FAULT ;

	if (utfname != NULL) {
	    lip->have.utfname = TRUE ;
	    lip->final.utfname = TRUE ;
	    lip->utfname = utfname ;
	}

	return SR_OK ;
}
/* end subroutine (locinfo_utfname) */


static int locinfo_flags(LOCINFO *lip,int f_init,int f_nocache)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	lip->f.init = f_init ;
	lip->f.nocache = f_nocache ;

#if	CF_PERCACHE
	if (f_init) {
	    rs1 = percache_invalidate(&pc) ;
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_PERCACHE */

	return rs ;
}
/* end subroutine (locinfo_flags) */


static int locinfo_to(LOCINFO *lip,int to)
{
	if (to < 0) to = TO_CACHE ;
	lip->to = to ;
	return SR_OK ;
}
/* end subroutine (locinfo_to) */


static int locinfo_defaults(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if ((rs >= 0) && (lip->utfname == NULL) && (! lip->final.utfname)) {
	    cchar	*cp = getourenv(pip->envv,VARUTFNAME) ;
	    if (cp != NULL) {
	        cchar	**vpp = &lip->utfname ;
	        rs = locinfo_setentry(lip,vpp,cp,-1) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_defaults) */


static int locinfo_uname(LOCINFO *lip)
{
	int		rs = 1 ; /* cache-hit indication */

	if (! lip->f.uname) {
	    lip->f.uname = TRUE ;
	    rs = uinfo_name(&lip->uname) ;
	}

	return rs ;
}
/* end subroutine (locinfo_uname) */


static int locinfo_uaux(LOCINFO *lip)
{
	int		rs = 1 ; /* cache-hit indication */

	if (! lip->f.uaux) {
	    lip->f.uaux = TRUE ;
	    rs = uinfo_aux(&lip->uaux) ;
	}

	return rs ;
}
/* end subroutine (locinfo_uaux) */


static int locinfo_sysdomain(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("sysval/locinfo_sysdomain: ent dn=%s\n",
	        lip->sysdomain) ;
#endif

	if (lip->sysdomain == NULL) {
	    const int	dlen = MAXHOSTNAMELEN ;
	    char	dbuf[MAXHOSTNAMELEN+1] ;
	    if ((rs = getsysdomain(dbuf,dlen)) >= 0) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(dbuf,rs,&cp)) >= 0) {
	            lip->sysdomain = cp ;
	            len = (rs-1) ;
	        }
	    } /* end if (getsysdomain) */
	} else {
	    len = strlen(lip->sysdomain) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("sysval/locinfo_sysdomain: ret dn=%s\n",
	        lip->sysdomain) ;
	    debugprintf("sysval/locinfo_sysdomain: ret rs=%d l=%u\n",rs,len) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_sysdomain) */


static int locinfo_fsdir(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip->fname == NULL) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((rs = getuserhome(hbuf,hlen,"-")) >= 0) {
	        cchar	**vpp = &lip->fname ;
	        if ((rs = locinfo_setentry(lip,vpp,hbuf,rs)) >= 0) {
	            lip->f.allocfname = TRUE ;
	        }
	    }
	} else {
	    rs = strlen(lip->fname) ;
	}

	return rs ;
}
/* end subroutine (locinfo_fsdir) */


static int locinfo_hostid(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		to = lip->to ;
	int		f_hostid = lip->init.hostid ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_hostid: init-hostid=%u\n",lip->init.hostid) ;
	    debugprintf("locinfo_hostid: f_percache=%u\n",lip->f.percache) ;
	}
#endif

	if ((! f_hostid) || ((pip->daytime - lip->ti_hostid) >= to)) {
	    uint	uv = 0 ;
	    lip->init.hostid = TRUE ;
	    lip->ti_hostid = pip->daytime ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_hostid: inside\n") ;
#endif

#if	CF_PERCACHE
	    if (lip->f.percache) {
	        rs = percache_gethostid(&pc,pip->daytime,&uv) ;
	    }
#endif /* CF_PERCACHE */

	    if ((rs >= 0) && (uv == 0)) {
	        uv = gethostid() ;
	    }
	    lip->hostid = uv ;

	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_hostid) */


static int locinfo_pagesize(LOCINFO *lip)
{
	if (lip->pagesize == 0) {
	    lip->pagesize = getpagesize() ;
	}
	return lip->pagesize ;
}
/* end subroutine (locinfo_pagesize) */


static int getla(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	const int	to = TO_LOADAVG ;
	int		rs = SR_OK ;

	if ((! lip->init.fla) || ((pip->daytime - lip->ti_la) >= to)) {
	    lip->init.fla = TRUE ;
	    lip->ti_la = pip->daytime ;
	    rs = uc_getloadavg(lip->fla,3) ;
	} /* end if */

	return rs ;
}
/* end subroutine (getla) */


static int getsysmisc(PROGINFO *pip)
{
	int		rs ;

	if ((rs = getncpus(pip)) >= 0) {
	    if ((rs = getnprocs(pip,0)) >= 0) {
	        rs = getbtime(pip) ;
	    }
	}

	return rs ;
}
/* end subroutine (getsysmisc) */


static int getncpus(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		to ;
	int		f_to ;

	to = lip->to ;
	f_to = ((pip->daytime - lip->ti_ncpus) >= to) ;
	if ((! lip->init.ncpus) || f_to) {
	    lip->init.ncpus = TRUE ;
	    lip->ti_ncpus = pip->daytime ;

	    rs = getnprocessors(pip->envv,0) ;
	    lip->ncpus = rs ;

	} /* end if (needed to get some stuff) */

	return (rs >= 0) ? lip->ncpus : rs ;
}
/* end subroutine (getncpus) */


static int getnprocs(PROGINFO *pip,int w)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		to ;
	int		n = 0 ;
	int		f_to ;

	to = lip->to ;
	f_to = ((pip->daytime - lip->ti_nprocs[w]) >= to) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_la/getnprocs: w=%u f_to=%u\n",w,f_to) ;
#endif

	if ((! lip->init.nprocs) || f_to) {
	    lip->init.nprocs = TRUE ;
	    lip->ti_nprocs[w] = pip->daytime ;
	    switch (w) {
	    case 0:
	        rs = getnprocs_all(pip) ;
	        lip->nprocs[w] = rs ;
	        break ;
	    case 1:
	    case 2:
	    case 3:
	        rs = uc_nprocs(w) ;
	        lip->nprocs[w] = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_la/getnprocs: uc_nprocs() rs=%d\n",rs) ;
#endif
	        break ;
	    default:
	        rs = SR_NOSYS ;
	        break ;
	    } /* end switch */
	} /* end if (needed to get some stuff) */
	n = lip->nprocs[w] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_la/getnprocs: rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getnprocs) */


static int getnprocs_all(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		na = 0 ;

#if	CF_PERCACHE
	if (lip->f.percache) {
	    rs = percache_getnprocs(&pc,pip->daytime) ;
	    na = rs ;
	}
#endif /* CF_PERCACHE */

	if ((rs >= 0) && (na == 0)) {
	    rs = uc_nprocs(0) ;
	    na = rs ;
	}

	return (rs >= 0) ? na : rs ;
}
/* end subroutine (getnprocs_all) */


static int getbtime(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		to ;

	to = lip->to ;
	if ((! lip->init.btime) || ((pip->daytime - lip->ti_btime) >= to)) {
	    time_t	bt = 0 ;
	    lip->init.btime = TRUE ;
	    lip->ti_btime = pip->daytime ;

#if	CF_PERCACHE
	    if (lip->f.percache) {
	        rs = percache_getbtime(&pc,pip->daytime,&bt) ;
	    }
#endif /* CF_PERCACHE */

	    if ((rs >= 0) && (bt == 0)) {
	        rs = utmpacc_boottime(&bt) ;
	    }
	    lip->btime = bt ;

	} /* end if */

	return rs ;
}
/* end subroutine (getbtime) */


static int getnusers(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		to ;

	to = lip->to ;
	if ((! lip->init.nusers) || ((pip->daytime - lip->ti_nusers) >= to)) {
	    int		nu = 0 ;
	    lip->init.nusers = TRUE ;
	    lip->ti_nusers = pip->daytime ;

#if	CF_PERCACHE
	    if (lip->f.percache) {
	        rs = percache_getnusers(&pc,pip->daytime) ;
	        nu = rs ;
	    }
#endif /* CF_PERCACHE */

	    if ((rs >= 0) && (nu == 0)) {
	        rs = nusers(lip->utfname) ;
	        nu = rs ;
	    }
	    lip->nusers = nu ;

	} /* end if */

	return (rs >= 0) ? lip->nusers : rs ;
}
/* end subroutine (getnusers) */


/* make a random number (is this already more than is ever needed?) */
static int getrnum(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	struct timeval	tod ;
	uid_t		uid ;
	pid_t		pid ;
	uint		rv ;
	uint		v ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (! lip->init.rnum) {

	    lip->init.rnum = TRUE ;
	    rv = 0 ;

/* these are the same on a given triplet of node-user-process */

	    v = gethostid() ;
	    rv ^= v ;

	    uid = getuid() ;
	    v = uid ;
	    rv += v ;

	    pid = ugetpid() ;
	    v = pid ;
	    rv += v ;

	    pid = getppid() ;
	    v = pid ;
	    rv += v ;

	    lip->rnum = (rv & INT_MAX) ;

	} /* end if */

/* these do shake things up a bit */

	rv = lip->rnum ;
	if ((rs1 = getsysmisc(pip)) >= 0) {
	    rv += rs1 ;
	    rv += lip->ncpus ;
	    rv += lip->btime ;
	}

/* these are somewhat cyclical at the low end */

	uc_gettimeofday(&tod,NULL) ; /* cannot fail? */
	rv ^= tod.tv_sec ;
	rv += tod.tv_usec ;

#if	defined(SYSHAS_HRTIME) && (SYSHAS_HRTIME > 0)
	{
	    hrtime_t	hrt = gethrtime() ;
	    rv ^= hrt ;
	}
#endif /* SYSHAS_HRTIME */

	lip->rnum = (rv & INT_MAX) ;

	return (rs >= 0) ? lip->rnum : rs ;
}
/* end subroutine (getrnum) */


static int getmem(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f_to ;

	f_to = ((pip->daytime - lip->ti_mem) >= lip->to) ;
	if ((! lip->init.mem) || f_to) {
	    if ((rs = locinfo_pagesize(lip)) >= 0) {
		SYSMEMUTIL	sm ;
		const int	ps = rs ;
		lip->init.mem = TRUE ;
		lip->ti_mem = pip->daytime ;
		if ((rs = sysmemutil(&sm)) >= 0) {
		    const long	ppm = ((1024 * 1024) / ps) ;
		    lip->pmu = rs ;
	    	    lip->pmt = (sm.mt / ppm) ;
	    	    lip->pma = (sm.ma / ppm) ;
		} else if (rs == SR_NOSYS) {
		    lip->pmu = 99 ;
		    lip->pmt = 1 ;
		    lip->pma = 1 ;
		}
	    } /* end if (locinfo_pagesize) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (getmem) */


