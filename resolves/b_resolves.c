/* b_resolves */

/* SHELL built-in for Message-of-the-Day */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_GNCACHE	1		/* use GID (group) cache */
#define	CF_ENVIRON	0		/* change environment on processing */
#define	CF_PROCID	1		/* call 'resolves_procid(3dam)' */
#define	CF_UGETPW	1		/* use 'ugetpw(3uc)' */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was written as a KSH builtin.  

	= 2011-09-23, David A­D­ Morano
	I put the "environment" hack into this code.  See the design
	note in the comments below.

	= 2011-11-08, David A­D­ Morano
	I took the "environment" hack *out* of the code.  It was actually
	not correct in a multithreaded environment.  Currently (at this
	present time) the KSH Shell is not multithreaded, but other
	programs (notably servers) that dynamically load "programs" 
	(commands) from a shared-object library containing these might
	someday (probably much sooner than KSH) become multithreaded.
	I needed to make environment handling multithreaded before
	this present command gets dynamically loaded and executed by
	some server or another.  The fix was to take environment handling
	out of this code and to put into the RESOLVES object code.  Also,
	a new LIBUC-level call had to be invented to handle the new
	case of opening a general file w/ a specified environment.  
	The LIBUC call to open programs specifically handled passing
	environment already but it was not general for opening any
	sort of "file."  The new LIBUC call ('uc_openenv(3uc)') is.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a built-in command to the KSH shell. It should also be able to
        be made into a stand-alone program without much (if almost any)
        difficulty, but I have not done that yet.

	Synopsis:

	$ motd [-u <username>] [-a <admin(s)>] [-d[=<intrun>] [-V]

	Design problems:

        I put a real hack into this code. The RESOLVES object was supposed to
        handle all aspects of the actual RESOLVES processing. But a new issue
        arose. People want any subprograms executed as a result of reading
        sub-RESOLVES files to know the client UID and GID (the only things that
        we know). We are currently doing this by placing these as special
        environment variables into our own process environment before executing
        'resolves_process()'. But switching out own actual environment in a way
        that does not leak memory (meaning do not use 'putenv(3c)') adds a
        little complication, which can be seen below. Somehow in the future we
        will try to move some kind of processing into the RESOLVES object
        itself.

	Updated note on design problems:

        The hack above to pass modified environment down to the RESOLVES object
        is no longer needed. The RESOLVES object itself now handles that. A new
        RESOLVES object method has been added to pass fuller specified
        identification down into the RESOLVES object. This new interface is
        'resolves_procid()'.


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
#include	<sys/wait.h>
#include	<sys/stat.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<sigman.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<lfm.h>
#include	<getutmpent.h>
#include	<fsdir.h>
#include	<ptm.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"resolves_config.h"
#include	"defs.h"
#include	"upt.h"
#include	"gncache.h"
#include	"resolves.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	BUFLEN
#define	BUFLEN		8196
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	COLS_USERNAME	8
#define	COLS_REALNAME	39

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif

#define	EXTRAENVS	6		/* possible extra variables */

#undef	TMPDMODE
#define	TMPDMODE	0777

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9

#define	PO_ADMIN	"admin"

#define	TO_TMPFILE	3600		/* temporary file time-out */
#define	TO_GID		(5*60)		/* group (GID) cache time-out */
#define	TO_CHECK	(10*60)		/* RESOLVES check time-out */

#define	TSFNAME		".lastmaint"

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getutmpterm(char *,int,pid_t) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	termwritable(const char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		un:1 ;
	uint		mnt:1 ;
	uint		pidfname:1 ;
	uint		pidlock:1 ;
	uint		fg:1 ;
} ;

struct locinfo {
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	open ;
	PROGINFO	*pip ;
	const char	**envv ;
	const char	*un ;
	const char	*pidfname ;
	const char	*mdname ;
	IDS		id ;
	PTM		envm ;
	VECSTR		tmpstr ;
	pid_t		sid ;
	pid_t		pid ;
	uid_t		uid, euid ;
	uid_t		uid_prog ;
	uid_t		uid_motd ;
	gid_t		gid, egid ;
	gid_t		gid_prog ;
	gid_t		gid_motd ;
	int		to_cache ;
	int		envc ;
	char		username[USERNAMELEN + 1] ;
	char		groupname[GROUPNAMELEN + 1] ;
	char		termfname[MAXPATHLEN + 1] ;
} ;

struct dargs {
	const char	*tmpdname ;
} ;

struct client {
	const char	**avp ;		/* admins-vector-pointer */
	const char	*gn ;
	const char	*un ;
	uid_t		uid ;
	gid_t		gid ;
	int		fd ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	locinfo_start(struct locinfo *,PROGINFO *) ;
static int	locinfo_finish(struct locinfo *) ;
static int	locinfo_mkenvv(struct locinfo *) ;
#if	CF_ENVIRON
static int	locinfo_process(struct locinfo *,RESOLVES *,struct client *) ;
static int	locinfo_addenvdig(struct locinfo *,int,const char *,int) ;
static int	locinfo_addenvstr(struct locinfo *,int,const char *,
			const char *,int) ;
#endif /* CF_ENVIRON */
static int	locinfo_loadids(struct locinfo *) ;
static int	locinfo_mdname(struct locinfo *) ;
static int	locinfo_tmpmaint(struct locinfo *) ;
static int	locinfo_getgid(struct locinfo *) ;
static int	locinfo_chgrp(struct locinfo *,const char *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procregular(PROGINFO *,PARAMOPT *,const char *) ;
static int	procdaemon(PROGINFO *,PARAMOPT *,const char *) ;
static int	procregout(PROGINFO *,PARAMOPT *,SHIO *) ;
static int	procregouter(PROGINFO *,const char **,SHIO *) ;
static int	procmotd(PROGINFO *,const char *,const char **,int) ;
static int	procextras(PROGINFO *) ;
static int	procpidfile(PROGINFO *) ;
static int	proclockacquire(PROGINFO *,LFM *,int) ;
static int	proclockrelease(PROGINFO *,LFM *) ;
static int	proclockcheck(PROGINFO *,LFM *) ;
static int	proclockprint(PROGINFO *,LFM_CHECK *) ;
static int	procdown(PROGINFO *,LFM *,const char *) ;
static int	procserve(PROGINFO *,LFM *,const char *) ;
static int	prochandle(PROGINFO *,GNCACHE *,RESOLVES *,
			uid_t,gid_t,int) ;

static int	vecstr_loadadmins(vecstr *,PARAMOPT *) ;

static int	deleter(void *) ;
static int	deleter_all(struct dargs *) ;

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
	"H",
	"HELP",
	"LOGFILE",
	"sn",
	"af",
	"of",
	"ef",
	"mnt",
	"pid",
	"fg",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_hh,
	argopt_help,
	argopt_logfile,
	argopt_sn,
	argopt_af,
	argopt_of,
	argopt_ef,
	argopt_mnt,
	argopt_pid,
	argopt_fg,
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
	{ SR_BUSY, EX_TEMPFAIL },
	{ SR_EXIT, EX_TERM },
	{ SR_INTR, EX_INTR },
	{ 0, 0 }
} ;

static const char *akonames[] = {
	"quiet",
	"runint",
	NULL
} ;

enum akonames {
	akoname_quiet,
	akoname_intrun,
	akoname_overlast
} ;

#if	CF_ENVIRON
static const char *strvar_motdun = "RESOLVES_USERNAME" ;
static const char *strvar_motdgn = "RESOLVES_GROUPNAME" ;
static const char *strvar_motduid = "RESOLVES_UID" ;
static const char *strvar_motdgid = "RESOLVES_GID" ;
#endif /* CF_ENVIRON */

static const char *badenvs[] = {
	"_",
	"TMOUT",
	NULL
} ;


/* exported subroutines */


int b_resolves(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	PROGINFO	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;

	SIGMAN		sm ;

	PARAMOPT	aparams ;

	SHIO		errfile ;

	KEYOPT		akopts ;

	uint	mo_start = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	n, i, j ;
	int	size, v ;
	int	cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_child = FALSE ;
	int	f ;

	const char	*po_admin = PO_ADMIN ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*mntfname = NULL ;
	const char	*cp ;


	if (contextp != NULL) lib_initenviron() ;

	if_exit = 0 ;
	if_int = 0 ;

	rs = sigman_start(&sm, sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("b_resolves: starting\n") ;
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
	pip->intrun = 0 ;
	pip->intpoll = -1 ;
	pip->intmark = -1 ;
	pip->intlock = -1 ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

/* do we have a keyword match or should we assume only key letters? */

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
	                            rs = cfdeci(avp,avl,&v) ;
	                            pip->verboselevel = v ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    }
	                    break ;

	                case argopt_pid:
	                    lip->have.pidfname = TRUE ;
			    lip->final.pidfname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->pidfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->pidfname = argp ;
	                    }
	                    break ;

	                case argopt_mnt:
	                    lip->have.mnt = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            mntfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            mntfname = argp ;
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

/* output name */
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

	                case argopt_fg:
	                    lip->final.fg = TRUE ;
	                    lip->have.fg = TRUE ;
	                    lip->f.fg = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                    	    lip->f.fg = (rs > 0) ;
				}
	                    }
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

			    case 'P':
	                        lip->have.pidfname = TRUE ;
				lip->final.pidfname = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                lip->pidfname = avp ;
	                        } else {
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->pidfname = argp ;
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->have.quiet = TRUE ;
				pip->final.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					pip->f.quiet = (rs > 0) ;
				    }
	                        }
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

/* print header */
	                    case 'a':
	                        pip->have.aparams = TRUE ;
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = paramopt_loads(&aparams,
	                                po_admin,argp,argl) ;
	                        break ;

	                    case 'd':
	                        pip->have.daemon = TRUE ;
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                pip->final.intrun = TRUE ;
	                                pip->have.intrun = TRUE ;
					pip->intrun = -1 ;
					if (avp[0] != '-')
	                                    rs = cfdecti(avp,avl,&pip->intrun) ;
	                            }
	                        }
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

/* target username */
	                    case 'u':
	                        lip->have.un = TRUE ;
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lip->un = argp ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_resolves: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,
	        "%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
		int	f_sfio = FALSE ;
		int	f_builtin = FALSE ;
#if	CF_SFIO
		f_sfio = TRUE ;
#endif
#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
		f_builtin = TRUE ;
#endif
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    shio_printf(pip->efp,"%s: f_sfio=%u f_builtin=%u\n",
	        pip->progname,f_sfio,f_builtin) ;
	}

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

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* argument defaults */

	if (lip->un == NULL) lip->un = getenv(VARTARUSER) ;

	if (argval != NULL) {
	    rs = cfdeci(argval,-1,&argvalue) ;
	    if (rs < 0) {
		ex = EX_USAGE ;
		goto retearly ;
	    }
	}

	if (pip->intrun < 0)
	    pip->intrun = TO_RUN ;

	if (pip->intpoll < 0)
	    pip->intpoll = TO_POLL ;

	if (pip->intmark < 0)
	    pip->intmark = TO_MARK ;

	if (pip->intlock < 0)
	    pip->intlock = TO_LOCK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_resolves: intrun=%d\n",pip->intrun) ;
#endif

/* other initilization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* other */

	if (pip->f.daemon)
	    rs = ids_load(&pip->id) ;

	if (rs < 0)
	    goto badusername ;

/* gather invocation login names */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    if (cp[0] != '\0')
	        rs = paramopt_loads(&aparams,po_admin,cp,-1) ;

	    if (rs < 0)
	        break ;

	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int	len ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            cl = len ;
	            if ((cp[0] == '\0') || (cp[0] == '#')) continue ;

	            pan += rs ;
	            rs = paramopt_loads(&aparams,po_admin,cp,cl) ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        shio_close(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: argument file inaccessible (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if (rs < 0)
	    goto badloadname ;

	if (mntfname == NULL)
	    mntfname = getenv(VARMNTFNAME) ;

	if (mntfname != NULL)
	    pip->f.daemon = TRUE ;

	if ((mntfname == NULL) || (mntfname[0] == '\0'))
	    mntfname = MNTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves: sizeof(RESOLVES)=%u\n",sizeof(RESOLVES)) ;
#endif

	if (pip->f.daemon) {

	    if (pip->debuglevel > 0)
		shio_printf(pip->efp,"%s: f_fg=%u\n",
		pip->progname,lip->f.fg) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_resolves: f_fg=%u\n",lip->f.fg) ;
#endif

	    rs = procdaemon(pip,&aparams,mntfname) ;
	    f_child = (rs > 0) ;

	} else
	    rs = procregular(pip,&aparams,ofname) ;

badoutopen:
badcacheadd:
badloadname:
	if (pip->f.daemon)
	    ids_release(&pip->id) ;

badusername:
done:
	if ((rs < 0) && (ex != EX_OK) && (! pip->f.quiet)) {
	    shio_printf(pip->efp,
	        "%s: could not perform function (%d)\n",
	        pip->progname,rs) ;
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_BUSY:
	        ex = EX_TEMPFAIL ;
	        break ;

	    default:
		ex = mapex(mapexs,rs) ;
	        if (! pip->f.quiet)
	            shio_printf(pip->efp,
	                "%s: could not perform function (%d)\n",
	                pip->progname,rs) ;
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
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

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
	    debugprintf("b_resolves: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

ret0:
	return ex ;
}
/* end subroutine (b_resolves) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{

	switch (sn) {

	case SIGINT:
	    if_int = TRUE ;
	    break ;

	default:
	    if_exit = TRUE ;
	    break ;

	} /* end switch */

}
/* end subroutine (sighand_int) */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [-u <username>] [-a <admin(s)>] [<admin(s)>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-d[=<intrun>]] [-mnt <mntfile>] [-pid <pidfile>]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
PROGINFO	*pip ;
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;
	lip->gid_prog = -1 ;
	lip->gid_motd = -1 ;
	lip->gid = getgid() ;
	lip->egid = getegid() ;
	lip->uid_prog = -1 ;
	lip->uid_motd = -1 ;
	lip->uid = getuid() ;
	lip->euid = geteuid() ;

	if ((rs = ptm_create(&lip->envm,NULL)) >= 0) {
	    if ((rs = locinfo_mkenvv(lip)) >= 0) {
		rs = vecstr_start(&lip->tmpstr,2,0) ;
		if (rs < 0)
		    if (lip->mdname != NULL) {
	    	    uc_free(lip->mdname) ;
	    	    lip->mdname = NULL ;
		}
	    }
	    if (rs < 0)
	    ptm_destroy(&lip->envm) ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = locinfo_tmpmaint(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->mdname != NULL) {
	    rs1 = uc_free(lip->mdname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->mdname = NULL ;
	}

	rs1 = vecstr_finish(&lip->tmpstr) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->envv != NULL) {
	    rs1 = uc_free(lip->envv) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->envv = NULL ;
	}

	rs1 = ptm_destroy(&lip->envm) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_mkenvv(lip)
struct locinfo	*lip ;
{
	PROGINFO	*pip ;

	int	rs = SR_OK ;
	int	i ;
	int	envl = 0 ;
	int	envc = 0 ;
	int	size ;

	const char	**envp ;
	const char	**envv ;
	const char	*ep ;

	char	*p ;


	pip = lip->pip ;
	envp = pip->envv ;
	for (i = 0 ; envp[i] != NULL ; i += 1) envl += 1 ;

	size = (envl + 1 + EXTRAENVS) * sizeof(char *) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0)
	    goto ret0 ;

	lip->envv = (const char **) p ;
	envv = (const char **) p ;
	for (i = 0 ; envp[i] != NULL ; i += 1) {
	    ep = envp[i] ;
	    if ((matstr(badenvs,ep,-1) < 0) && (strncmp(ep,"RESOLVES_",5) != 0)) {
		envv[envc++] = ep ;
	    }
	}

	lip->envc = envc ;
	envv[envc] = NULL ;

ret0:
	return rs ;
}
/* end subroutine (locinfo_mkenvv) */


#if	CF_ENVIRON

static int locinfo_process(lip,mp,cip)
struct locinfo	*lip ;
RESOLVES		*mp ;
struct client	*cip ;
{
	int	rs = SR_OK ;
	int	iw ;
	int	i ;
	int	envc = lip->envc ;
	int	wlen = 0 ;


/* IMPORTANT NOTE: only add extra environment variables up to EXTRAENVS! */

	i = 0 ;

	iw = cip->uid ;
	if (rs >= 0) {
	    if (i++ < EXTRAENVS) {
	        rs = locinfo_addenvdig(lip,envc,strvar_motduid,iw) ;
	        envc = rs ;
	    } else rs = SR_NOANODE ;
	}

	iw = cip->gid ;
	if (rs >= 0) {
	    if (i++ < EXTRAENVS) {
	        rs = locinfo_addenvdig(lip,envc,strvar_motdgid,iw) ;
	        envc = rs ;
	    } else rs = SR_NOANODE ;
	}

	if ((rs >= 0) && (lip->un != NULL)) {
	    if (i++ < EXTRAENVS) {
		const char	*vp = lip->un ;
	        rs = locinfo_addenvstr(lip,envc,strvar_motdun,vp,-1) ;
	        envc = rs ;
	    } else rs = SR_NOANODE ;
	}

	if ((rs >= 0) && (lip->groupname[0] != '\0')) {
	    if (i++ < EXTRAENVS) {
		const char	*vp = lip->groupname ;
	        rs = locinfo_addenvstr(lip,envc,strvar_motdgn,vp,-1) ;
	        envc = rs ;
	    } else rs = SR_NOANODE ;
	}

/* IMPORTANT NOTE: only add extra environment variables up to EXTRAENVS! */

	lip->envv[envc] = NULL ;

/* enter mutual-exclusion region */

	if ((rs >= 0) && ((rs = ptm_lock(&lip->envm)) >= 0)) {
	    char	**oenvv = environ ; /* save old */

	    environ = (char **) lip->envv ;

	    if (rs >= 0) {
	        rs = resolves_process(mp,cip->gn,cip->avp,cip->fd) ;
	        wlen = rs ;
	    }

	    environ = oenvv ; /* restore old */
	    ptm_unlock(&lip->envm) ;
	} /* end if (mutual-exclusion region) */

/* exited mutual-exclusion region */

	vecstr_delall(&lip->tmpstr) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_process) */


static int locinfo_addenvdig(lip,envc,s,iw)
struct locinfo	*lip ;
int		envc ;
const char	*s ;
int		iw ;
{
	int	rs = SR_OK ;
	int	diglen ;

	char	digbuf[DIGBUFLEN + 1] ;


	if (rs >= 0) {
	    rs = ctdeci(digbuf,DIGBUFLEN,iw) ;
	    diglen = rs ;
	}

	if (rs >= 0) {
	    rs = locinfo_addenvstr(lip,envc,s,digbuf,diglen) ;
	    envc = rs ;
	}

	return (rs >= 0) ? envc : rs ;
}
/* end subroutine (locinfo_addenvdig) */


static int locinfo_addenvstr(lip,envc,kn,vp,vl)
struct locinfo	*lip ;
int		envc ;
const char	*kn ;
const char	*vp ;
int		vl ;
{
	int	rs ;
	int	i ;

	char	*p ;


	    rs = vecstr_envset(&lip->tmpstr,kn,vp,vl) ;
	    i = rs ;
	    if (rs >= 0) {
		rs = vecstr_get(&lip->tmpstr,i,&p) ;
		lip->envv[envc++] = p ;
	    }

	return (rs >= 0) ? envc : rs ;
}
/* end subroutine (locinfo_addenvstr) */

#endif /* CF_ENVIRON */


static int locinfo_mdname(lip)
struct locinfo	*lip ;
{
	PROGINFO	*pip = lip->pip ;

	struct ustat	usb ;

	mode_t	dmode = TMPDMODE ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_created = FALSE ;

	const char	*mdn = NULL ;

	char	mdname[MAXPATHLEN + 1] ;


	if (lip->mdname != NULL)
	    goto ret0 ;

	rs = proginfo_rootname(pip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = mkpath3(mdname,pip->tmpdname,pip->rootname,MDNAME) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(mdname,&usb) ;

	if (rs1 >= 0) {

	    mdn = (S_ISDIR(usb.st_mode)) ? mdname : pip->tmpdname ;

	} else {

	    f_created = TRUE ;
	    mdn = mdname ;
	    if ((rs = mkdirs(mdname,dmode)) >= 0) {
	        if ((rs = u_chmod(mdname,dmode)) >= 0)
		    locinfo_chgrp(lip,mdname) ;
	    }

	} /* end if */

	if ((rs >= 0) && (mdn != NULL)) {
	    const char	*cp ;
	    rs = uc_mallocstrw(mdn,-1,&cp) ;
	    if (rs >= 0) {
		lip->mdname = cp ;
	    } else
	        lip->mdname = NULL ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/locinfo_mdname: ret rs=%d f_created=%u\n",
	        rs,f_created) ;
#endif

	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (locinfo_mdname) */


static int locinfo_loadids(lip)
struct locinfo	*lip ;
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f_other = FALSE ;
	const char	*un = lip->un ;

	if (lip->groupname[0] == '\0') {

	    if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
		const int	unlen = USERNAMELEN ;
		rs = getusername(lip->username,unlen,lip->uid) ;
	    	lip->un = lip->username ;
	    	lip->uid_motd = lip->uid ;
	    	lip->gid_motd = lip->gid ;
	    } else {
	        struct passwd	pw ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    	    rs = GETPW_NAME(&pw,pwbuf,pwlen,lip->un) ;
	    	    lip->uid_motd = pw.pw_uid ;
	    	    lip->gid_motd = pw.pw_gid ;
	    	    uc_free(pwbuf) ;
		} /* end if (memory-allocation) */
	    } /* end if (alternatives) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/locinfo_loadids: tar gid=%d\n",
		lip->gid_motd) ;
#endif

	    if (rs >= 0) {
	        const int	gnlen = GROUPNAMELEN ;
	        rs = getgroupname(lip->groupname,gnlen,lip->gid_motd) ;
	    }

	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/locinfo_loadids: ret rs=%d gn=%s\n",
	        rs,lip->groupname) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_loadids) */


static int locinfo_tmpmaint(lip)
struct locinfo	*lip ;
{
	struct ustat	usb ;

	struct dargs	da ;

	time_t	daytime = time(NULL) ;

	const int	to = TO_TMPFILE ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_needed = FALSE ;

	char	tsfname[MAXPATHLEN+1] ;


	if (lip->mdname == NULL)
	    goto ret0 ;

/* get out if no possible need */

	rs = mkpath2(tsfname,lip->mdname,TSFNAME) ;
	if (rs < 0) goto ret0 ;

	{
	    int	fd ;
	    rs1 = u_open(tsfname,O_WRONLY|O_CREAT,0664) ;
	    fd = rs1 ;
	    if (rs1 >= 0) {
	        rs1 = u_fstat(fd,&usb) ;
		if (rs1 >= 0) {
		    if ((daytime - usb.st_mtime) >= to) rs1 = SR_NOENT ;
		    if (rs1 < 0) {
		        int		tl ;
		        char	timebuf[TIMEBUFLEN + 3] ;
		        timestr_log(daytime,timebuf) ;
		        tl = strlen(timebuf) ;
		        timebuf[tl++] = '\n' ;
		        rs = u_write(fd,timebuf,tl) ;
		    }
		}
		u_close(fd) ;
	    }
	}
	if (rs1 >= 0) goto ret0 ;
	if (rs < 0) goto ret0 ;

	f_needed = TRUE ;

/* continue */

	{
	    memset(&da,0,sizeof(struct dargs)) ;
	    da.tmpdname = lip->mdname ;
	}

	rs = uc_fork() ;

	if (rs == 0) {
	    int	ex ;
	    int	i ;


#if	CF_DEBUGS
#else
	    for (i = 0 ; i < NOFILE ; i += 1)
		u_close(i) ;
#endif

	    u_setsid() ;

	    uc_sigignore(SIGHUP) ;

	    rs = deleter(&da) ;

	    ex = (rs >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex) ;

	} /* end if (child) */

ret0:
	return (rs >= 0) ? f_needed : rs ;
}
/* end subroutine (locinfo_tmpmaint) */


static int locinfo_getgid(lip)
struct locinfo	*lip ;
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_got = FALSE ;

	if (lip->gid_prog < 0) {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {

	    if (lip->euid == lip->uid) { /* like we are a SHELL builtin */

	    if ((rs = proginfo_rootname(pip)) >= 0) {
		rs1 = GETPW_NAME(&pw,pwbuf,pwlen,pip->rootname) ;
		if (rs1 == SR_NOTFOUND) {
			char	username[USERNAMELEN + 1] ;
			strwcpylc(username,VARPRNAME,USERNAMELEN) ;
			rs1 = GETPW_NAME(&pw,pwbuf,pwlen,username) ;
		}
		f_got = (rs1 >= 0) ;
		lip->uid_prog = pw.pw_uid ;
		lip->gid_prog = pw.pw_gid ;
	    }

	}

	if ((rs >= 0) && (! f_got)) {
	    rs = GETPW_UID(&pw,pwbuf,pwlen,lip->euid) ; /* cannot fail? */
	    lip->uid_prog = pw.pw_uid ;
	    lip->gid_prog = pw.pw_gid ;
	}

		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_getgid) */


static int locinfo_chgrp(lip,fname)
struct locinfo	*lip ;
const char	fname[] ;
{
	struct ustat	usb ;

	uid_t	cuid = -1 ;

	int	rs = SR_OK ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	rs = locinfo_getgid(lip) ;
	if (rs < 0)
	    goto ret0 ;

	if (lip->euid != lip->uid_prog)
	    cuid = lip->uid_prog ;

	rs = u_stat(fname,&usb) ;
	if ((rs >= 0) && (usb.st_gid != lip->gid_prog))
	    u_chown(fname,cuid,lip->gid_prog) ; /* could be "restricted" */

ret0:
	return rs ;
}
/* end subroutine (locinfo_chgrp) */


/* process the program ako-options */
static int procopts(pip,kop)
PROGINFO	*pip ;
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
	        uint	uv ;

	        switch (oi) {

	        case akoname_quiet:
	            if (! pip->final.quiet) {
	                pip->have.quiet = TRUE ;
	                pip->final.quiet = TRUE ;
	                pip->f.quiet = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    pip->f.quiet = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_intrun:
	            if (! pip->final.intrun) {
	                pip->have.intrun = TRUE ;
	                pip->final.intrun = TRUE ;
	                pip->f.intrun = TRUE ;
	                if (vl > 0) {
			    rs = cfdecui(vp,vl,&uv) ;
	                    pip->intrun = uv ;
			}
	            }
	            break ;

	        } /* end switch */

	        c += 1 ;

	        } /* end if (valid option) */

		if (rs < 0) break ;

	    } /* end while (looping through key options) */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (keyopts) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procregular(pip,app,ofname)
PROGINFO	*pip ;
PARAMOPT	*app ;
const char	ofname[] ;
{
	struct locinfo	*lip = pip->lip ;

	SHIO	outfile, *ofp = &outfile ;

	int	rs = SR_OK ;
	int	wlen = 0 ;


	rs = locinfo_loadids(lip) ;
	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregular: un=%s\n",lip->un) ;
#endif

	if ((pip->debuglevel > 0) && (lip->groupname != NULL))
	    shio_printf(pip->efp,"%s: group=%s\n",
		pip->progname,lip->groupname) ;

/* open output file */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {

	    rs = procregout(pip,app,ofp) ;
	    wlen += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregular: procregout() rs=%d\n",rs) ;
#endif

	    shio_close(ofp) ;
	} else {
	    shio_printf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;
	}

ret0:
	return (rs >= 0) ? wlen : rs ;

bad0:
	goto ret0 ;
}
/* end subroutine (procregular) */


static int procdaemon(pip,app,mntfname)
PROGINFO	*pip ;
PARAMOPT	*app ;
const char	mntfname[] ;
{
	struct locinfo	*lip = pip->lip ;

	struct ustat	usb ;

	LFM	pidlock, *plp = &pidlock ;

	pid_t	pid = 0 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_child = FALSE ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procdaemon: entered\n") ;
#endif

	pip->daytime = time(NULL) ;

	if (pip->debuglevel > 0) {
	    const char	*fmt ;

	    shio_printf(pip->efp,"%s: mntfile=%s\n",
	        pip->progname,mntfname) ;

	    fmt = "%s: runint=(inf)\n" ;
	    if (pip->intrun >= 0) fmt = "%s: runint=%u\n",
	    shio_printf(pip->efp,fmt,
	        pip->progname,pip->intrun) ;

	} /* end if */

	if (rs >= 0)
	    rs = procextras(pip) ;

	rs = procpidfile(pip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = proclockacquire(pip,plp,TRUE) ;

	if (rs >= 0)
	    proclockrelease(pip,plp) ;

	if (rs < 0) {
	    if (! pip->f.quiet)
	        shio_printf(pip->efp,"%s: could not acquire PID lock (%d)\n",
	            pip->progname,rs) ;
	    goto ret0 ;
	}

/* check the mount point */

	rs = u_stat(mntfname,&usb) ;
	if ((rs >= 0) && (! S_ISREG(usb.st_mode)))
	    rs = SR_BUSY ;

	if (rs >= 0)
	    rs = sperm(&pip->id,&usb,W_OK) ;

	if (rs < 0) {
	    if (! pip->f.quiet)
	        shio_printf(pip->efp,"%s: inaccessible mount point (%d)\n",
	            pip->progname,rs) ;
	    goto ret0 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procdaemon: spawn\n") ;
#endif

/* spawn child */

	if (! lip->f.fg) {
	    shio_flush(pip->efp) ;
	    rs = uc_fork() ;
	    pid = rs ;
	    f_child = (pid == 0) ;
	    if (rs < 0) goto ret0 ;
	}

	if (pid == 0) {
	    int	ex ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	    uint	mall_start ;
#endif

	    if (! lip->f.fg) {

		if ((pip->efp != NULL) && pip->open.errfile) {
		    pip->open.errfile = FALSE ;
		    shio_close(pip->efp) ;
	    	    memset(pip->efp,0,sizeof(SHIO)) ;
		}

	    lip->pid = getpid() ;

	    lip->sid = getsid(0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_resolves/procdaemon: child sid=%u pid=%u\n",
	            lip->sid,lip->pid) ;
#endif

	    rs1 = getutmpterm(lip->termfname,MAXPATHLEN,lip->sid) ;
	    if (rs1 < 0)
	        mkpath1(lip->termfname,DEVTTY) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_resolves/procdaemon: termfname=%s\n",
	            lip->termfname) ;
#endif

#if	CF_DEBUGS || CF_DEBUG
#else
	    {
	        int	i ;
	        for (i = 0 ; i < NOFILE ; i += 1) {
	            if (i != 2) u_close(i) ;
	        }
	    } /* end block */
#endif /* CF_DEBUG */

	    uc_sigignore(SIGHUP) ;

	    if ((! lip->f.fg) && (lip->pid != lip->sid)) u_setsid() ;

	    if (lip->termfname[0] != '\0') {
		const char	*tf = lip->termfname ;
	        rs1 = shio_open(pip->efp,tf,"w",0666) ;
		if ((rs1 == SR_ACCESS) && (lip->uid != lip->euid)) {
		    rs1 = perm(tf,lip->euid,lip->egid,NULL,X_OK) ;
		    if (rs1 >= 0) {
		        u_setreuid(-1,lip->uid) ;
	                rs1 = shio_open(pip->efp,tf,"w",0666) ;
		        u_setreuid(-1,lip->euid) ;
		    }
		}
		pip->open.errfile = (rs1 >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_resolves/procdaemon: shio_open() rs=%d\n",
	                rs1) ;
#endif /* CF_DEBUG */

	    } /* end if (opening controlling terminal) */

	    if ((lip->termfname[0] == '\0') || (rs1 < 0))
	        pip->efp = NULL ;

/* after the last 'open', we no longer need our real UID */

	    if (lip->uid != lip->euid)
	        u_setreuid(lip->euid,-1) ;

	    } /* end if (daemon adjustment) */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	    uc_mallout(&mall_start) ;
#endif

	    if (rs >= 0) {
	        if ((rs = proclockacquire(pip,plp,FALSE)) >= 0) {

		    rs = procdown(pip,plp,mntfname) ;

	            proclockrelease(pip,plp) ;
	 	}
	    } /* end if */

	    if ((pip->debuglevel > 0) && (pip->efp != NULL))
	        shio_printf(pip->efp,"%s: daemon exiting (%d)\n",
	            pip->progname,rs) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mall_end, mo ;
	    uc_mallout(&mall_end) ;
	    mo = (mall_end - mall_start) ;
	    debugprintf("b_resolves: daemon final mallout=%u\n",mo) ;
	}
#endif /* CF_DEBUGMALL */

	    if (pip->efp != NULL) {
		pip->open.errfile = FALSE ;
	        shio_close(pip->efp) ;
	    }

	    if (f_child) {
	        ex = (rs >= 0) ? EX_OK : EX_DATAERR ;
	        uc_exit(ex) ;
	    }

	} /* end if (child) */

	if ((rs >= 0) && (pip->debuglevel > 0) && (pid > 0))
	    shio_printf(pip->efp,"%s: daemon pid=%u\n",
	        pip->progname,pid) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procdaemon: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdaemon) */


static int proclockacquire(pip,plp,f)
PROGINFO	*pip ;
LFM		*plp ;
int		f ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	cl ;

	const char	*ccp ;
	const char	*cp ;

	char	tmpfname[MAXPATHLEN + 1] ;


	ccp = lip->pidfname ;
	if ((ccp != NULL) && (ccp[0] != '\0') && (ccp[0] != '-')) {

	    struct ustat	usb ;

	    LFM_CHECK	lc ;

	    if (f) {

	        cl = sfdirname(lip->pidfname,-1,&cp) ;

	        rs = mkpath1w(tmpfname,cp,cl) ;

	        if ((rs >= 0) && (u_stat(tmpfname,&usb) == SR_NOENT)) {
	            mkdirs(tmpfname,TMPDMODE) ;
		    locinfo_chgrp(lip,tmpfname) ;
		}

	    } /* end if (checking PID directory) */

	    if (rs >= 0) {

	        rs = lfm_start(plp,lip->pidfname,
	            LFM_TRECORD, pip->intlock,&lc,
	            pip->nodename,pip->username,pip->banner) ;

	        lip->open.pidlock = (rs >= 0) ;
	        if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN))
	            proclockprint(pip,&lc) ;

	    } /* end if */

	} /* end if (pidlock) */

	return rs ;
}
/* end subroutine (proclockacquire) */


static int proclockrelease(pip,plp)
PROGINFO	*pip ;
LFM		*plp ;
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.pidlock) {
	    lip->open.pidlock = FALSE ;
	    rs1 = lfm_finish(plp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (proclockrelease) */


static int procdown(pip,plp,mntfname)
PROGINFO	*pip ;
LFM		*plp ;
const char	mntfname[] ;
{
	int	rs = SR_OK ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procdown: mntfile=%s\n",mntfname) ;
#endif

	rs = procserve(pip,plp,mntfname) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procdown: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdown) */


static int procserve(pip,plp,mntfname)
PROGINFO	*pip ;
LFM		*plp ;
const char	mntfname[] ;
{
	struct locinfo	*lip = pip->lip ;

	struct pollfd	fds[2] ;

	GNCACHE	g ;

	RESOLVES	m ;

	time_t	ti_lock = pip->daytime ;
	time_t	ti_gncache = pip->daytime ;
	time_t	ti_run = pip->daytime ;
	time_t	ti_wait = pip->daytime ;
	time_t	ti_check = pip->daytime ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	to = pip->intpoll ;
	int	to_gid = TO_GID ;
	int	pto ;
	int	i ;
	int	cfd, sfd, pfd ;
	int	pipes[2] ;
	int	nhandle = 0 ;
	int	f ;


	if (mntfname[0] == '\0')
	    return SR_INVALID ;

	rs = u_pipe(pipes) ;
	sfd = pipes[0] ;		/* server-side */
	cfd = pipes[1] ;		/* client-side */
	if (rs < 0)
	    goto ret0 ;

	rs = u_ioctl(cfd,I_PUSH,"connld") ;
	if (rs < 0)
	    goto ret1 ;

/* attach the client end to the file created above */

	rs = uc_fattach(cfd,mntfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procserve: uc_fattach() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    if ((! pip->f.quiet) && (pip->efp != NULL))
	        shio_printf(pip->efp,"%s: could not perform mount (%d)\n",
	            pip->progname,rs) ;
	    goto ret2 ;
	}

	u_close(cfd) ;
	cfd = -1 ;

	uc_closeonexec(sfd,TRUE) ;

	rs = gncache_start(&g,211,to_gid) ;
	if (rs < 0)
	    goto ret3 ;

/* open the RESOLVES object */

	rs = resolves_open(&m,pip->pr) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procserve: resolves_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret4 ;

/* ready */

	i = 0 ;
	fds[i].fd = sfd ;
	fds[i].events = (POLLIN | POLLPRI) ;
	i += 1 ;
	fds[i].fd = -1 ;
	fds[i].events = 0 ;

	pto = (to * POLLMULT) ;
	while (rs >= 0) {

	    rs = u_poll(fds,1,pto) ;
	    pip->daytime = time(NULL) ;

	    if (rs > 0) {
	        int	re = fds[0].revents ;

	        if ((re & POLLIN) || (re & POLLPRI)) {
	            struct strrecvfd	passer ;
		    uid_t	uid ;
	            gid_t	gid ;

	            rs = acceptpass(sfd,&passer,-1) ;
	            pfd = rs ;
	            if (rs >= 0) {
			nhandle += 1 ;
			uid = passer.uid ;
	                gid = passer.gid ;
	                rs = prochandle(pip,&g,&m,uid,gid,pfd) ;
	                u_close(pfd) ;
	            } /* end if */

	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } /* end if (poll returned) */

	    } else if (rs == SR_INTR)
	        rs = SR_OK ;

	    if ((rs >= 0) && if_exit) {
		rs = SR_EXIT ;
	        break ;
	    }

	    f = ((pip->daytime - ti_wait) > (to*4)) ;
	    if (nhandle || f) {
		ti_wait = pip->daytime ;
		rs1 = SR_OK ;
		while (((nhandle > 0) || f) &&
		    ((rs1 = u_waitpid(-1,NULL,WNOHANG)) > 0)) {
		        if (nhandle > 0) nhandle -= 1 ;
		}
		if ((rs1 == SR_CHILD) && f && (nhandle > 0))
		        nhandle -= 1 ;
	    }

	    if ((rs >= 0) && lip->open.pidlock &&
	        ((pip->daytime - ti_lock) >= TO_PID)) {
		ti_lock = pip->daytime ;
	        rs = proclockcheck(pip,plp) ;
	    }

#if	CF_GNCACHE
	    if ((rs >= 0) && ((pip->daytime - ti_gncache) >= TO_CACHE)) {
		ti_gncache = pip->daytime ;
	        rs = gncache_check(&g,pip->daytime) ;
	    }
#endif /* CF_GNCACHE */

	    f = ((pip->daytime - ti_check) > TO_CHECK) ;
	    if ((rs >= 0) && f) {
		ti_check = pip->daytime ;
		rs = resolves_check(&m,pip->daytime) ;
	    }

	    if ((rs >= 0) && (pip->intrun > 0) &&
	        ((pip->daytime - ti_run) >= pip->intrun)) {

	        if (pip->efp != NULL)
	            shio_printf(pip->efp,"%s: exiting on run-int timeout\n",
	                pip->progname) ;

	        break ;
	    }

	    if ((pip->efp != NULL) && if_int)		/* fun only! */
		shio_printf(pip->efp,"%s: interrupt\n",	/* fun only! */
	                pip->progname) ;

	} /* end while */

	resolves_close(&m) ;

ret4:
	gncache_finish(&g) ;

ret3:
	uc_fdetach(mntfname) ;

ret2:
ret1:
	if (sfd >= 0) {
	    u_close(sfd) ;
	    sfd = -1 ;
	}

	if (cfd >= 0) {
	    u_close(cfd) ;
	    cfd = -1 ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/procserve: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procserve) */


static int prochandle(pip,gp,mp,uid,gid,pfd)
PROGINFO	*pip ;
GNCACHE		*gp ;
RESOLVES		*mp ;
uid_t		uid ;
gid_t		gid ;
int		pfd ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;

	char	groupname[GROUPNAMELEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_resolves/prochandle: uid=%u\n",uid) ;
	    debugprintf("b_resolves/prochandle: gid=%u\n",gid) ;
	}
#endif

	if (gid < 0)
	    goto ret0 ;

#if	CF_GNCACHE
	{
	    GNCACHE_ENT	gre ;
	    rs1 = gncache_lookgid(gp,&gre,gid) ;
	    if (rs1 >= 0)
		strwcpy(groupname,gre.groupname,GROUPNAMELEN) ;
	}
#else
	rs1 = getgroupname(groupname,GROUPNAMELEN,gid) ;
#endif /* CF_GNCACHE */
	if (rs1 < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/prochandle: groupname=%s\n",groupname) ;
#endif

#if	CF_PROCID
	{
	    RESOLVES_ID	id ;
	    resolvesid_load(&id,NULL,groupname,uid,gid) ;
	    rs = resolves_procid(mp,&id,NULL,pfd) ;
	    wlen = rs ;
	}
#else /* CF_PROCID */
#if	CF_ENVIRON
	{
	    struct locinfo	*lip = pip->lip ;
	    struct client	ci ;
	    memset(&ci,0,sizeof(struct client)) ;
	    ci.gn = groupname ;
	    ci.fd = pfd ;
	    ci.uid = uid ;
	    ci.gid = gid ;
	    rs = locinfo_process(lip,mp,&ci) ;
	    wlen = rs ;
	}
#else
	rs = resolves_process(mp,groupname,NULL,pfd) ;
	wlen = rs ;
#endif /* CF_ENVIRON */
#endif /* CF_PROCID */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_resolves/prochandle: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	if ((rs == SR_PIPE) || (rs = SR_HANGUP))
	    rs = SR_OK ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (prochandle) */


static int procextras(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if ((rs >= 0) && (pip->username == NULL)) {
	    char	username[USERNAMELEN + 1] ;
	    rs = getusername(username,USERNAMELEN,-1) ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->username,username,-1) ;
	} /* end if (username) */

	if ((rs >= 0) && (pip->nodename == NULL)) {
	    char	nodename[NODENAMELEN + 1] ;
	    char	domainname[MAXHOSTNAMELEN + 1] ;
	    rs = getnodedomain(nodename,domainname) ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->nodename,nodename,-1) ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->domainname,domainname,-1) ;
	} /* end if (nodename-domainname) */

ret0:
	return rs ;
}
/* end subroutine (procextras) */


static int procpidfile(pip)
PROGINFO	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;

	const char	*pf = lip->pidfname ;

	char	cname[MAXNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	pf = lip->pidfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procpidfile: entered\n") ;
#endif

	if ((pf == NULL) || (pf[0] == '+')) {

	    rs = snsds(cname,MAXNAMELEN,pip->nodename,PIDFNAME) ;

	    if (rs >= 0)
	        rs = mkpath3(tmpfname,pip->pr,RUNDNAME,cname) ;

	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&lip->pidfname,tmpfname,-1) ;

	} /* end if (pidfname) */

	if (pip->debuglevel > 0) {
	    pf = lip->pidfname ;
	    if ((pf != NULL) && (pf[0] != '\0') && (pf[0] != '-'))
	        shio_printf(pip->efp,"%s: pidfile=%s\n",
	            pip->progname,lip->pidfname) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_resolves/procpidfile: pidfname=%s\n",lip->pidfname) ;
	    debugprintf("b_resolves/procpidfile: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procpidfile) */


static int procregout(pip,app,ofp)
PROGINFO	*pip ;
PARAMOPT	*app ;
SHIO		*ofp ;
{
	vecstr	admins ;

	int	rs = SR_OK ;
	int	wlen = 0 ;

	const char	**av ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregout: entered\n") ;
#endif


	rs = vecstr_start(&admins,4,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_loadadmins(&admins,app) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregout: vecstr_loadadmins() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret1 ;

	rs = vecstr_getvec(&admins,&av) ;

	if (rs >= 0) {
	    rs = procregouter(pip,av,ofp) ;
	    wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_resolves: procregouter() rs=%d\n",rs) ;
#endif

	}

ret1:
	vecstr_finish(&admins) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregout) */


static int procregouter(pip,av,ofp)
PROGINFO	*pip ;
const char	**av ;
SHIO		*ofp ;
{
	struct locinfo	*lip = pip->lip ;

	const mode_t	operms = 0644 ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	bl ;
	int	oflags = O_RDWR ;
	int	mlen ;
	int	wlen = 0 ;

	char	template[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	if (lip->groupname[0] == '\0')
	    goto ret0 ;

	rs = locinfo_mdname(lip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = mkpath2(template,lip->mdname,"motdXXXXXXXXXX") ;
	if (rs < 0)
	    goto ret0 ;

	rs = opentmpfile(template,oflags,operms,tmpfname) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregouter: procmotd() fd=%d\n",fd) ;
#endif

	rs = procmotd(pip,lip->groupname,av,fd) ;
	mlen = rs ;

	if ((rs >= 0) && if_int)
		    rs = SR_INTR ;

	if ((rs >= 0) && if_exit)
		    rs = SR_EXIT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregouter: procmotd() rs=%d\n",rs) ;
#endif

	if ((rs > 0) && (mlen > 0))
	    rs = u_rewind(fd) ;

	if ((rs >= 0) && (mlen > 0) && (pip->verboselevel > 0)) {
	    char	buf[BUFLEN + 1] ;

	    while ((rs = u_read(fd,buf,BUFLEN)) > 0) {
	        bl = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_resolves/procregouter: u_read() rs=%d\n",rs) ;
#endif

	        rs = shio_write(ofp,buf,bl) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

		if (if_int) {
		    rs = SR_INTR ;
		    break ;
		}

		if (if_exit) {
		    rs = SR_EXIT ;
		    break ;
		}

	    } /* end while */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_resolves/procregouter: while-bot rs=%d\n",rs) ;
#endif

	} /* end if */

ret1:
	if (fd >= 0) {
	    u_close(fd) ;
	    fd = -1 ;
	}

	if (tmpfname[0] != '\0') {
	    u_unlink(tmpfname) ;
	    tmpfname[0] = '\0' ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procregouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregouter) */


static int procmotd(pip,groupname,av,fd)
PROGINFO	*pip ;
const char	groupname[] ;
const char	**av ;
int		fd ;
{
	struct locinfo	*lip = pip->lip ;

	RESOLVES	m ;

	int	rs ;
	int	wlen = 0 ;


	if (groupname == NULL)
	    return SR_FAULT ;

	if (groupname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_resolves/procmotd: tar uid=%u\n",lip->uid_motd) ;
	    debugprintf("b_resolves/procmotd: tar gid=%u\n",lip->gid_motd) ;
	}
#endif

	rs = resolves_open(&m,pip->pr) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_PROCID
	{
	    RESOLVES_ID	id ;
	    resolvesid_load(&id,lip->un,groupname,lip->uid_motd,lip->gid_motd) ;
	    rs = resolves_procid(&m,&id,av,fd) ;
	    wlen = rs ;
	}
#else /* CF_PROCID */
#if	CF_ENVIRON
	{
	    struct client	ci ;
	    memset(&ci,0,sizeof(struct client)) ;
	    ci.un = lip->un ;
	    ci.gn = groupname ;
	    ci.fd = fd ;
	    ci.uid = lip->uid_motd ;
	    ci.gid = lip->gid_motd ;
	    ci.avp = av ;
	    rs = locinfo_process(lip,&m,&ci) ;
	    wlen = rs ;
	}
#else
	rs = resolves_process(&m,groupname,av,fd) ;
	wlen = rs ;
#endif /* CF_ENVIRON */
#endif /* CF_PROCID */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_resolves/procmotd: resolves_process() rs=%d\n",rs) ;
#endif

	resolves_close(&m) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmotd) */


static int proclockcheck(pip,plp)
PROGINFO	*pip ;
LFM		*plp ;
{
	LFM_CHECK	lc ;

	int	rs = SR_OK ;


	rs = lfm_check(plp,&lc,pip->daytime) ;

	if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN))
	    proclockprint(pip,&lc) ;

	return rs ;
}
/* end subroutine (proclockcheck) */


/* print out lock-check information */
static int proclockprint(pip,lcp)
PROGINFO	*pip ;
LFM_CHECK	*lcp ;
{
	int		rs = SR_OK ;
	const char	*np ;
	char		timebuf[TIMEBUFLEN + 1] ;

	switch (lcp->stat) {

	case SR_AGAIN:
	    np = "busy" ;
	    break ;

	case SR_LOCKLOST:
	    np = "lost" ;
	    break ;

	default:
	    np = "unknown" ;
	    break ;

	} /* end switch */

#ifdef	COMMENT
	if (pip->open.logfile) {

	    logfile_printf(&pip->lh,
	        "%s lock %s\n",
	        timestr_logz(pip->daytime,timebuf),
	        np) ;

	    logfile_printf(&pip->lh,
	        "other_pid=%d\n",
	        lcp->pid) ;

	    if (lcp->nodename != NULL)
	        logfile_printf(&pip->lh,
	            "other_node=%s\n",
	            lcp->nodename) ;

	    if (lcp->username != NULL)
	        logfile_printf(&pip->lh,
	            "other_user=%s\n",
	            lcp->username) ;

	    if (lcp->banner != NULL)
	        logfile_printf(&pip->lh,
	            "other_banner=%s\n",
	            lcp->banner) ;

	} /* end if (logging) */
#endif /* COMMENT */

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {

	    shio_printf(pip->efp,
	        "%s: %s lock %s\n",
	        pip->progname,
	        timestr_logz(pip->daytime,timebuf),
	        np) ;

	    shio_printf(pip->efp,
	        "%s: other_pid=%d\n",
	        pip->progname,lcp->pid) ;

	    if (lcp->nodename != NULL)
	        shio_printf(pip->efp,
	            "%s: other_node=%s\n",
	            pip->progname,lcp->nodename) ;

	    if (lcp->username != NULL)
	        rs = shio_printf(pip->efp,
	            "%s: other_user=%s\n",
	            pip->progname,lcp->username) ;

	    if (lcp->banner != NULL)
	        shio_printf(pip->efp,
	            "%s: other_banner=»%s«\n",
	            pip->progname,lcp->banner) ;

	} /* end if (standard-error) */

	return rs ;
}
/* end subroutine (proclockprint) */


static int vecstr_loadadmins(alp,app)
vecstr		*alp ;
PARAMOPT	*app ;
{
	PARAMOPT_CUR	pcur ;

	int	rs = SR_OK ;
	int	cl ;
	int	c = 0 ;

	const char	*po = PO_ADMIN ;
	const char	*cp ;


	if ((rs = paramopt_curbegin(app,&pcur)) >= 0) {

	    while (rs >= 0) {

	        cl = paramopt_fetch(app,po,&pcur,&cp) ;
	        if (cl == SR_NOTFOUND) break ;

	        rs = cl ;
	        if ((rs >= 0) && (cl > 0)) {
#if	CF_DEBUGS
		debugprintf("vecstr_loadadmins: a=%t\n",cp,cl) ;
#endif
	            rs = vecstr_adduniq(alp,cp,cl) ;
	            if (rs < INT_MAX)
	                c += 1 ;
	        }

	    } /* end while */

	    paramopt_curend(app,&pcur) ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadadmins) */


static int deleter(vap)
void		*vap ;
{
	struct dargs	*dap = (struct dargs *) vap ;

	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("msgfile/deleter: entered\n") ;
#endif

	if (rs >= 0)
	    rs = deleter_all(dap) ;

	return rs ;
}
/* end subroutine (deleter) */


static int deleter_all(dap)
struct dargs	*dap ;
{
	struct ustat	usb ;

	FSDIR		dir ;

	FSDIR_ENT	de ;

	vecstr		files ;

	time_t		daytime = time(NULL) ;

	const int	to = TO_TMPFILE ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	fl ;
	int	i ;

	const char	*fp ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (dap->tmpdname == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	if (dap->tmpdname[0] == '\0') {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = vecstr_start(&files,0,0) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("msgfile/deleter_all: tmpdname=%s\n",dap->tmpdname) ;
#endif

	if ((rs = fsdir_open(&dir,dap->tmpdname)) >= 0) {

	while (fsdir_read(&dir,&de) > 0) {

	    if (de.name[0] == '.') continue ;

	    rs1 = mkpath2(tmpfname,dap->tmpdname,de.name) ;
	    fl = rs1 ;
	    if (rs1 >= 0)
	        rs1 = u_stat(tmpfname,&usb) ;

#if	CF_DEBUGS
	debugprintf("msgfile/deleter_all: name=%s rs1=%d\n",
			de.name,rs1) ;
#endif

	    if (rs1 >= 0) {
		if ((daytime - usb.st_mtime) >= to) {

#if	CF_DEBUGS
	debugprintf("msgfile/deleter_all: sched_del name=%s\n",
			de.name) ;
#endif

		    rs = vecstr_add(&files,tmpfname,fl) ;

		}
	    } /* end if */

	    if (rs < 0)
		break ;

	} /* end while */

	fsdir_close(&dir) ;
	} /* end if (fsdir) */

	if (rs >= 0) {
	    for (i = 0 ; vecstr_get(&files,i,&fp) >= 0 ; i += 1) {
	        if (fp == NULL) continue ;
	        if (fp[0] != '\0') {

#if	CF_DEBUGS
	debugprintf("msgfile/deleter_all: unlink name=%s\n",fp) ;
#endif

		    u_unlink(fp) ;
		}
	    } /* end for */
	} /* end if */

ret1:
	vecstr_finish(&files) ;

ret0:
	return rs ;
}
/* end subroutine (deleter_all) */


