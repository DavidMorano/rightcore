/* b_motd */

/* SHELL built-in for Message-of-the-Day (MOTD) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGN	0		/* special */
#define	CF_GNCACHE	1		/* use GID (group) cache */
#define	CF_PROCID	1		/* call 'motd_processid(3dam)' */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_MOTDPROC	1		/* call |motd_processXX()| */
#define	CF_MOTD		1		/* call |procmotd()| */
#define	CF_LOCJOBDNAME	0		/* compile |locinfo_jobdname{}| */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was written as a KSH builtin.  

	= 2011-09-23, David A­D­ Morano
	I put the "environment" hack into this code.  See the design
	note in the comments below.

	= 2011-11-08, David A­D­ Morano
        I took the "environment" hack *out* of the code. It was actually not
        correct in a multithreaded environment. Currently (at this present time)
        the KSH Shell is not multithreaded, but other programs (notably servers)
        that dynamically load "programs" (commands) from a shared-object library
        containing these might someday (probably much sooner than KSH) become
        multithreaded. I needed to make environment handling multithreaded
        before this present command gets dynamically loaded and executed by some
        server or another. The fix was to take environment handling out of this
        code and to put into the MOTD object code. Also, a new LIBUC-level call
        had to be invented to handle the new case of opening a general file w/ a
        specified environment. The LIBUC call to open programs specifically
        handled passing environment already but it was not general for opening
        any sort of "file." The new LIBUC call ('uc_openenv(3uc)') is.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet.

	Synopsis:

	$ motd [-u <username>] [-a <admin(s)>] [-d[=<intrun>] [-V]

	Design problems:

	I put a real hack into this code.  The MOTD object was supposed to
	handle all aspects of the actual MOTD processing.  But a new issue
	arose.  People want any subprograms executed as a result of reading
	sub-MOTD files to know the client UID and GID (the only things that we
	know).  We are currently doing this by placing these as special
	environment variables into our own process environment before executing
	'motd_process()'.  But switching out own actual environment in a way
	that does not leak memory (meaning do not use 'putenv(3c)') adds a
	little complication, which can be seen below.  Somehow in the future we
	will try to move some kind of processing into the MOTD object itself.

	Updated note on design problems:

	The hack above to pass modified environment down to the MOTD object is
	no longer needed.  The MOTD object itself now handles that.  A new MOTD
	object method has been added to pass fuller specified identification
	down into the MOTD object.  This new interface is 'motd_processid()'.


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
#include	<sys/wait.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<vecobj.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<lfm.h>
#include	<getutmpent.h>
#include	<fsdir.h>
#include	<ptm.h>
#include	<filebuf.h>
#include	<termout.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<tmtime.h>
#include	<upt.h>
#include	<ascii.h>
#include	<buffer.h>
#include	<spawner.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_motd.h"
#include	"defs.h"
#include	"upt.h"
#include	"gncache.h"
#include	"motd.h"


/* local typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

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

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	2048
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	COLS_USERNAME	8
#define	COLS_REALNAME	39

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif

#define	EXTRAENVS	6		/* possible extra variables */

#undef	TMPDMODE
#define	TMPDMODE	(0777 | S_ISGID | S_ISVTX)

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9

#define	PO_ADMIN	"admin"

#define	TO_LASTFILE	(12*3600)	/* "last" file time-out */
#define	TO_GID		(5*60)		/* group (GID) cache time-out */
#define	TO_CHECK	(10*60)		/* MOTD check time-out */

#ifndef	TO_TMPFILES
#define	TO_TMPFILES	(1*3600)	/* temporary file time-out */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_GMCUR	struct locinfo_gmcur
#define	LOCINFO_RNCUR	struct locinfo_rncur


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mknpath2(char *,int,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getrunlevel(cchar *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	termwritable(cchar *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,
			mode_t) ;
extern int	prgetprogpath(cchar *,char *,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	hasalldig(cchar *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isIOError(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proginfo_rootname(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strdcpy2(char *,int,cchar *,cchar *) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


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
	uint		un:1 ;
	uint		mnt:1 ;
	uint		pidlock:1 ;
	uint		envm:1 ;
	uint		tmpstr:1 ;
	uint		fg:1 ;
	uint		termout:1 ;
	uint		outer:1 ;
	uint		gm:1 ;
	uint		rn:1 ;
	uint		tmpmaint:1 ;
	uint		maint:1 ;
	uint		mntfname:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	cchar		**envv ;
	cchar		*un ;
	cchar		*mdname ;
	cchar		*termtype ;
	cchar		*jobdname ;
	cchar		*mntfname ;
	IDS		id ;
	PTM		envm ;
	VECSTR		tmpstr ;
	TERMOUT		outer ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	pid_t		sid ;
	pid_t		pid ;
	uid_t		uid, euid ;
	uid_t		uid_pr ;
	uid_t		uid_prog ;
	uid_t		uid_dirs ;
	gid_t		gid, egid ;
	gid_t		gid_prog ;
	gid_t		gid_dirs ;
	gid_t		gid_pr ;
	pthread_t	tid ;
	int		to_cache ;
	int		to_lock ;
	int		envc ;
	char		unbuf[USERNAMELEN + 1] ;
	char		gnbuf[GROUPNAMELEN + 1] ;
	char		termfname[MAXPATHLEN + 1] ;
} ;

struct dargs {
	cchar		*tmpdname ;
} ;

struct client {
	cchar		**avp ;		/* admins-vector-pointer */
	cchar		*gn ;
	cchar		*un ;
	uid_t		uid ;
	gid_t		gid ;
	int		fd ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,cchar *) ;

static int	process(PROGINFO *,PARAMOPT *,cchar *) ;
static int	procbackinfo(PROGINFO *) ;
static int	procback(PROGINFO *) ;
static int	procbacks(PROGINFO *) ;
static int	procbackcheck(PROGINFO *) ;
static int	procbacker(PROGINFO *,cchar *,cchar **) ;
static int	procbackenv(PROGINFO *,SPAWNER *) ;
static int	procmntcheck(PROGINFO *) ;
static int	procdaemon(PROGINFO *) ;
static int	procdaemons(PROGINFO *) ;
static int	procdaemoncheck(PROGINFO *) ;

static int	procloadadmins(PROGINFO *,VECPSTR *,PARAMOPT *) ;
static int	procloadadmin(PROGINFO *,VECPSTR *,cchar *,int) ;
static int	procregular(PROGINFO *,PARAMOPT *,cchar *) ;
static int	procregulars(PROGINFO *,PARAMOPT *,cchar *) ;
static int	procregout(PROGINFO *,PARAMOPT *,SHIO *) ;
static int	procregouter(PROGINFO *,cchar **,SHIO *) ;
static int	procregouterterm(PROGINFO *,void *,int) ;
static int	procregouterfile(PROGINFO *,void *,int) ;
static int	procmotd(PROGINFO *,cchar *,cchar **,int) ;
static int	procextras(PROGINFO *) ;
static int	procpidfname(PROGINFO *) ;
static int	proclockacquire(PROGINFO *,LFM *,int) ;
static int	proclockrelease(PROGINFO *,LFM *) ;
static int	proclockcheck(PROGINFO *,LFM *) ;
static int	proclockprint(PROGINFO *,LFM_CHECK *) ;
static int	procdown(PROGINFO *,LFM *,cchar *) ;
static int	procserve(PROGINFO *,LFM *,cchar *) ;
static int	procserver(PROGINFO *,LFM *,int) ;
static int	prochandle(PROGINFO *,GNCACHE *,MOTD *,uid_t,gid_t,int) ;
static int	procexecname(PROGINFO *,char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_loadids(LOCINFO *) ;
static int	locinfo_mdname(LOCINFO *) ;
static int	locinfo_tmpcheck(LOCINFO *) ;
static int	locinfo_tmpmaint(LOCINFO *) ;
static int	locinfo_tmpdone(LOCINFO *) ;
static int	locinfo_fchmodown(LOCINFO *,int,struct ustat *,mode_t) ;
static int	locinfo_getgid(LOCINFO *) ;
static int	locinfo_chgrp(LOCINFO *,cchar *) ;
static int	locinfo_termoutbegin(LOCINFO *,void *) ;
static int	locinfo_termoutend(LOCINFO *) ;
static int	locinfo_termoutprint(LOCINFO *,void *,cchar *,int) ;
static int	locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,cchar *,int) ;
static int	locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;
static int	locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
static int	locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
static int	locinfo_loadprids(LOCINFO *) ;
static int	locinfo_username(LOCINFO *) ;
static int	locinfo_groupname(LOCINFO *) ;

#if	CF_LOCJOBDNAME
static int	locinfo_jobdname(LOCINFO *) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"H",
	"HELP",
	"LOGFILE",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"mnt",
	"pid",
	"fg",
	"daemon",
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
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_mnt,
	argopt_pid,
	argopt_fg,
	argopt_daemon,
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
	{ SR_BUSY, EX_TEMPFAIL },
	{ SR_EXIT, EX_TERM },
	{ SR_INTR, EX_INTR },
	{ 0, 0 }
} ;

static const char	*akonames[] = {
	"quiet",
	"intrun",
	"termout",
	"maint",
	"daemon",
	"pidfile",
	"mntfile",
	"reuseaddr",
	NULL
} ;

enum akonames {
	akoname_quiet,
	akoname_intrun,
	akoname_termout,
	akoname_maint,
	akoname_daemon,
	akoname_pidfile,
	akoname_mntfile,
	akoname_reuseaddr,
	akoname_overlast
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	SIGHUP,
	0
} ;


/* exported subroutines */


int b_motd(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_motd) */


int p_motd(int argc,cchar *argv[],cchar *envv[],void *contextp)
{

	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_motd) */


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
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	cchar		*po_admin = PO_ADMIN ;
	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*cp = NULL ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_motd: starting DFD=%d\n",rs) ;
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
	pip->to_open = -1 ;
	pip->to_read = -1 ;
	pip->daytime = time(NULL) ;

	pip->lip = &li ;
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

	                case argopt_pid:
	                    pip->have.pidfname = TRUE ;
	                    pip->final.pidfname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->pidfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pidfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_mnt:
	                    lip->have.mnt = TRUE ;
	                    lip->final.mnt = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->mntfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->mntfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                case argopt_daemon:
	                    pip->final.daemon = TRUE ;
	                    pip->have.daemon = TRUE ;
	                    pip->f.daemon = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intrun = v ;
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

	                    case 'P':
	                        pip->have.pidfname = TRUE ;
	                        pip->final.pidfname = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->pidfname = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->pidfname = argp ;
	                            } else
	                                rs = SR_INVALID ;
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
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* terminal-type */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* print header */
	                    case 'a':
	                        pip->have.aparams = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					PARAMOPT	*pop = &aparams ;
	                                cchar	*po = po_admin ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'd':
	                        pip->have.background = TRUE ;
	                        pip->f.background = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                pip->final.intrun = TRUE ;
	                                pip->have.intrun = TRUE ;
	                                pip->intrun = 0 ;
	                                if (avp[0] != '-') {
	                                    rs = cfdecti(avp,avl,&v) ;
	                                    pip->intrun = v ;
	                                }
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

/* target username */
	                    case 'u':
	                        lip->have.un = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->un = argp ;
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
	    debugprintf("b_motd: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    int	f_sfio = FALSE ;
	    int	f_builtin = FALSE ;
#if	CF_SFIO
	    f_sfio = TRUE ;
#endif
#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
	    f_builtin = TRUE ;
#endif
	    shio_printf(pip->efp,"%s: f_sfio=%u f_builtin=%u\n",
	        pip->progname,f_sfio,f_builtin) ;
	}

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

/* check if we should be doing anything */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if ((rs = getrunlevel(NULL)) >= 0) {
	    switch (rs) {
	    case '2':
	    case '3':
	    case '4':
	        break ;
	    default:
	        if (pip->debuglevel > 0) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: administrative run-level\n" ;
	            shio_printf(pip->efp,fmt,pn) ;
	        }
	        ex = EX_TEMPFAIL ;
	        break ;
	    } /* end if */
	}

	if ((rs >= 0) && (pip->intrun == 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&v) ;
	    pip->intrun = v ;
	}

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG
	debugprintf("b_motd: pidfname=%s\n",pip->pidfname) ;
	debugprintf("b_motd: mntfname=%s\n",lip->mntfname) ;
#endif

/* argument defaults */

	if (lip->un == NULL) lip->un = getourenv(envv,VARTARUSER) ;

	if (pip->intpoll == 0) pip->intpoll = TO_POLL ;

	if (pip->intmark == 0) pip->intmark = TO_MARK ;

	if (pip->intlock == 0) pip->intlock = TO_LOCK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_motd: intrun=%d\n",pip->intrun) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: intrun=%d\n",pn,pip->intrun) ;
	    shio_printf(pip->efp,"%s: intpoll=%d\n",pn,pip->intpoll) ;
	    shio_printf(pip->efp,"%s: intmark=%d\n",pn,pip->intmark) ;
	}

/* other initilization */

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* other */

	if (lip->mntfname == NULL)
	    lip->mntfname = getourenv(envv,VARMNTFNAME) ;

	if (lip->mntfname != NULL)
	    pip->f.daemon = TRUE ;

	if ((lip->mntfname == NULL) || (lip->mntfname[0] == '\0'))
	    lip->mntfname = MNTFNAME ;

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*afn = afname ;
	    if ((rs = procargs(pip,&ainfo,&pargs,&aparams,afn)) >= 0) {
	        rs = process(pip,&aparams,ofname) ;
	    } /* end if (procargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex != EX_OK) && (! pip->f.quiet)) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: could not perform function (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	}
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_BUSY:
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_motd: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
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

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
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

	fmt = "%s: USAGE> %s [-u <username>] [<admin(s)>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-d[=<intrun>]] [-mnt <mntfile>] [-pid <pidfile>]\n" ;
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
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	v ;
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

			vl = keyopt_fetch(kop,kp,NULL,&vp) ;

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
	                case akoname_termout:
	                    if (! lip->final.termout) {
	                        lip->have.termout = TRUE ;
	                        lip->final.termout = TRUE ;
	                        lip->f.termout = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.termout = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_maint:
	                    if (! lip->final.maint) {
	                        lip->have.maint = TRUE ;
	                        lip->final.maint = TRUE ;
	                        lip->f.maint = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.maint = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_daemon:
	                    if (! pip->final.daemon) {
	                        pip->have.daemon = TRUE ;
	                        pip->final.daemon = TRUE ;
	                        pip->f.daemon = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.daemon = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_pidfile:
	                    if (! pip->final.pidfname) {
	                        if (vl > 0) {
	                            cchar **vpp = &pip->pidfname ;
	                            pip->have.pidfname = TRUE ;
	                            pip->final.pidfname = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_mntfile:
	                    if (! lip->final.mntfname) {
	                        lip->have.mntfname = TRUE ;
	                        lip->final.mntfname = TRUE ;
	                        if (vl > 0) {
	                            cchar **vpp = &lip->mntfname ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_reuseaddr:
	                    if (! pip->final.reuseaddr) {
	                        pip->have.reuseaddr = TRUE ;
	                        pip->final.reuseaddr = TRUE ;
	                        pip->f.reuseaddr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_intrun:
	                    if (! pip->final.intrun) {
	                        pip->have.intrun = TRUE ;
	                        pip->final.intrun = TRUE ;
	                        pip->f.intrun = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->intrun = v ;
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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		PARAMOPT *app,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*po_admin = PO_ADMIN ;
	cchar		*cp ;

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
	                rs = paramopt_loads(app,po_admin,cp,-1) ;
	            }
	        }

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
	                    rs = paramopt_loads(app,po_admin,cp,cl) ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procargs: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int process(PROGINFO *pip,PARAMOPT *app,cchar *ofn)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.background || pip->f.daemon) {
	    if ((rs = procextras(pip)) >= 0) {
	        if ((rs = procpidfname(pip)) >= 0) {
	            if ((rs = procbackinfo(pip)) >= 0) {
	                if ((rs = ids_load(&pip->id)) >= 0) {
	                    if (pip->f.background) {
	                        rs = procback(pip) ;
	                    } else if (pip->f.daemon) {
	                        rs = procdaemon(pip) ;
	                    }
	                    rs1 = ids_release(&pip->id) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (ids) */
	            } /* end if (procbackinfo) */
	        } /* end if (procpidfname) */
	    } /* end if (procextras) */
	} else {
	    rs = procregular(pip,app,ofn) ;
	}

	return rs ;
}
/* end subroutine (process) */


static int procbackinfo(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*mntfname ;

	mntfname = lip->mntfname ;
	if (pip->debuglevel > 0) {

	    shio_printf(pip->efp,"%s: mntfile=%s\n",pn,mntfname) ;

	    fmt = "%s: runint=(inf)\n" ;
	    if (pip->intrun > 0) fmt = "%s: runint=%u\n",
	    shio_printf(pip->efp,fmt,pn,pip->intrun) ;

	} /* end if */

	return rs ;
}
/* end subroutine (procbackinfo) */


static int procback(PROGINFO *pip)
{
	int		rs ;

	if (pip->open.logprog) {
	    logfile_printf(&pip->lh,"mode=background") ;
	    logfile_flush(&pip->lh) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mode=background\n",pip->progname) ;
	    bflush(pip->efp) ;
	}

	if ((rs = procbackcheck(pip)) >= 0) {
	    rs = procbacks(pip) ;
	}

	return rs ;
}
/* end subroutine (procback) */


static int procbackcheck(PROGINFO *pip)
{
	LFM		pidlock, *plp = &pidlock ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procbackcheck: ent\n") ;
#endif

	if ((rs = proclockacquire(pip,plp,TRUE)) >= 0) {
	    {
	        rs = procmntcheck(pip) ;
	    }
	    rs1 = proclockrelease(pip,plp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
	        fmt = "%s: could not acquire PID lock (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (procbackcheck) */


static int procmntcheck(PROGINFO *pip)
{
	struct ustat	usb ;
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((rs = u_stat(lip->mntfname,&usb)) >= 0) {
	    if (S_ISREG(usb.st_mode)) {
	        rs = sperm(&pip->id,&usb,W_OK) ;
	    } else
	        rs = SR_BUSY ;
	    if (rs < 0) {
	        if (! pip->f.quiet) {
	            fmt = "%s: inaccessible mount point (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (procmntcheck) */


static int procbacks(PROGINFO *pip)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ebuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: ent\n") ;
#endif

	if ((rs = procexecname(pip,ebuf,elen)) >= 0) {
	    int		el = rs ;
	    cchar	*pf = ebuf ;
	    cchar	*tp ;
	    char	pbuf[MAXPATHLEN+1] ;

	    if (pip->debuglevel > 0) {
	        fmt = "%s: execname=%t\n" ;
	        shio_printf(pip->efp,fmt,pn,ebuf,el) ;
	    }

	    if ((tp = strnrpbrk(ebuf,el,"/.")) != NULL) {
	        if (tp[0] == '.') {
	            el = (tp-ebuf) ;
	            ebuf[el] = '\0' ;
	        }
	    }

	    if ((rs = prgetprogpath(pip->pr,pbuf,ebuf,el)) > 0) {
	        pf = pbuf ;
	    }

	    if (rs >= 0) {
	        int	i = 0 ;
	        cchar	*av[5] ;
	        char	dbuf[10+1] ;
	        if (pip->debuglevel > 0) {
	            fmt = "%s: pf=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,pf) ;
	        }
	        av[i++] = pip->progname ;
	        av[i++] = "-daemon" ;
	        if (pip->debuglevel > 0) {
	            bufprintf(dbuf,10,"-D=%u",pip->debuglevel) ;
	            av[i++] = dbuf ;
	        }
	        av[i++] = NULL ;
	        rs = procbacker(pip,pf,av) ;
	    } /* end if (ok) */
	} /* end if (procexecname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procback: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbacks) */


static int procbacker(PROGINFO *pip,cchar *pf,cchar **av)
{
	SPAWNER		s ;
	int		rs ;
	int		rs1 ;
	int		pid = 0 ;
	cchar		**ev = pip->envv ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ent\n") ;
#endif
	if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
	    if ((rs = procbackenv(pip,&s)) >= 0) {
	        int	i ;
	        for (i = 0 ; sigignores[i] > 0 ; i += 1) {
	            spawner_sigignore(&s,sigignores[i]) ;
	        }
	        spawner_setsid(&s) ;
	        if (pip->uid != pip->euid) {
	            spawner_seteuid(&s,pip->uid) ;
		}
	        if (pip->gid != pip->egid) {
	            spawner_setegid(&s,pip->gid) ;
		}
	        for (i = 0 ; i < 2 ; i += 1) {
	            spawner_fdclose(&s,i) ;
	        }
	        if ((rs = spawner_run(&s)) >= 0) {
	            LOGFILE	*lfp = &pip->lh ;
	            cchar	*fmt ;
	            pid = rs ;
	            if (pip->open.logprog) {
	                fmt = "backgrounded (%u)" ;
	                logfile_printf(lfp,fmt,pid) ;
	            }
	        }
	    } /* end if (procbackenv) */
	    rs1 = spawner_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (spawner) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ret rs=%d pid=%u\n",rs,pid) ;
#endif
	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (procbacker) */


static int procbackenv(PROGINFO *pip,SPAWNER *srp)
{
	LOCINFO		*lip = pip->lip ;
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	cchar		*varopts = VAROPTS ;
	if ((rs = buffer_start(&b,ENVBUFLEN)) >= 0) {
	    cchar	*np ;
	    int		v ;
	    int		i ;
	    int		c = 0 ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        np = NULL ;
	        switch (i) {
	        case 0:
	            v = pip->intrun ;
	            if (v > 0) np = "intrun" ;
	            break ;
	        case 1:
	            v = pip->intidle ;
	            if (v > 0) np = "intidle" ;
	            break ;
	        case 2:
	            v = (pip->f.reuseaddr&1) ;
	            if (v > 0) np = "resueaddr" ;
	            break ;
	        } /* end switch */
	        if (np != NULL) {
	            if (c++ > 0) {
	                buffer_char(&b,CH_COMMA) ;
	            }
	            rs = buffer_printf(&b,"%s=%d",np,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        cchar	*vp ;
	        for (i = 0 ; i < 2 ; i += 1) {
	            np = NULL ;
	            switch (i) {
	            case 0:
	                if (pip->pidfname != NULL) {
	                    np = "pidfile" ;
	                    vp = pip->pidfname ;
	                }
	                break ;
	            case 1:
	                if (lip->mntfname != NULL) {
	                    np = "mntfile" ;
	                    vp = lip->mntfname ;
	                }
	                break ;
	            } /* end switch */
	            if (np != NULL) {
	                if (c++ > 0) {
	                    buffer_char(&b,CH_COMMA) ;
	                }
	                rs = buffer_printf(&b,"%s=%s",np,vp) ;
	            } /* end if (non-null) */
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (c > 0)) {
	        if ((rs = buffer_get(&b,&np)) >= 0) {
	            rs = spawner_envset(srp,varopts,np,rs) ;
	        }
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbackenv: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbackenv) */


static int procdaemon(PROGINFO *pip)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procdaemon: ent\n") ;
#endif

	if (pip->open.logprog) {
	    logfile_printf(&pip->lh,"mode=daemon") ;
	}
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mode=daemon\n",pip->progname) ;
	}

	if ((rs = procdaemoncheck(pip)) >= 0) {
	    rs = procdaemons(pip) ;
	}

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: daemon exiting (%d)\n",
	        pip->progname,rs) ;
	}

	return rs ;
}
/* end subroutine (procdaemon) */


static int procdaemoncheck(PROGINFO *pip)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procdaemoncheck: ent\n") ;
#endif

	rs = procmntcheck(pip) ;

	return rs ;
}
/* end subroutine (procdaemoncheck) */


static int procregular(PROGINFO *pip,PARAMOPT *app,cchar ofname[])
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procregular: un=%s\n",lip->un) ;
#endif

	if ((rs = locinfo_loadids(lip)) >= 0) {

	    if ((pip->debuglevel > 0) && (lip->gnbuf != NULL)) {
	        shio_printf(pip->efp,"%s: group=%s\n",
	            pip->progname,lip->gnbuf) ;
	    }

	    rs = procregulars(pip,app,ofname) ;
	    wlen += rs ;

	} /* end if (locinfo_loadids) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregular) */


static int procregulars(PROGINFO *pip,PARAMOPT *app,cchar ofn[])
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {

	    if ((rs = locinfo_termoutbegin(lip,ofp)) >= 0) {

	        rs = procregout(pip,app,ofp) ;
	        wlen += rs ;

	        rs1 = locinfo_termoutend(lip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (termout) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregulars) */


static int procdaemons(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procdaemons: ent\n") ;
#endif

	if (rs >= 0) {
	    LFM	pidlock, *plp = &pidlock ;
	    if ((rs = proclockacquire(pip,plp,FALSE)) >= 0) {
	        cchar	*mntfname = lip->mntfname ;

	        rs = procdown(pip,plp,mntfname) ;

	        rs1 = proclockrelease(pip,plp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (proc-lock) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/procdaemons: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdaemons) */


static int proclockacquire(PROGINFO *pip,LFM *plp,int f)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*pfn = pip->pidfname ;

#if	CF_DEBUG
	debugprintf("proclockacquire: ent pidfname=%s\n",pfn) ;
#endif

	if ((pfn != NULL) && (pfn[0] != '\0') && (pfn[0] != '-')) {
	    USTAT	usb ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    if (f) {
	        int	cl ;
	        cchar	*cp ;

	        cl = sfdirname(pip->pidfname,-1,&cp) ;

	        rs = mkpath1w(tmpfname,cp,cl) ;

	        if ((rs >= 0) && (u_stat(tmpfname,&usb) == SR_NOENT)) {
	            mkdirs(tmpfname,TMPDMODE) ;
	            locinfo_chgrp(lip,tmpfname) ;
	        }

	    } /* end if (checking PID directory) */

	    if (rs >= 0) {
	        LFM_CHECK	lc ;
	        const int	ltype = LFM_TRECORD ;
	        const int	to = lip->to_lock ;
	        cchar		*nn = pip->nodename ;
	        cchar		*un = pip->username ;
	        cchar		*bn = pip->banner ;

	        rs = lfm_start(plp,pfn,ltype,to,&lc,nn,un,bn) ;
	        lip->open.pidlock = (rs >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("proclockacquire: lfm_start() rs=%d\n",rs) ;
#endif

	        if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
	            proclockprint(pip,&lc) ;
		}

	    } /* end if */

	} /* end if (pidlock) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proclockacquire: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proclockacquire) */


static int proclockrelease(PROGINFO *pip,LFM *plp)
{
	LOCINFO		*lip = pip->lip ;
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


static int procdown(PROGINFO *pip,LFM *plp,cchar mntfname[])
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/procdown: mntfile=%s\n",mntfname) ;
#endif

	rs = procserve(pip,plp,mntfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/procdown: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdown) */


static int procserve(PROGINFO *pip,LFM *plp,cchar mntfname[])
{
	int		rs ;
	int		pipes[2] ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (mntfname[0] == '\0') return SR_INVALID ;

	if ((rs = u_pipe(pipes)) >= 0) {
	    const int	sfd = pipes[0] ;		/* server-side */
	    int		cfd = pipes[1] ;		/* client-side */
	    if ((rs = u_ioctl(cfd,I_PUSH,"connld")) >= 0) {
	        if ((rs = uc_fattach(cfd,mntfname)) >= 0) {
	            u_close(cfd) ;
	            cfd = -1 ;
	            if ((rs = uc_closeonexec(sfd,TRUE)) >= 0) {
	                rs = procserver(pip,plp,sfd) ;
	            }
	            uc_fdetach(mntfname) ;
	        } else {
	            if ((! pip->f.quiet) && (pip->efp != NULL)) {
	                fmt = "%s: could not perform mount (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	            }
	        }
	    } /* end if (u_ioctl) */
	    if (cfd >= 0) u_close(cfd) ;
	    u_close(sfd) ;
	} /* end if (u_pipe) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/procserve: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procserve) */


static int procserver(PROGINFO *pip,LFM *plp,int sfd)
{
	LOCINFO		*lip = pip->lip ;
	GNCACHE		g ;
	time_t		ti_pid = pip->daytime ;
	time_t		ti_gncache = pip->daytime ;
	time_t		ti_run = pip->daytime ;
	time_t		ti_wait = pip->daytime ;
	time_t		ti_check = pip->daytime ;
	const int	to_gid = TO_GID ;
	const int	to = pip->intpoll ;
	int		rs ;
	int		rs1 ;
	int		nhandle = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = gncache_start(&g,211,to_gid)) >= 0) {
	    MOTD	m ;
	    if ((rs = motd_open(&m,pip->pr)) >= 0) {
	        POLLFD	fds[2] ;
	        int	pto ;
	        int	i = 0 ;
	        int	f = FALSE ;

	        fds[i].fd = sfd ;
	        fds[i].events = (POLLIN | POLLPRI) ;
	        i += 1 ;
	        fds[i].fd = -1 ;
	        fds[i].events = 0 ;

	        pto = (to * POLLMULT) ;
	        while (rs >= 0) {

	            if ((rs = u_poll(fds,1,pto)) > 0) {
	                const int	re = fds[0].revents ;
	                pip->daytime = time(NULL) ;

	                if ((re & POLLIN) || (re & POLLPRI)) {
	                    struct strrecvfd	passer ;
	                    if ((rs = acceptpass(sfd,&passer,-1)) >= 0) {
	                        const uid_t	uid = passer.uid ;
	                        const gid_t	gid = passer.gid ;
	                        const int	pfd = rs ;
	                        nhandle += 1 ;
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

		    } else {
	                pip->daytime = time(NULL) ;
	                if (rs == SR_INTR) rs = SR_OK ;
		    }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;

	            f = ((pip->daytime - ti_wait) > (to*10)) ;
	            if ((rs >= 0) && (nhandle || f)) {
	                ti_wait = pip->daytime ;
	                rs1 = SR_OK ;
	                while ((nhandle > 0) || f) {
	                    rs1 = u_waitpid(-1,NULL,WNOHANG) ;
	                    if (rs1 <= 0) break ;
	                    if (nhandle > 0) nhandle -= 1 ;
	                }
	                if ((pip->daytime & 3) == 0) {
	                    if ((rs1 == SR_CHILD) && (nhandle > 0)) {
	                        nhandle -= 1 ;
	                    }
	                }
	            }

	            f = ((pip->daytime - ti_pid) >= TO_PID) ;
	            if ((rs >= 0) && f && lip->open.pidlock) {
	                ti_pid = pip->daytime ;
	                rs = proclockcheck(pip,plp) ;
	            }

#if	CF_GNCACHE
	            f = ((pip->daytime - ti_gncache) >= TO_CACHE) ;
	            if ((rs >= 0) && f) {
	                ti_gncache = pip->daytime ;
	                rs = gncache_check(&g,pip->daytime) ;
	            }
#endif /* CF_GNCACHE */

	            f = ((pip->daytime - ti_check) > TO_CHECK) ;
	            if ((rs >= 0) && f) {
	                ti_check = pip->daytime ;
	                rs = motd_check(&m,pip->daytime) ;
	            }

	            if ((rs >= 0) && (pip->intrun > 0) &&
	                ((pip->daytime - ti_run) >= pip->intrun)) {

	                if (pip->efp != NULL) {
	                    fmt = "%s: exiting on run-int timeout\n" ;
	                    shio_printf(pip->efp,fmt,pn) ;
	                }

	                break ;
	            } /* end if (run-expiration) */

	            if (rs == SR_INTR) {
	                if (pip->efp != NULL) {
	                    fmt = "%s: interrupt\n" ;
	                    shio_printf(pip->efp,fmt,pn) ;
	                }
	                rs = lib_sigreset(SIGINT) ;
	            } /* end if (interrupt) */

	        } /* end while (polling) */

	        rs1 = motd_close(&m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (motd) */
	    rs1 = gncache_finish(&g) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (gncache) */

	return rs ;
}
/* end subroutine (procserver) */


static int prochandle(PROGINFO *pip,GNCACHE *gp,MOTD *mp,
		uid_t uid,gid_t gid,int pfd)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	char		groupname[GROUPNAMELEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_motd/prochandle: uid=%d\n",uid) ;
	    debugprintf("b_motd/prochandle: gid=%d\n",gid) ;
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
	    debugprintf("b_motd/prochandle: groupname=%s\n",groupname) ;
#endif

#if	CF_MOTDPROC
#if	CF_PROCID
	{
	    MOTD_ID	id ;
	    motdid_load(&id,NULL,groupname,uid,gid) ;
	    rs = motd_processid(mp,&id,NULL,pfd) ;
	    wlen = rs ;
	}
#else /* CF_PROCID */
	rs = motd_process(mp,groupname,NULL,pfd) ;
	wlen = rs ;
#endif /* CF_PROCID */
#endif /* CF_MOTDPROC */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/prochandle: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	if (isIOError(rs)) {
	    rs = SR_OK ;
	    wlen = 0 ;
	}

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (prochandle) */


static int procextras(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	if ((rs = locinfo_username(lip)) >= 0) {
	    pip->username = lip->unbuf ;
	    if (pip->nodename == NULL) {
	        char	nn[NODENAMELEN + 1] ;
	        char	dn[MAXHOSTNAMELEN + 1] ;
	        if ((rs = getnodedomain(nn,dn)) >= 0) {
	            cchar	**vpp = &pip->nodename ;
	            if ((rs = proginfo_setentry(pip,vpp,nn,-1)) >= 0) {
	                cchar	**vpp = &pip->domainname ;
	                rs = proginfo_setentry(pip,vpp,dn,-1) ;
	            }
	        }
	    }
	} /* end if (locinfo_username) */
	return rs ;
}
/* end subroutine (procextras) */


static int procpidfname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*pfn = pip->pidfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procpidfname: ent\n") ;
#endif

	if ((pfn == NULL) || (pfn[0] == '+')) {
	    cchar	*nn = pip->nodename ;
	    cchar	*pr = pip->pr ;
	    char	cname[MAXNAMELEN + 1] ;
	    if ((rs = snsds(cname,MAXNAMELEN,nn,PIDFNAME)) >= 0) {
	        char	tbuf[MAXPATHLEN + 1] ;
	        if ((rs = mkpath3(tbuf,pr,RUNDNAME,cname)) >= 0) {
	            cchar	**vpp = &pip->pidfname ;
	            rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	        }
	    }
	} /* end if (pidfname) */

	if (pip->debuglevel > 0) {
	    pfn = pip->pidfname ;
	    if ((pfn != NULL) && (pfn[0] != '\0') && (pfn[0] != '-')) {
	        shio_printf(pip->efp,"%s: pidfile=%s\n",
	            pip->progname,pip->pidfname) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_motd/procpidfname: pidfname=%s\n",pip->pidfname) ;
	    debugprintf("b_motd/procpidfname: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procpidfname) */


static int procregout(PROGINFO	*pip,PARAMOPT *app,SHIO *ofp)
{
	vecpstr		admins ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procregout: ent\n") ;
#endif

	if ((rs = vecpstr_start(&admins,4,0,0)) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_motd/procregout: procloadadmins\n") ;
#endif
	    if ((rs = procloadadmins(pip,&admins,app)) >= 0) {
	        cchar	**av ;
	        if ((rs = vecpstr_getvec(&admins,&av)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_motd/procregout: procregouter()\n") ;
#endif
	            rs = procregouter(pip,av,ofp) ;
	            wlen += rs ;
	        } /* end if (vecpstr_getvec) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_motd/procregout: vecpstr_getvec rs=%d\n",
	                rs) ;
#endif
	    } /* end if (procloadadmins) */
	    vecpstr_finish(&admins) ;
	} /* end if (admins) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procregout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregout) */


static int procregouter(PROGINFO *pip,cchar **av,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (lip->gnbuf[0] != '\0') {
	    if ((rs = locinfo_mdname(lip)) >= 0) {
	        lip->jobdname = lip->mdname ;
	        if ((rs = locinfo_tmpcheck(lip)) >= 0) {
	            const mode_t	om = 0664 ;
	            const int		of = O_RDWR ;
	            cchar		*jobdname = lip->mdname ;
	            if ((rs = opentmp(jobdname,of,om)) >= 0) {
	                const int	fd = rs ;
	                if ((rs = procmotd(pip,lip->gnbuf,av,fd)) >= 0) {
	                    int	mlen = rs ;
	                    int	verlev = pip->verboselevel ;
	                    if ((rs >= 0) && (mlen > 0) && (verlev > 0)) {
	                        if ((rs = u_rewind(fd)) >= 0) {
	                            if (lip->open.outer) {
	                                rs = procregouterterm(pip,ofp,fd) ;
	                                wlen += rs ;
	                            } else {
	                                rs = procregouterfile(pip,ofp,fd) ;
	                                wlen += rs ;
	                            }
	                        } /* end if (ok) */
	                    } /* end if (output) */
	                } /* end if (procmotd) */
	                u_close(fd) ;
	            } /* end if (opentmp) */
	        } /* end if (locinfo_tmpcheck) */
	    } /* end if (jobdname) */
	} /* end if (non-null) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procregouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregouter) */


static int procregouterterm(PROGINFO *pip,void *ofp,int fd)
{
	LOCINFO		*lip = pip->lip ;
	FILEBUF		b ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	    const int	to = pip->to_read ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = filebuf_readlines(&b,lbuf,llen,to,NULL)) > 0) {
	        len = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("b_motd/procregouterterm: l=>%t<\n",
		lbuf,strlinelen(lbuf,len,40)) ;
#endif
		if (pip->debuglevel > 0) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: term l=>%t<\n" ;
		    shio_printf(pip->efp,fmt,pn,lbuf,strlinelen(lbuf,len,60)) ;
		}
	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs >= 0) {
	            rs = locinfo_termoutprint(lip,ofp,lbuf,len) ;
	            wlen += rs ;
	        }
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregouterterm) */


static int procregouterfile(PROGINFO *pip,void *ofp,int fd)
{
	const int	to = pip->to_read ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		len ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN+1] ;
	while ((rs = uc_reade(fd,lbuf,llen,to,FM_TIMED)) > 0) {
	    len = rs ;
		if (pip->debuglevel > 0) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: file l=>%t<\n" ;
		    shio_printf(pip->efp,fmt,pn,lbuf,strlinelen(lbuf,len,60)) ;
		}
	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;
	    if (rs >= 0) {
	        rs = shio_write(ofp,lbuf,len) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end while (reading) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procregouterfile) */


static int procmotd(PROGINFO *pip,cchar	*groupname,cchar **av,int fd)
{
	LOCINFO		*lip = pip->lip ;
	MOTD		m ;
	int		rs ;
	int		wlen = 0 ;

	if (groupname == NULL) return SR_FAULT ;

	if (groupname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_motd/procmotd: tar uid=%d\n",lip->uid_dirs) ;
	    debugprintf("b_motd/procmotd: tar gid=%d\n",lip->gid_dirs) ;
	}
#endif

#if	CF_MOTD
	if ((rs = motd_open(&m,pip->pr)) >= 0) {

#if	CF_MOTDPROC
#if	CF_PROCID
	    {
	        MOTD_ID		id ;
		const uid_t	uid = lip->uid_dirs ;
		const gid_t	gid = lip->gid_dirs ;
	        motdid_load(&id,lip->un,groupname,uid,gid) ;
	        rs = motd_processid(&m,&id,av,fd) ;
	        wlen = rs ;
	    }
#else /* CF_PROCID */
	    rs = motd_process(&m,groupname,av,fd) ;
	    wlen = rs ;
#endif /* CF_PROCID */
#endif /* CF_MOTDPROC */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        const int	srs = uc_fsize(fd) ;
	        debugprintf("b_motd/procmotd: motd_process() rs=%d\n",rs) ;
	        debugprintf("b_motd/procmotd: fsize=%d\n",srs) ;
	    }
#endif

	    motd_close(&m) ;
	} /* end if (motd) */
#else /* CF_MOTD */
	rs = SR_OK ;
#endif /* CF_MOTD */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procmotd: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmotd) */


static int proclockcheck(PROGINFO *pip,LFM *plp)
{
	LFM_CHECK	lc ;
	int		rs ;

	rs = lfm_check(plp,&lc,pip->daytime) ;

	if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
	    proclockprint(pip,&lc) ;
	}

	return rs ;
}
/* end subroutine (proclockcheck) */


/* print out lock-check information */
static int proclockprint(PROGINFO *pip,LFM_CHECK *lcp)
{
	int		rs = SR_OK ;
	cchar		*np ;
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
	if (pip->open.logprog) {

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


static int procloadadmins(PROGINFO *pip,vecpstr *alp,PARAMOPT *app)
{
	PARAMOPT_CUR	pcur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = paramopt_curbegin(app,&pcur)) >= 0) {
	    int		cl ;
	    cchar	*po = PO_ADMIN ;
	    cchar	*cp ;
	    while (rs >= 0) {
	        cl = paramopt_fetch(app,po,&pcur,&cp) ;
	        if (cl == SR_NOTFOUND) break ;
	        rs = cl ;
	        if ((rs >= 0) && (cl > 0)) {
	            rs = procloadadmin(pip,alp,cp,cl) ;
	            c += rs ;
	        }
	    } /* end while */
	    rs1 = paramopt_curend(app,&pcur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramopt-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_motd/procloadadmins: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadadmins) */


static int procloadadmin(PROGINFO *pip,VECPSTR *nlp,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadrecip: rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || hasMeAlone(np,nl)) {
	    if ((rs = locinfo_username(lip)) >= 0) {
	        np = lip->unbuf ;
	        nl = rs ;
	        rs = vecpstr_adduniq(nlp,np,nl) ;
	        if (rs < INT_MAX) c += 1 ;
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
	                    rs = vecpstr_adduniq(nlp,ub,rs) ;
	                    if (rs < INT_MAX) c += 1 ;
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
	                        rs = vecpstr_adduniq(nlp,ub,rs) ;
	                        if (rs < INT_MAX) c += 1 ;
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
	            rs = vecpstr_adduniq(nlp,np,nl) ;
	            if (rs < INT_MAX) c += 1 ;
		}
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadadmin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadadmin) */


static int procexecname(PROGINFO *pip,char *rbuf,int rlen)
{
	int		rs ;
	if ((rs = proginfo_progdname(pip)) >= 0) {
	    cchar	*dn = pip->progdname ;
	    cchar	*pn = pip->progname ;
	    rs = mknpath2(rbuf,rlen,dn,pn) ;
	}
	return rs ;
}
/* end subroutine (procexecname) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*varterm = VARTERM ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->gid_prog = -1 ;
	lip->gid_dirs = -1 ;
	lip->gid_pr = -1 ;
	lip->gid = getgid() ;
	lip->egid = getegid() ;
	lip->uid_prog = -1 ;
	lip->uid_dirs = -1 ;
	lip->uid_pr = -1 ;
	lip->uid = getuid() ;
	lip->euid = geteuid() ;
	lip->to_cache = -1 ;
	lip->to_lock = -1 ;
	lip->termtype = getourenv(pip->envv,varterm) ;

	{
	    PROGINFO	*pip = lip->pip ;
	    pip->uid = lip->uid ;
	    pip->euid = lip->euid ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

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

	rs1 = locinfo_tmpdone(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->mdname != NULL) {
	    rs1 = uc_free(lip->mdname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->mdname = NULL ;
	}

	if (lip->open.tmpstr) {
	    lip->open.tmpstr = FALSE ;
	    rs1 = vecstr_finish(&lip->tmpstr) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->envv != NULL) {
	    rs1 = uc_free(lip->envv) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->envv = NULL ;
	}

	if (lip->open.envm) {
	    lip->open.envm = FALSE ;
	    rs1 = ptm_destroy(&lip->envm) ;
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


#if	CF_LOCJOBDNAME
static int locinfo_jobdname(LOCINFO *lip)
{
	return locinfo_mdname(lip) ;
}
/* end subroutine (locinfo_jobdname) */
#endif /* CF_LOCJOBDNAME */


static int locinfo_mdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	dm = TMPDMODE ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip->mdname == NULL) {
	    cchar	*pn = pip->progname ;
	    char	mdname[MAXPATHLEN + 1] ;
	    if ((rs = prmktmpdir(pip->pr,mdname,TMPDNAME,pn,dm)) >= 0) {
	        cchar	*cp ;
	        len = rs ;
	        if ((rs = uc_mallocstrw(mdname,len,&cp)) >= 0) {
	            lip->mdname = cp ;
	        } else {
	            lip->mdname = NULL ;
		}
	    }
	} else {
	    len = strlen(lip->mdname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/locinfo_mdname: ret rs=%d len=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mdname) */


static int locinfo_loadids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (lip->gnbuf[0] == '\0') {
	    cchar	*un = lip->un ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_motd/locinfo_loadids: un=%s\n",lip->un) ;
#endif

	    if ((un == NULL) || (un[0] == '\0') || (un[0] == '-')) {
		if ((rs = locinfo_username(lip)) >= 0) {
	            lip->un = lip->unbuf ;
	            lip->uid_dirs = lip->uid ;
	            lip->gid_dirs = lip->gid ;
		}
	    } else {
	        struct passwd	pw ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
		if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,lip->un)) >= 0) {
	                lip->uid_dirs = pw.pw_uid ;
	                lip->gid_dirs = pw.pw_gid ;
		    }
		    uc_free(pwbuf) ;
		} /* end if (m-a) */
	    } /* end if */

	    if (rs >= 0) {
		const gid_t	gid = lip->gid_dirs ;
	        const int	gnlen = GROUPNAMELEN ;
	        rs = getgroupname(lip->gnbuf,gnlen,gid) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("b_motd/locinfo_loadids: tar uid=%d\n",
	            lip->uid_dirs) ;
	        debugprintf("b_motd/locinfo_loadids: tar gid=%d\n",
	            lip->gid_dirs) ;
	    }
#endif

	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_motd/locinfo_loadids: ret rs=%d gn=%s\n",
	        rs,lip->gnbuf) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_loadids) */


/* this runs as an independent thread */
static int locinfo_tmpcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->jobdname != NULL) {
	    TMTIME	t ;
	    if ((rs = tmtime_localtime(&t,pip->daytime)) >= 0) {
	        if ((t.hour >= HOUR_MAINT) || lip->f.maint) {
		    uptsub_t	thr = (uptsub_t) locinfo_tmpmaint ;
	            pthread_t	tid ;
	            if ((rs = uptcreate(&tid,NULL,thr,lip)) >= 0) {
	                rs = 1 ;
	                lip->tid = tid ;
	                lip->f.tmpmaint = TRUE ;
	            } /* end if (uptcreate) */
	        } /* end if (after hours) */
	    } /* end if (tmtime_localtime) */
	} /* end if (job-dname) */

	return rs ;
}
/* end subroutine (locinfo_tmpcheck) */


/* this runs as an independent thread */
static int locinfo_tmpmaint(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = TO_TMPFILES ;
	int		rs ;
	int		c = 0 ;
	int		f_need = lip->f.maint ;
	cchar		*dname = lip->jobdname ;
	char		tsfname[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tsfname,dname,TSFNAME)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = (O_WRONLY|O_CREAT) ;
	    if ((rs = u_open(tsfname,of,om)) >= 0) {
	        struct ustat	usb ;
	        const int	fd = rs ;
	        if ((rs = u_fstat(fd,&usb)) >= 0) {
	            time_t	dt = pip->daytime ;
	            if ((rs = locinfo_fchmodown(lip,fd,&usb,om)) >= 0) {
	                int	maintlapse = (dt - usb.st_mtime) ;
	                f_need = f_need || (usb.st_size == 0) ;
	                f_need = f_need || (maintlapse >= to) ;
	                if (f_need) {
	                    int		tl ;
	                    char	timebuf[TIMEBUFLEN + 3] ;
	                    timestr_log(dt,timebuf) ;
	                    tl = strlen(timebuf) ;
	                    timebuf[tl++] = '\n' ;
	                    rs = u_write(fd,timebuf,tl) ;
	                } /* end if (timed-out) */
	            } /* end if (locinfo_fchmodown) */
	        } /* end if (stat) */
	        u_close(fd) ;
	    } /* end if (open file) */
	} /* end if (mkpath timestamp) */

	if ((rs >= 0) && f_need) {
	    rs = rmdirfiles(dname,NULL,to) ;
	    c = rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_tmpmaint) */


static int locinfo_tmpdone(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->f.tmpmaint) {
	    int	trs ;
	    rs1 = uptjoin(lip->tid,&trs) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs >= 0) rs = trs ;
	}
	return rs ;
}
/* end subroutine (locinfo_tmpdone) */


static int locinfo_fchmodown(LOCINFO *lip,int fd,struct ustat *sbp,mode_t mm)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if ((sbp->st_size == 0) && (pip->euid == sbp->st_uid)) {
	    if ((sbp->st_mode & S_IAMB) != mm) {
	        if ((rs = locinfo_loadprids(lip)) >= 0) {
	            if ((rs = uc_fminmod(fd,mm)) >= 0) {
	                const uid_t	uid_pr = lip->uid_pr ;
	                const gid_t	gid_pr = lip->gid_pr ;
	                const int	n = _PC_CHOWN_RESTRICTED ;
	                if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
	                    f = TRUE ;
	                    u_fchown(fd,uid_pr,gid_pr) ; /* may fail */
	                } else if (rs == SR_NOSYS) {
			    rs = SR_OK ;
	                }
	            }
	        } /* end if (locinfo_loadprids) */
	    } /* end if (need change) */
	} /* end if (zero-file) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_fchmodown) */


static int locinfo_getgid(LOCINFO *lip)
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

	        if (lip->uid == lip->euid) { /* like we are a SHELL builtin */
	            if ((rs = proginfo_rootname(pip)) >= 0) {
	                const int	nrs = SR_NOTFOUND ;
		        cchar		*rn = pip->rootname ;
	                if ((rs1 = GETPW_NAME(&pw,pwbuf,pwlen,rn)) == nrs) {
	                    char	ubuf[USERNAMELEN + 1] ;
	                    strwcpylc(ubuf,VARPRNAME,USERNAMELEN) ;
	                    if (strcmp(pip->rootname,ubuf) != 0) {
	                        rs1 = GETPW_NAME(&pw,pwbuf,pwlen,ubuf) ;
	                    } /* end if (trying a different username) */
	                } /* end if (getpw_name) */
	                if (rs1 >= 0) {
	                    f_got = TRUE ;
	                    lip->uid_prog = pw.pw_uid ;
	                    lip->gid_prog = pw.pw_gid ;
	                }
	            } /* end if (rootname) */
	        } /* end if */

	        if ((rs >= 0) && (! f_got)) {
	            if ((rs = GETPW_UID(&pw,pwbuf,pwlen,lip->euid)) >= 0) {
	                f_got = TRUE ;
	                lip->uid_prog = pw.pw_uid ;
	                lip->gid_prog = pw.pw_gid ;
	            }
	        } /* end if */

		uc_free(pwbuf) ;
	    } /* end if (m-a) */
	} /* end if (needed) */

	return (rs >= 0) ? f_got : rs ;
}
/* end subroutine (locinfo_getgid) */


static int locinfo_chgrp(LOCINFO *lip,cchar fname[])
{
	int		rs ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = locinfo_getgid(lip)) >= 0) {
	    USTAT	usb ;
	    uid_t	cuid = -1 ;
	    if (lip->euid != lip->uid_prog) {
	        cuid = lip->uid_prog ;
	    }
	    if ((rs = u_stat(fname,&usb)) >= 0) {
	        if (usb.st_gid != lip->gid_prog) {
	            u_chown(fname,cuid,lip->gid_prog) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_chgrp) */


static int locinfo_termoutbegin(LOCINFO *lip,void *ofp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_termout = FALSE ;
	cchar		*tstr = lip->termtype ;
	cchar		*vp ;

	if (lip->f.termout || ((rs = shio_isterm(ofp)) > 0)) {
	    int	ncols = COLUMNS ;
	    if ((vp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	        int	v ;
	        rs1 = cfdeci(vp,-1,&v) ;
	        if (rs1 >= 0) ncols = v ;
	    }
	    if (rs >= 0) {
	        rs = termout_start(&lip->outer,tstr,-1,ncols) ;
	        lip->open.outer = (rs >= 0) ;
	    }
	} /* end if */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    f_termout = lip->open.outer ;
	    shio_printf(pip->efp,"%s: termout=%u\n",pn,f_termout) ;
	    if (f_termout)
	        shio_printf(pip->efp,"%s: termtype=%s\n",pn,tstr) ;
	}

	return (rs >= 0) ? f_termout : rs ;
}
/* end subroutine (locinfo_termoutbegin) */


static int locinfo_termoutend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.outer) {
	    lip->open.outer = FALSE ;
	    rs1 = termout_finish(&lip->outer) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_termoutend) */


static int locinfo_termoutprint(LOCINFO *lip,void *ofp,cchar lbuf[],int llen)
{
	PROGINFO	*pip = lip->pip ;
	TERMOUT		*top = &lip->outer ;
	int		rs ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (llen > 0) {
	    if ((rs = termout_load(top,lbuf,llen)) >= 0) {
	        int	ln = rs ;
	        int	i ;
	        int	ll ;
	        cchar	*lp ;
	        for (i = 0 ; i < ln ; i += 1) {
	            ll = termout_getline(top,i,&lp) ;
	            if (ll < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("b_motd/locinfo_termoutprint: ll=%u\n",
	                    ll) ;
	                debugprintf("b_motd/locinfo_termoutprint: l=>%t<\n",
	                    lp,strlinelen(lp,ll,40)) ;
	            }
#endif

	            rs = shio_print(ofp,lp,ll) ;
	            wlen += rs ;

	            if (rs < 0) break ;
	        } /* end for */
	        if ((rs >= 0) && (ll != SR_NOTFOUND)) rs = ll ;
	    } /* end if (termoutprint) */
	} else {
	    rs = shio_print(ofp,lbuf,llen) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (locinfo_termoutprint) */


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


int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char ubuf[],int ulen)
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
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.rn) {
	    rs = sysrealname_open(&lip->rn,NULL) ;
	    lip->open.rn = (rs >= 0) ;
	}

	if (rs >= 0)
	    rs = sysrealname_curbegin(&lip->rn,&curp->rncur) ;

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

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

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


static int locinfo_loadprids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->uid_pr < 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	        lip->uid_pr = sb.st_uid ;
	        lip->gid_pr = sb.st_gid ;
	    } /* end if (u_stat) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_loadprids) */


static int locinfo_username(LOCINFO *lip)
{
	int		rs ;
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


