/* b_varadm */

/* SHELL built-in to return load averages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGSIG	0		/* debug signal handling */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_PERCACHE	1		/* use persistent cache */
#define	CF_GETSYSMISC	1		/* use 'getsysmisc()' */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ la <spec(s)>

	Special note:

	Just so an observer (like myself later on) won't go too
	crazy trying to understand what is going on with the 'struct
	percache' local data, it is a persistent data structure.
	This program can operate as both a regular program (is flushed
	from system memory when it exits) or it can be operated as sort
	of a terminate-stay-resident (TSR) program (its data is not
	flushed when it exists).  We detect which it is (which mode we
	are executing in) dynamically.	We do this by simply looking at
	the persistent data and seeing if elements of it are non-zero.
	Any non-zero data indicates that we have already been executed
	in the past.  This data is allocated in the BSS section of
	our process memory map so it is initialized to all-zero on
	program-load (a UNIX standard now for ? over twenty years!).

	Hopefully, everything else now makes sense upon inspection with
	this understanding.

	Why do this?  Because it speeds things up.  Everything in this
	program is already quite fast, but we have the chance of reducing
	some file-access work with the introduction of a persistent
	data cache.  It is hard to get faster than a single file-access
	(like a shared-memory cache), so anything worth doing has to
	be a good bit faster than that.  Hence, pretty much only TSR
	behavior can beat a single file access.

	Parallelism?  There isn't any, so we don't have to worry about
	using mutexes or semaphores.  Maybe someday we will have to
	think about parallelism, but probably not any time soon!


*****************************************************************************/


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
#include	<sys/loadavg.h>
#include	<sys/statvfs.h>
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<utmpx.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<tmpx.h>
#include	<kinfo.h>
#include	<getsysmisc.h>
#include	<field.h>
#include	<ctdec.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"varadm_config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	CVTBUFLEN	100

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif


/* external subroutines */

extern int	snfsflags(char *,int,ulong) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	statvfsdir(const char *,struct statvfs *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	nusers(const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

enum pertypes {
	pertype_nusers,
	pertype_kinfo,
	pertype_overlast
} ;

struct sysmisc {
	time_t		btime ;
	uint		nprocs ;
	uint		ncpus ;
} ;

struct percache_flags {
	uint		nusers:1 ;
	uint		kinfo:1 ;
} ;

struct perstate {
	time_t		last ;
} ;

struct percache {
	struct percache_flags	f ;
	struct perstate		state[pertype_overlast] ;
	struct sysmisc		sm ;
	time_t		daytime ;	/* current time */
	uint		nusers ;
	char		utfname[MAXPATHLEN + 1] ;
} ;

struct locinfo_flags {
	uint		percache:1 ;
	uint		init:1 ;
	uint		nocache:1 ;
	uint		utfname:1 ;
	uint		fla:1 ;
	uint		nprocs:1 ;
	uint		nusers:1 ;
	uint		ncpus:1 ;
	uint		btime:1 ;
	uint		rnum:1 ;
	uint		mem:1 ;
	uint		to:1 ;
} ;

struct locinfo {
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	init, open ;
	vecstr		stores ;
	struct proginfo	*pip ;
	const char	*utfname ;
	const char	*fname ;
	double		fla[3] ;	/* floating load-averages */
	time_t		btime ;		/* machine boot-time */
	time_t		ti_mem ;
	time_t		ti_la ;
	time_t		ti_nusers ;
	time_t		ti_nprocs ;
	uint		nprocs ;
	uint		ncpus ;
	uint		nusers ;
	uint		rnum ;
	uint		pmt ;		/* physical-memory-total */
	uint		pma ;		/* physical-memory-avail */
	uint		pmu ;		/* physical-memory-usage */
	int		to ;		/* time-out */
	int		pagesize ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procspec(struct proginfo *,void *, const char *) ;
static int	procla(struct proginfo *,SHIO *,char *,int) ;
static int	procout(struct proginfo *,SHIO *,const char *) ;

static int	getla(struct proginfo *) ;
static int	getnusers(struct proginfo *) ;
static int	getnprocs(struct proginfo *) ;
static int	getncpus(struct proginfo *) ;
static int	getbtime(struct proginfo *) ;
static int	getrnum(struct proginfo *) ;
static int	getmem(struct proginfo *) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_utfname(struct locinfo *,const char *) ;
static int	locinfo_flags(struct locinfo *,int,int) ;
static int	locinfo_to(struct locinfo *,int) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
static int	locinfo_defaults(struct locinfo *) ;
static int	locinfo_finish(struct locinfo *) ;

static int	loadkinfo(struct sysmisc *,time_t) ;

#if	CF_PERCACHE
static int	percache_start(struct percache *,time_t,const char *,int) ;
static int	percache_getnusers(struct percache *) ;
static int	percache_getsysmisc(struct percache *,struct sysmisc *) ;
static int	percache_finish(struct percache *) ;
#endif /* CF_PERCACHE */

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"of",
	"ef",
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
	argopt_of,
	argopt_ef,
	argopt_utf,
	argopt_db,
	argopt_nocache,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
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

static const char *akonames[] = {
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
static const char *qopts[] = {
	"la1min",
	"la5min",
	"la15min",
	"nusers",
	"nprocs",
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
	NULL
} ;

enum qopts {
	qopt_la1min,
	qopt_la5min,
	qopt_la15min,
	qopt_nusers,
	qopt_nprocs,
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

static const int	timeouts[] = {
	5, 	/* nusers */
	2, 	/* kinfo */
	0
} ;


/* persistent local variables (special class of local variables) */

#if	CF_PERCACHE
static struct percache	pc ;		/* unitialized it stays in BSS */
#endif


/* exported subroutines */


int b_varadm(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;

	SIGMAN		sm ;

	SHIO		errfile ;
	SHIO		outfile, *ofp = &outfile ;

	KEYOPT		akopts ;

	uint	mo_start = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs, rs1 ;
	int	n, len ;
	int	i, j ;
	int	to = -1 ;
	int	size, v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_init = FALSE ;
	int	f_nocache = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*efname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*utfname = NULL ;
	const char	*cp ;


	if (contextp != NULL) lib_initenviron() ;

	if_exit = 0 ;
	if_int = 0 ;

#if	CF_DEBUGSIG
#else
	rs = sigman_start(&sm,argv[0],
		sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto ret0 ;
#endif

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("b_varadm: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,environ,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
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

		int ch = (argp[1] & 0xff) ;
	        if (isdigit(ch)) {

	            argval = (argp+1) ;

	        } else if (ch == '-') {

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

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
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
	                            rs = cfdeci(avp,avl,&v) ;
	                            pip->verboselevel = v ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            utfname = argp ;
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
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                pip->debuglevel = v ;
				    }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* file-name (for FS operations) */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->fname = argp ;
	                        break ;

/* special initialization for persistent cache */
			    case 'i':
				f_init = TRUE ;
				break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* time-out */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                to = v ;
	                        }
				break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                pip->verboselevel = v ;
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
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varadm: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	locinfo_utfname(lip,utfname) ;

	locinfo_flags(lip,f_init,f_nocache) ;

	locinfo_to(lip,to) ;

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	rs = locinfo_defaults(lip) ;
	if (rs < 0) goto retearly ;

/* OK, we finally do our thing */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	rs = shio_open(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

/* go through the loops */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procspec(pip,ofp,cp) ;
	    if (rs < 0)
	        break ;

	    if (if_int || if_exit)
	        break ;

	} /* end for (handling positional arguments) */

	if ((pip->verboselevel > 0) && (pan > 0))
	    shio_putc(ofp,'\n') ;

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	argfile, *afp = &argfile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
	        FIELD	fsb ;
		const int	llen = LINEBUFLEN ;
	        int	ml ;
	        int	fl ;
	        const char	*fp ;
	        char	lbuf[LINEBUFLEN + 1] ;
	        char	name[MAXNAMELEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((rs = field_start(&fsb,lbuf,len)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	                    if (fl == 0) continue ;

	                    ml = MIN(fl,MAXNAMELEN) ;
	                    strwcpy(name,fp,ml) ;

	                    pan += 1 ;
	                    rs = procspec(pip,ofp,name) ;

			    if (pip->verboselevel > 0)
	    			shio_putc(ofp,'\n') ;

	                    if (if_int || if_exit) break ;
	                    if (fsb.term == '#') break ;
	                    if (rs < 0) break ;
			    pip->daytime = time(NULL) ;
	                } /* end while */

	                field_finish(&fsb) ;
	            } /* end if (field) */

	            if (if_int || if_exit) break ;
		    if (rs < 0) break ;
	        } /* end while (reading lines) */

	        shio_close(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: argument file inaccessible (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: argfile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {
	    rs = SR_INVALID ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: no specifications given\n",
	        pip->progname) ;
	}

	shio_close(ofp) ;

/* finish */
badoutopen:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet)
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
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
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

/* early return thing */
retearly:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

baderropen:
	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_motd: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

#if	CF_DEBUGSIG
#else
	sigman_finish(&sm) ;
#endif

ret0:
	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (b_varadm) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
	if_exit = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	i ;
	int	wlen = 0 ;


	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [<spec(s)> ...] [-af <argfile>] [-f <file>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-db <utmpx>]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:   possible specifications are: \n",
	    pip->progname) ;

	wlen += rs ;
	for (i = 0 ; qopts[i] != NULL ; i += 1) {

	    if ((i % USAGECOLS) == 0) {

	        rs = shio_printf(pip->efp,"%s: \t",pip->progname) ;
	        wlen += rs ;
	    }

	    rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;

	    wlen += rs ;
	    if ((i % USAGECOLS) == 3) {
	        rs = shio_printf(pip->efp,"\n") ;
	        wlen += rs ;
	    }

	} /* end for */

	if ((i % USAGECOLS) != 0) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;

	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;
	const char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	    while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

	        if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	            switch (oi) {

	            case akoname_utf:
	            case akoname_db:
	                if (! lip->final.utfname) {
	                    lip->have.utfname = TRUE ;
	                    lip->final.utfname = TRUE ;
	                    if (vl > 0)
				rs = locinfo_setentry(lip,&lip->utfname,vp,vl) ;
	                }
	                break ;

	            } /* end switch */

	            c += 1 ;

	        } /* end if (valid option) */

		if (rs < 0) break ;

	    } /* end while (looping through key options) */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (key-options) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


/* process a specification name */
static int procspec(pip,ofp,req)
struct proginfo	*pip ;
void		*ofp ;
const char	req[] ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	ri ;
	int	reqlen ;
	int	wlen = 0 ;

	const char	*vp, *tp ;

	char	cvtbuf[CVTBUFLEN + 1] ;
	char	*cbp = NULL ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varadm/procspec: req=>%s<\n",req) ;
#endif

	reqlen = -1 ;
	vp = NULL ;
	if ((tp = strchr(req,'=')) != NULL) {
	    vp = (tp+1) ;
	    reqlen = (tp-req) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_varadm/procspec: rk=%t\n",req,reqlen) ;
		if (vp != NULL)
	    debugprintf("b_varadm/procspec: rv=%s\n",vp) ;
	}
#endif

	cvtbuf[0] = '\0' ;
	wlen = 0 ;
	ri = matostr(qopts,2,req,reqlen) ;

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: spec=%t (%d)\n",
		pip->progname,req,reqlen,ri) ;

	switch (ri) {

	case qopt_la1min:
	case qopt_la5min:
	case qopt_la15min:
	    cbp = cvtbuf ;
	    rs = procla(pip,ofp,cvtbuf,ri) ;
	    wlen += rs ;
	    cvtbuf[0] = '\0' ;
	    break ;

	case qopt_lax:
	    cbp = cvtbuf ;
	    {
		int	i ;
		int	ris[3] = { qopt_la1min, qopt_la5min, qopt_la15min } ;
		for (i = 0 ; i < 3 ; i += 1) {
		    ri = ris[i] ;
	            rs = procla(pip,ofp,cvtbuf,ri) ;
	    	    wlen += rs ;
		} /* end for */
	    }
	        cvtbuf[0] = '\0' ;

	    break ;

	case qopt_nusers:
	    rs = getnusers(pip) ;
	    if (rs >= 0) {
	        rs = ctdecui(cvtbuf,CVTBUFLEN,(uint) rs) ;
	        if (rs >= 0)
	            cbp = cvtbuf ;
	    }
	    break ;

	case qopt_nprocs:
	    rs = getnprocs(pip) ;
	    if (rs >= 0) {
	        rs = ctdecui(cvtbuf,CVTBUFLEN,(uint) lip->nprocs) ;
	        if (rs >= 0)
	            cbp = cvtbuf ;
	    }
	    break ;

	case qopt_ncpus:
	    rs = getncpus(pip) ;
	    if (rs >= 0) {
	        rs = ctdecui(cvtbuf,CVTBUFLEN,(uint) lip->ncpus) ;
	        if (rs >= 0)
	            cbp = cvtbuf ;
	    }
	    break ;

	case qopt_btime:
	    rs = getbtime(pip) ;
	    if (rs >= 0) {
	        rs = ctdecul(cvtbuf,CVTBUFLEN,(ulong) lip->btime) ;
	        if (rs >= 0)
	            cbp = cvtbuf ;
	    }
	    break ;

	case qopt_ctime:
	case qopt_utime:
	    rs = ctdecul(cvtbuf,CVTBUFLEN,(ulong) pip->daytime) ;
	    if (rs >= 0)
	        cbp = cvtbuf ;

	    break ;

	case qopt_rnum:
	    rs = getrnum(pip) ;
	    if (rs >= 0) {
	        rs = ctdecul(cvtbuf,CVTBUFLEN,(ulong) lip->rnum) ;
	        if (rs >= 0)
	            cbp = cvtbuf ;
	    }
	    break ;

	case qopt_pmtotal:
	case qopt_pmavail:
	case qopt_pmu:
	case qopt_mtotal:
	case qopt_mavail:
	case qopt_mu:
	    rs = getmem(pip) ;

	    if (rs >= 0) {

	        int	v = -1 ;

	        cbp = cvtbuf ;
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
	            rs = bufprintf(cvtbuf,CVTBUFLEN,"%u%%",lip->pmu) ;
	            break ;

	        } /* end switch */

		if (v >= 0)
	            rs = ctdeci(cvtbuf,CVTBUFLEN,v) ;

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
	    if (vp == NULL) vp = lip->fname ;
	    if (vp != NULL) {
		struct statvfs	fi ;
		rs1 = statvfsdir(vp,&fi) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varadm/procspec: statvfsdir() rs=%d\n",rs1) ;
#endif
		if (rs1 >= 0) {
		    LONG	vt ;
		    LONG	v = -1 ;
		    cbp = cvtbuf ;
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
	                        rs = bufprintf(cvtbuf,CVTBUFLEN,"%u%%",per) ;
			    } else 
				rs = sncpy1(cvtbuf,CVTBUFLEN,"na") ;
			}
			break ;
		    case qopt_fstype:
			rs = snwcpy(cvtbuf,CVTBUFLEN,fi.f_basetype,FSTYPSZ) ;
			break ;
		    case qopt_fsstr:
			rs = snwcpy(cvtbuf,CVTBUFLEN,fi.f_fstr,32) ;
			break ;
		    case qopt_fsid:
			rs = ctdecul(cvtbuf,CVTBUFLEN,fi.f_fsid) ;
			break ;
		    case qopt_fsflags:
			rs = snfsflags(cvtbuf,CVTBUFLEN,fi.f_flag) ;
			break ;
		    } /* end switch */
		    if (v >= 0)
	                rs = bufprintf(cvtbuf,CVTBUFLEN,"%llu",v) ;
		}
	    }
	    break ;

	default:
	    rs = SR_INVALID ;
	    break ;

	} /* end switch */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs1 = procout(pip,ofp,cbp) ;
	    wlen += rs1 ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varadm/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int procla(pip,ofp,cvtbuf,ri)
struct proginfo	*pip ;
SHIO		*ofp ;
char		cvtbuf[] ;
int		ri ;
{
	struct locinfo	*lip = pip->lip ;

	double	v ;

	int	rs ;
	int	rs1 ;
	int	wlen = 0 ;

	char	*cbp = NULL ;


	cvtbuf[0] = '\0' ;
	        rs = getla(pip) ;

	        v = -1.0 ;
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

	        if ((rs >= 0) && (v > -0.5))
	            rs = bufprintf(cvtbuf,CVTBUFLEN,"%7.3f",v) ;

	if ((rs >= 0) && (cvtbuf[0] != '\0')) cbp = cvtbuf ;

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs1 = procout(pip,ofp,cbp) ;
	    wlen += rs1 ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
} 
/* end subroutine (procla) */


static int procout(pip,ofp,buf)
struct proginfo	*pip ;
SHIO		*ofp ;
const char	*buf ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (pip->verboselevel > 0) {
	    if (buf == NULL) buf = "*" ;
	    rs = shio_printf(ofp," %s",buf) ;
	    wlen += rs ;
	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int	rs ;


	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;
	lip->to = -1 ;

	rs = vecstr_start(&lip->stores,0,0) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (lip == NULL)
	    return SR_FAULT ;

#if	CF_PERCACHE
	if (lip->f.percache && lip->f.nocache) {
	    lip->f.percache = FALSE ;
	    rs1 = percache_finish(&pc) ;
	    if (rs >= 0) rs = rs1 ;
	}
#endif

	rs1 = vecstr_start(&lip->stores,0,0) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_utfname(lip,utfname)
struct locinfo	*lip ;
const char	*utfname ;
{


	if (lip == NULL)
	    return SR_FAULT ;

	if (utfname != NULL) {
	    lip->have.utfname = TRUE ;
	    lip->final.utfname = TRUE ;
	    lip->utfname = utfname ;
	}

	return SR_OK ;
}
/* end subroutine (locinfo_utfname) */


static int locinfo_flags(lip,f_init,f_nocache)
struct locinfo	*lip ;
int		f_init ;
int		f_nocache ;
{


	if (lip == NULL)
	    return SR_FAULT ;

	lip->f.init = f_init ;
	lip->f.nocache = f_nocache ;
	return 0 ;
}
/* end subroutine (locinfo_flags) */


static int locinfo_to(lip,to)
struct locinfo	*lip ;
int		to ;
{


	if (to < 0) to = TO_CACHE ;

	lip->to = to ;
	return SR_OK ;
}
/* end subroutine (locinfo_to) */


static int locinfo_defaults(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;


	if (lip == NULL)
	    return SR_FAULT ;


	if ((lip->utfname == NULL) && (! lip->final.utfname)) {
	    const char	*cp = getenv(VARUTFNAME) ;
	    if (cp != NULL)
		rs = locinfo_setentry(lip,&lip->utfname,cp,-1);
	}

	return rs ;
}
/* end subroutine (locinfo_defaults) */


int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int	rs = SR_OK ;
	int	oi, i ;
	int	vnlen = 0 ;


	if (lip == NULL)
	    return SR_FAULT ;

	if (epp == NULL)
	    return SR_INVALID ;

/* find existing entry for later deletion */

	oi = -1 ;
	if (*epp != NULL)
	    oi = vecstr_findaddr(&lip->stores,*epp) ;

/* add the new entry */

	if (vp != NULL) {

	    vnlen = strnlen(vp,vl) ;

#if	CF_DEBUG && 0
	    {
	        struct proginfo	*pip = lip->pip ;
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/locinfo_setentry: vlen=%u v=>%t<\n",
	                vnlen,vp,vnlen) ;
	    }
#endif

	    rs = vecstr_add(&lip->stores,vp,vnlen) ;
	    i = rs ;
	    if (rs >= 0) {
	        const char	*cp ;

	        rs = vecstr_get(&lip->stores,i,&cp) ;
	        if (rs >= 0)
	            *epp = cp ;

	    } /* end if (added new entry) */

	} /* end if (had a new entry) */

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(&lip->stores,oi) ;

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (locinfo_setentry) */


#if	CF_PERCACHE

static int percache_start(pcp,daytime,utfname,f_init)
struct percache	*pcp ;
time_t		daytime ;
const char	*utfname ;
int		f_init ;
{
	int	rs = SR_OK ;
	int	f_newfile = FALSE ;
	int	f_nullfile = FALSE ;


	if (pcp == NULL)
	    return SR_FAULT ;

	if (daytime == 0)
	     daytime = time(NULL) ;

	if (f_init)
	    memset(pcp,0,sizeof(struct percache)) ;

	pcp->daytime = daytime ;
	f_nullfile = (utfname == NULL) ;
	f_newfile = (f_nullfile && (pcp->utfname[0] != '\0')) ;
	if (! f_newfile) {
	    f_newfile = (! f_nullfile) ;
	    if (f_newfile)
		f_newfile = (strcmp(utfname,pcp->utfname) != 0) ;
	}

	if (f_newfile) {

	    pcp->nusers = 0 ;
	    pcp->f.nusers = FALSE ;
	    pcp->state[pertype_nusers].last = 0 ;
	    pcp->utfname[0] = '\0' ;
	    if (utfname != NULL)
		rs = mkpath1(pcp->utfname,utfname) ;

	} /* end if */

	return rs ;
}
/* end subroutine (percache_start) */


/* this is only called if the "no-cache" option was specified */
static int percache_finish(struct percache *pcp)
{


	if (pcp == NULL)
	    return SR_FAULT ;

	memset(pcp,0,sizeof(struct percache)) ;

	return SR_OK ;
}
/* end subroutine (percache_finish) */


static int percache_getnusers(struct percache *pcp)
{
	time_t	span ;

	int	rs = SR_OK ;
	int	to ;


	if (pcp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("b_varadm/percache_getnusers: entered\n") ;
#endif

	span = (pcp->daytime - pcp->state[pertype_nusers].last) ;
	to = timeouts[pertype_nusers] ;
	if (span > to) {

	    pcp->state[pertype_nusers].last = pcp->daytime ;
	    rs = nusers(pcp->utfname) ;
	    if (rs >= 0)
	        pcp->nusers = rs ;

#if	CF_DEBUGS
	debugprintf("b_varadm/percache_getnusers: nusers() rs=%d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("b_varadm/percache_getnusers: ret rs=%d nusers=%u\n",
		rs,pcp->nusers) ;
#endif

	return (rs >= 0) ? pcp->nusers : rs ;
}
/* end subroutine (percache_getnusers) */


static int percache_getsysmisc(struct percache *pcp,struct sysmisc *smp)
{
	time_t	span ;

	int	rs = SR_OK ;
	int	to ;
	int	n = 0 ;


	if (pcp == NULL)
	    return SR_FAULT ;

	if (smp == NULL)
	    return SR_FAULT ;

	span = (pcp->daytime - pcp->state[pertype_kinfo].last) ;
	to = timeouts[pertype_kinfo] ;
	if (span > to) {

	    pcp->state[pertype_kinfo].last = pcp->daytime ;
	    rs = loadkinfo(&pcp->sm,pcp->daytime) ;
	    if (rs < 0)
		memset(&pcp->sm,0,sizeof(struct sysmisc)) ;

#if	CF_DEBUGS
	debugprintf("b_varadm/percache_getsysmisc: loadkinfo() rs=%d\n",rs) ;
#endif

	} /* end if */

	*smp = pcp->sm ;
	n = pcp->sm.nprocs ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (percache_getsysmisc) */

#endif /* CF_PERCACHE */


static int getla(pip)
struct proginfo	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	to = 1 ;


	if ((! lip->init.fla) || ((pip->daytime - lip->ti_la) >= to)) {

	    lip->init.fla = TRUE ;
	    lip->ti_la = pip->daytime ;
	    rs = uc_getloadavg(lip->fla,3) ;

	} /* end if */

	return rs ;
}
/* end subroutine (getla) */


static int getnusers(pip)
struct proginfo	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	to = lip->to ;


	if ((! lip->init.nusers) || ((pip->daytime - lip->ti_nusers) >= to)) {

	    lip->init.nusers = TRUE ;
	    lip->ti_nusers = pip->daytime ;

#if	CF_PERCACHE
	    if (! lip->f.percache) {
		int f_init = lip->f.init ;
		lip->f.percache = TRUE ;
		rs = percache_start(&pc,pip->daytime,lip->utfname,f_init) ;
	    }

	    if (rs >= 0) {
		rs = percache_getnusers(&pc) ;
	    }
#else /* CF_PERCACHE */
	    rs = nusers(lip->utfname) ;
#endif /* CF_PERCACHE */

	    if (rs >= 0)
	        lip->nusers = rs ;

	} /* end if */

	return (rs >= 0) ? lip->nusers : rs ;
}
/* end subroutine (getnusers) */


static int getnprocs(pip)
struct proginfo	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	struct sysmisc	sm ;

	int	rs = SR_OK ;
	int	to = lip->to ;
	int	f_to ;


	f_to = ((pip->daytime - lip->ti_nprocs) >= to) ;
	if ((! lip->init.nprocs) || (! lip->init.ncpus) || 
	    (! lip->init.btime) || f_to) {

	    lip->init.nprocs = TRUE ;
	    lip->init.ncpus = TRUE ;
	    lip->init.btime = TRUE ;
	    lip->ti_nprocs = pip->daytime ;

#if	CF_PERCACHE
	    if (! lip->f.percache) {
		int f_init = lip->f.init ;
		lip->f.percache = TRUE ;
		rs = percache_start(&pc,pip->daytime,lip->utfname,f_init) ;
	    }

	    if (rs >= 0) {
		rs = percache_getsysmisc(&pc,&sm) ;
	    }
#else /* CF_PERCACHE */
	    rs = loadkinfo(&sm,pip->daytime) ;
#endif /* CF_PERCACHE */

	    if (rs >= 0) {
	        lip->nprocs = sm.nprocs ;
	        lip->ncpus = sm.ncpus ;
	        lip->btime = sm.btime ;
	    }

	} /* end if (needed to get some stuff) */

	return (rs >= 0) ? lip->nprocs : rs ;
}
/* end subroutine (getnprocs) */


static int getncpus(pip)
struct proginfo	*pip ;
{


	return getnprocs(pip) ;
}
/* end subroutine (getncpus) */


static int getbtime(pip)
struct proginfo	*pip ;
{


	return getnprocs(pip) ;
}
/* end subroutine (getbtime) */


/* make a random number (is this already more than is ever needed?) */
static int getrnum(pip)
struct proginfo	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	struct timeval	tod ;

	uid_t	uid ;
	pid_t	pid ;

	uint	rv ;
	uint	v ;

	int	rs = SR_OK ;
	int	rs1 ;


	if (! lip->init.rnum) {

	lip->init.rnum = TRUE ;
	rv = 0 ;

/* these are the same on a given triplet of node-user-process */

	v = gethostid() ;
	rv ^= v ;

	uid = getuid() ;
	v = uid ;
	rv += v ;

	pid = getpid() ;
	v = pid ;
	rv += v ;

	pid = getppid() ;
	v = pid ;
	rv += v ;

	lip->rnum = (rv & INT_MAX) ;

	} /* end if */

/* these do shake things up a bit */

	rv = lip->rnum ;
	rs1 = getnprocs(pip) ;

	if (rs1 >= 0) {
	    rv += lip->nprocs ;
	    rv += lip->ncpus ;
	    rv += lip->btime ;
	}

/* these are somewhat cyclical at the low end */

	rs1 = uc_gettimeofday(&tod,NULL) ;

	if (rs1 >= 0) {
	    rv ^= tod.tv_sec ;
	    rv += tod.tv_usec ;
	} else {
	    rv ^= pip->daytime ;
	}

#if	defined(SYSHAS_HRTIME) && (SYSHAS_HRTIME > 0)
	{
	    hrtime_t	hrt = gethrtime() ;
	    rv ^= hrt ;
	}
#endif /* SYSHAS_HRTIME */

	lip->rnum = (rv & INT_MAX) ;

ret0:
	return (rs >= 0) ? lip->rnum : rs ;
}
/* end subroutine (getrnum) */


static int getmem(pip)
struct proginfo	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	long	ppm ;
	long	lw ;

	int	rs = SR_OK ;
	int	rs1 ;


	if ((lip->init.mem || ((pip->daytime - lip->ti_mem) < lip->to)) )
	    goto ret0 ;

	lip->init.mem = TRUE ;
	lip->ti_mem = pip->daytime ;
	if (lip->pagesize == 0) lip->pagesize = getpagesize() ;

	ppm = (1024 * 1024) / lip->pagesize ;

/* OK, now calculate the megabytes of each type of memory */

#if	defined(_SC_PHYS_PAGES)

	rs1 = uc_sysconf(_SC_PHYS_PAGES,&lw) ;
	lip->pmt = 1 ;
	if ((rs1 >= 0) && (ppm > 0))
	    lip->pmt = (lw / ppm) ;

#endif /* defined(_SC_PHYS_PAGES) */

#if	defined(_SC_AVPHYS_PAGES)
	rs1 = uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;
	lip->pma = 1 ;
	if ((rs1 >= 0) && (ppm > 0))
	    lip->pma = (lw / ppm) ;

	if (lip->pmt > 0) {

	    long	n100 = ((lip->pmt - lip->pma) * 100) ;

	    lip->pmu = (n100 / lip->pmt) ;
	}

#endif /* defined(_SC_AVPHYS_PAGES) */

ret0:
	return rs ;
}
/* end subroutine (getmem) */


/* find the number of processes on the system */
static int loadkinfo(smp,daytime)
struct sysmisc	*smp ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	n = 0 ;


	if (smp == NULL)
	    return SR_FAULT ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	memset(smp,0,sizeof(struct sysmisc)) ;

#if	CF_GETSYSMISC
	{
	    GETSYSMISC	misc ;
	    if ((rs = getsysmisc(&misc,daytime)) >= 0) {
	        smp->btime = misc.btime ;
	        smp->nprocs = misc.nproc ;
	        smp->ncpus = misc.ncpu ;
	        n = misc.nproc ;
	    } /* end if (getsysmisc) */
	} /* end block */
#else /* CF_GETSYSMISC */
	{
	    KINFO	ki ;
	    KINFO_DATA	kd ;

	    if ((rs = kinfo_open(&ki,daytime)) >= 0) {

	        rs = kinfo_sysmisc(&ki,daytime,&kd) ;

	        if (rs >= 0) {
	            smp->btime = kd.boottime ;
	            smp->nprocs = kd.nproc ;
	            smp->ncpus = kd.ncpu ;
	            n = kd.nproc ;
	        }

	        kinfo_close(&ki) ;
	    } /* end if (opened KINFO) */
	} /* end block (kinfo) */
#endif /* CF_GETSYSMISC */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (loadkinfo) */



