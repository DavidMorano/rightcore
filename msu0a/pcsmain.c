/* pcs-main */

/* Personal Communication Services (PCS) server for PCS requests */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

	= 2005-04-20, David A­D­ Morano
	I changed the program so that the configuration file is consulted even
	if the program is not run in daemon-mode.  

	= 2011-01-25, David A­D­ Morano
	Code was removed and placed in other files (so that they can be
	compiled differently) due to AST-code conflicts over the system
	socket-library structure definitions.

*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a PCS program
	out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child processes to exit explicitly.  Child processes are only
	created when run in "daemon" mode.

	Synopsis:

	$ pcs [-speed[=<name>]] [-zerospeed] [-db <file>]


	Implementation note:

	It is difficult to close files when run as a SHELL builtin!  We want to
	close files when we run in the background, but when running as a SHELL
	builtin, we cannot close file descriptors untill after we fork (else we
	corrupt the enclosing SHELL).  However, we want to keep the files
	associated with persistent objects open across the fork.  This problem
	is under review.  Currently, there is not an adequate self-contained
	solution.


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
#include	<sys/uio.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<poll.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<ascii.h>
#include	<toxc.h>
#include	<spawner.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"proguserlist.h"
#include	"pcsmain.h"
#include	"pcsconfig.h"
#include	"pcslocinfo.h"
#include	"pcsadj.h"
#include	"pcscmd.h"
#include	"pcslog.h"
#include	"defs.h"
#include	"msflag.h"


/* local typedefs */


/* local defines */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	2048
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	DEBUGFNAME	"/tmp/pcs.deb"

#ifndef	REQCNAME
#define	REQCNAME	"req"
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mknpath2(char *,int,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	securefile(cchar *,uid_t,gid_t) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	getnprocessors(cchar **,int) ;
extern int	prgetprogpath(cchar *,char *,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
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
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	pcsdebug_lockprint(PROGINFO *,cchar *) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */


/* forward references */

static int	pcsmain(int,cchar *[],cchar *[],void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procdefargs(PROGINFO *) ;
static int	procfindconf(PROGINFO *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;
static int	procuserinfo_hostname(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *) ;
static int	procourconf_end(PROGINFO *) ;

static int	procourdefs(PROGINFO *) ;

static int	process(PROGINFO *,cchar *) ;
static int	procourcmds(PROGINFO *,cchar *) ;
static int	procregular(PROGINFO *) ;
static int	procbackinfo(PROGINFO *) ;
static int	procback(PROGINFO *) ;
static int	procbacks(PROGINFO *) ;
static int	procbackcheck(PROGINFO *) ;
static int	procbacker(PROGINFO *,cchar *,cchar **) ;
static int	procbackenv(PROGINFO *,SPAWNER *) ;
static int	procmntcheck(PROGINFO *) ;
static int	procdaemon(PROGINFO *) ;
static int	procdaemoncheck(PROGINFO *) ;

static int	procbackdefs(PROGINFO *) ;
static int	procdaemondefs(PROGINFO *) ;
static int	procpidfname(PROGINFO *) ;
static int	procservice(PROGINFO *) ;
static int	procfcmd(PROGINFO *) ;
static int	procexecname(PROGINFO *,char *,int) ;


/* local variables */

static cconst char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"PID",
	"pid",
	"REQ",
	"req",
	"sn",
	"af",
	"ef",
	"of",
	"cf",
	"db",
	"msfile",
	"mspoll",
	"speed",
	"speedint",
	"intconf",
	"intspeed",
	"zerospeed",
	"caf",
	"disable",
	"ra",
	"fg",
	"daemon",
	"cmd",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_pid0,
	argopt_pid1,
	argopt_req0,
	argopt_req1,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cf,
	argopt_db,
	argopt_msfile,
	argopt_mspoll,
	argopt_speed,
	argopt_speedint,
	argopt_intconf,
	argopt_intspeed,
	argopt_zerospeed,
	argopt_caf,
	argopt_disable,
	argopt_ra,
	argopt_fg,
	argopt_daemon,
	argopt_cmd,
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
	{ SR_NOMEM, EX_OSERR },
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_ALREADY, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"lockinfo",
	"quiet",
	"intrun",
	"intidle",
	"intpoll",
	"intconf",
	"intsvcs",
	"intwait",
	"intcache",
	"intmaint",
	"intspeed",
	"intdirmaint",
	"intclient",
	"quick",
	"listen",
	"ra",
	"reuse",
	"daemon",
	"pidfile",
	"reqfile",
	"mntfile",
	"msfile",
	"maint",
	NULL
} ;

enum progopts {
	progopt_lockinfo,
	progopt_quiet,
	progopt_intrun,
	progopt_intidle,
	progopt_intpoll,
	progopt_intconf,
	progopt_intsvcs,
	progopt_intwait,
	progopt_intcache,
	progopt_intmaint,
	progopt_intspeed,
	progopt_intdirmaint,
	progopt_intclient,
	progopt_quick,
	progopt_listen,
	progopt_ra,
	progopt_reuse,
	progopt_daemon,
	progopt_pidfile,
	progopt_reqfile,
	progopt_mntfile,
	progopt_msfile,
	progopt_maint,
	progopt_overlast
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*cmds[] = {
	"exit",
	"mark",
	"report",
	NULL
} ;

enum cmds {
	cmd_exit,
	cmd_mark,
	cmd_report,
	cmd_overlast
} ;

/* thses are for the spawned child */
static const int	sigignores[] = {
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
	0
} ;


/* exported subroutines */


int b_pcs(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = pcsmain(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_pcs) */


int p_pcs(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return pcsmain(argc,argv,envv,contextp) ;
}
/* end subroutine (p_pcs) */


/* local subroutines */


/* ARGSUSED */
static int pcsmain(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
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
	int		vl ;
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
	cchar		*vp ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("pcsmain: starting DFD=%d\n",rs) ;
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

	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

#if	CF_DEBUGS
	debugprintf("pcsmain: a[%u]=%s\n",ai,argp) ;
#endif

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

	                vp = NULL ;
	                vl = 0 ;
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
	                    pip->have.lfname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.lfname = TRUE ;
	                            pip->have.lfname = TRUE ;
	                            pip->lfname = avp ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* PID file */
	                case argopt_pid0:
	                case argopt_pid1:
			    vp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            vp = avp ;
				}
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                vp = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (vp != NULL)) {
	                        pip->final.pidfname = TRUE ;
	                        pip->have.pidfname = TRUE ;
	                        pip->pidfname = vp ;
	                    }
	                    break ;

/* REQ file */
	                case argopt_req0:
	                case argopt_req1:
			    vp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            vp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                vp = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (vp != NULL)) {
	                        lip->final.reqfname = TRUE ;
	                        lip->have.reqfname = TRUE ;
	                        lip->reqfname = vp ;
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

/* argument-list file */
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

/* conf-filename */
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* MS file name */
	                case argopt_db:
	                case argopt_msfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            lip->final.msfname = TRUE ;
	                            lip->have.msfname = TRUE ;
	                            lip->msfname = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->final.msfname = TRUE ;
	                                lip->have.msfname = TRUE ;
	                                lip->msfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* MS poll interval */
	                case argopt_mspoll:
			    vp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            vp = avp ;
	                            vl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                vp = argp ;
	                                vl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (vp != NULL) && (vl > 0)) {
	                        pip->final.intpoll = TRUE ;
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->intpoll = v ;
	                    }
	                    break ;

	                case argopt_speed:
	                    lip->f.speedname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->speedname = avp ;
	                    }
	                    break ;

	                case argopt_speedint:
	                case argopt_intspeed:
			    vp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            vp = avp ;
	                            vl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                vp = argp ;
	                                vl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (vp != NULL) && (vl > 0)) {
	                        lip->final.intspeed = TRUE ;
	                        rs = cfdecti(vp,vl,&v) ;
	                        lip->intspeed = v ;
	                    }
	                    break ;

	                case argopt_intconf:
			    vp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            vp = avp ;
	                            vl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                vp = argp ;
	                                vl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (vp != NULL) && (vl > 0)) {
	                        lip->final.intconf = TRUE ;
	                        rs = cfdecti(vp,vl,&v) ;
	                        lip->intconf = v ;
	                    }
	                    break ;

	                case argopt_zerospeed:
	                    lip->f.zerospeed = TRUE ;
	                    break ;

/* disable interval */
	                case argopt_disable:
	                    pip->f.disable = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.intdis = TRUE ;
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intdis = v ;
	                        }
	                    }
	                    break ;

/* reuse address */
	                case argopt_ra:
	                    lip->have.reuseaddr = TRUE ;
	                    lip->final.reuseaddr = TRUE ;
	                    lip->f.reuseaddr = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* run in the foreground */
	                case argopt_fg:
	                    lip->have.fg = TRUE ;
	                    lip->final.fg = TRUE ;
	                    lip->f.fg = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.fg = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* daemon mode */
	                case argopt_daemon:
	                    pip->have.daemon = TRUE ;
	                    pip->final.daemon = TRUE ;
	                    pip->f.daemon = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intrun = v ;
	                        }
	                    }
	                    break ;

	                case argopt_cmd:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_cmdsload(lip,argp,argl) ;
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
	                    const int kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                pip->cfname = argp ;
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

/* pid-file */
	                    case 'P':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->final.pidfname = TRUE ;
	                                pip->have.pidfname = TRUE ;
	                                pip->pidfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        pip->have.quiet = TRUE ;
	                        pip->final.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.quiet = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* number-updates */
	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecmfi(argp,argl,&v) ;
	                                lip->nu = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* daemon mode */
	                    case 'd':
	                        pip->final.background = TRUE ;
	                        pip->f.background = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                int	ch = MKCHAR(*avp) ;
	                                if (isdigitlatin(ch)) {
	                                    pip->final.intrun = TRUE ;
	                                    pip->have.intrun = TRUE ;
	                                    rs = cfdecti(avp,avl,&v) ;
	                                    pip->intrun = v ;
	                                } else if (tolc(ch) == 'i') {
	                                    pip->intrun = 0 ;
	                                } else
	                                    rs = SR_INVALID ;
	                            }
	                        }
	                        break ;

			    case 'm':
				lip->final.maint = TRUE ;
				lip->f.maint = TRUE ;
				break ;

/* MS-node */
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->msnode = argp ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->verboselevel = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* reuse listener port */
	                    case 'r':
	                        lip->have.reuseaddr = TRUE ;
	                        lip->final.reuseaddr = TRUE ;
	                        lip->f.reuseaddr = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.reuseaddr = (rs > 0) ;
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
	    debugprintf("pcsmain: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* continue with prepatory initialization */

#if	CF_DEBUGS
	debugprintf("pcsmain: pid=%d\n",pip->pid) ;
#endif

	if ((ai_pos < 0) || (ai_max < 0)) { /* lint */
	    rs = SR_BUGCHECK ;
	}

	rs1 = securefile(pip->pr,pip->euid,pip->egid) ;
	lip->f.sec_root = (rs1 > 0) ;

	if ((rs >= 0) && (pip->intpoll == 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&v) ;
	    pip->intpoll = v ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* defaults */

	if (pip->cfname == NULL) pip->cfname = getourenv(envv,VARCFNAME) ;
	if (pip->cfname == NULL) pip->cfname = getourenv(envv,VARCONFIG) ;

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLFNAME) ;

	if (lip->msnode == NULL) lip->msnode = getourenv(envv,VARMSNODE) ;

/* go */

	if (rs >= 0) {
	    if ((rs = procdefargs(pip)) >= 0) {
		USERINFO	u ;
		if ((rs = userinfo_start(&u,NULL)) >= 0) {
	            if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	                    if ((rs = procourconf_begin(pip)) >= 0) {
	                        if ((rs = procourdefs(pip)) >= 0) {
	                            if ((rs = logbegin(pip,&u)) >= 0) {
	                                {
					    cchar	*ofn = ofname ;
	                                    rs = process(pip,ofn) ;
	                                }
	                                rs1 = logend(pip) ;
	                                if (rs >= 0) rs = rs1 ;
	                            } /* end if (log) */
	                        } /* end if (procourdefs) */
	                        rs1 = procourconf_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (procourconf) */
	                rs1 = procuserinfo_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procuserinfo) */
		    rs1 = userinfo_finish(&u) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (userinfo) */
	    } /* end if (procdefargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: usage (%d)\n",pn,rs) ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("pcsmain: done ex=%u (%d)\n",ex,rs) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    char	timebuf[TIMEBUFLEN+1] ;
	    timestr_logz(pip->daytime,timebuf) ;
	    switch (rs) {
	    case SR_ALREADY:
	    case SR_AGAIN:
	        ex = EX_MUTEX ;
	        if ((! pip->f.quiet) && (pip->efp != NULL)) {
	            fmt = "%s: existing lock (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        if ((! pip->f.quiet) && (pip->efp != NULL)) {
	            fmt = "%s: abnormal exit (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	        break ;
	    } /* end switch */
	} else if ((rs = lib_sigterm()) < 0) {
	    ex = EX_TERM ;
	} else if ((rs = lib_sigintr()) < 0) {
	    ex = EX_INTR ;
	}

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (pip->f.background || pip->f.daemon) {
	        cchar	*w = ((pip->f.daemon) ? "child" : "parent") ;
		fmt = "%s: (%s) exiting ex=%u (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,w,ex,rs) ;
	    } else {
		fmt = "%s: exiting ex=%u (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,ex,rs) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("pcsmain: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if ((pip->efp != NULL) && pip->open.errfile) {
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
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("pcsmain: final mallout=%u\n",mdiff) ;
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
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
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
/* end subroutine (pcsmain) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-d[=<intrun>]] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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

#if	CF_DEBUG
		if (DEBUGLEVEL(3))
			debugprintf("b_pcs/procopt: k=%t\n",kp,kl) ;
#endif

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {
	                int	v = 0 ;

	                vl = keyopt_enumvalues(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_quiet:
	                    if (! pip->final.quiet) {
	                        c += 1 ;
	                        pip->final.quiet = TRUE ;
	                        pip->have.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_intrun:
	                case progopt_intidle:
	                case progopt_intpoll:
	                case progopt_intconf:
	                case progopt_intsvcs:
	                case progopt_intcache:
	                case progopt_intwait:
	                case progopt_intmaint:
	                case progopt_intspeed:
	                case progopt_intdirmaint:
	                case progopt_intclient:
	                    {
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                        }
	                        if ((rs >= 0) && (vl > 0)) {
	                            switch(oi) {
	                            case progopt_intrun:
	                                if (! pip->final.intrun) {
	                                    c += 1 ;
	                                    pip->final.intrun = TRUE ;
	                                    pip->have.intrun = TRUE ;
	                                    pip->intrun = v ;
	                                }
	                                break ;
	                            case progopt_intidle:
	                                if (! pip->final.intidle) {
	                                    c += 1 ;
	                                    pip->final.intidle = TRUE ;
	                                    pip->have.intidle = TRUE ;
	                                    pip->intidle = v ;
	                                }
	                                break ;
	                            case progopt_intpoll:
	                                if (! pip->final.intpoll) {
	                                    c += 1 ;
	                                    pip->final.intpoll = TRUE ;
	                                    pip->have.intpoll = TRUE ;
	                                    pip->intpoll = v ;
	                                }
	                                break ;
	                            case progopt_intconf:
	                                if (! lip->final.intconf) {
	                                    c += 1 ;
	                                    lip->final.intconf = TRUE ;
	                                    lip->have.intconf = TRUE ;
	                                    lip->intconf = v ;
	                                }
	                                break ;
	                            case progopt_intsvcs:
	                                if (! lip->final.intsvcs) {
	                                    c += 1 ;
	                                    lip->final.intsvcs = TRUE ;
	                                    lip->have.intsvcs = TRUE ;
	                                    lip->intsvcs = v ;
	                                }
	                                break ;
	                            case progopt_intcache:
	                                if (! lip->final.intcache) {
	                                    c += 1 ;
	                                    lip->final.intcache = TRUE ;
	                                    lip->have.intcache = TRUE ;
	                                    lip->intcache = v ;
	                                }
	                                break ;
	                            case progopt_intwait:
	                                if (! lip->final.intwait) {
	                                    c += 1 ;
	                                    lip->final.intwait = TRUE ;
	                                    lip->have.intwait = TRUE ;
	                                    lip->intwait = v ;
	                                }
	                                break ;
	                            case progopt_intmaint:
	                                if (! lip->final.intmaint) {
	                                    c += 1 ;
	                                    lip->final.intmaint = TRUE ;
	                                    lip->have.intmaint = TRUE ;
	                                    lip->intmaint = v ;
	                                }
	                                break ;
	                case progopt_intspeed:
	                    if (! lip->final.intspeed) {
	                        c += 1 ;
	                        lip->final.intspeed = TRUE ;
	                        lip->have.intspeed = TRUE ;
	                        lip->intspeed = v ;
	                    }
	                    break ;
	                case progopt_intdirmaint:
	                    if (! lip->final.intdirmaint) {
	                        c += 1 ;
	                        lip->final.intdirmaint = TRUE ;
	                        lip->have.intdirmaint = TRUE ;
	                        lip->intdirmaint = v ;
	                    }
	                    break ;
	                case progopt_intclient:
	                    if (! lip->final.intclient) {
	                        c += 1 ;
	                        lip->final.intclient = TRUE ;
	                        lip->have.intclient = TRUE ;
	                        lip->intclient = v ;
	                    }
	                    break ;
	                            } /* end switch */
	                        } /* end if (have) */
	                    } /* end block */
	                    break ;
	                case progopt_lockinfo:
	                    if (! lip->final.lockinfo) {
	                        c += 1 ;
	                        lip->final.lockinfo = TRUE ;
	                        lip->have.lockinfo = TRUE ;
	                        lip->f.lockinfo = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.lockinfo = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_quick:
	                    if (! lip->final.quick) {
	                        c += 1 ;
	                        lip->final.quick = TRUE ;
	                        lip->have.quick = TRUE ;
	                        lip->f.quick = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.quick = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_listen:
	                    if (! lip->final.adj) {
	                        c += 1 ;
	                        lip->final.adj = TRUE ;
	                        lip->have.adj = TRUE ;
	                        lip->f.adj = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.adj = (rs > 0) ;
	                        }
	                    } /* end if */
	                    break ;
	                case progopt_ra:
	                case progopt_reuse:
	                    if (! lip->final.reuseaddr) {
	                        c += 1 ;
	                        lip->final.reuseaddr = TRUE ;
	                        lip->have.reuseaddr = TRUE ;
	                        lip->f.reuseaddr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.reuseaddr = (rs > 0) ;
	                        }
	                    } /* end if */
	                    break ;
	                case progopt_daemon:
	                    if (! pip->final.daemon) {
	                        c += 1 ;
	                        pip->final.daemon = TRUE ;
	                        pip->have.daemon = TRUE ;
	                        pip->f.daemon = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->intrun = v ;
	                            if (v == 0) {
	                                pip->f.daemon = FALSE ;
	                            }
	                        }
	                    } /* end if */
	                    break ;
	                case progopt_pidfile:
	                    if (! pip->final.pidfname) {
	                        if (vl > 0) {
	                            cchar **vpp = &pip->pidfname ;
	                            pip->have.pidfname = TRUE ;
	                            pip->final.pidfname = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case progopt_reqfile:
	                    if (! lip->final.reqfname) {
	                        if (vl > 0) {
	                            cchar **vpp = &lip->reqfname ;
	                            lip->have.reqfname = TRUE ;
	                            lip->final.reqfname = TRUE ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case progopt_mntfile:
	                    if (! lip->final.mntfname) {
	                        if (vl > 0) {
	                            cchar **vpp = &lip->mntfname ;
	                            lip->have.mntfname = TRUE ;
	                            lip->final.mntfname = TRUE ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case progopt_msfile:
	                    if (! lip->final.msfname) {
	                        if (vl > 0) {
	                            cchar **vpp = &lip->msfname ;
	                            lip->have.msfname = TRUE ;
	                            lip->final.msfname = TRUE ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case progopt_maint:
	                    if (! lip->final.maint) {
	                            lip->final.maint = TRUE ;
	                            lip->f.maint = TRUE ;
	                        if ((vl > 0) && (! lip->final.intclient)) {
	                            rs = cfdecti(vp,vl,&v) ;
				    lip->intclient = v ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	            } else
			rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("b_pcs/procopt: intrun=%u\n",pip->intrun) ;
	debugprintf("b_pcs/procopt: rs=%d c=%u\n",rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procdefargs(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		**envv = pip->envv ;
	cchar		*cp ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if ((rs >= 0) && (! pip->final.quiet)) {
	    if ((cp = getourenv(envv,VARQUIET)) != NULL) {
	        rs = optbool(cp,-1) ;
	        pip->final.quiet = TRUE ;
	        pip->have.quiet = TRUE ;
	        pip->f.quiet = (rs > 0) ;
	    }
	}

	return rs ;
}
/* end subroutine (procdefargs) */


static int procfindconf(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (pip->cfname == NULL) {
	    vecstr	sv ;
	    int		rs1 ;
	    cchar	*cfn = CONFIGFNAME ;
	    if ((rs = vecstr_start(&sv,4,0)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        int		i ;
	        int		vl ;
	        cchar		keys[] = "pen" ;
	        cchar		*vp ;
	        char		ks[2] = { 0, 0 } ;
	        char		tbuf[MAXPATHLEN+1] ;
	        for (i = 0 ; keys[i] != '\0' ; i += 1) {
	            const int	kch = MKCHAR(keys[i]) ;
	            vp = NULL ;
	            vl = -1 ;
	            switch (kch) {
	            case 'p':
	                vp = pip->pr ;
	                break ;
	            case 'e':
	                vp = "etc" ;
	                break ;
	            case 'n':
	                vp = pip->searchname ;
	                break ;
	            } /* end switch */
	            if ((rs >= 0) && (vp != NULL)) {
	                ks[0] = kch ;
	                rs = vecstr_envadd(&sv,ks,vp,vl) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	        if (rs >= 0) {
	            if ((rs = permsched(sched1,&sv,tbuf,tlen,cfn,R_OK)) >= 0) {
	                cchar	**vpp = &pip->cfname ;
	                rl = rs ;
	                rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
		    }
	        }
	        rs1 = vecstr_finish(&sv) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	} /* end if (specified) */

	if ((pip->debuglevel > 0) && (pip->cfname != NULL)) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: conf=%s\n",pn,pip->cfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procfindconf: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (procfindconf) */


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

	if ((rs = proginfo_rootname(pip)) >= 0) {
	    if ((rs = procuserinfo_hostname(pip)) >= 0) {
	        rs = procuserinfo_logid(pip) ;
	    }
	} /* end if (ok) */

#ifdef	COMMENT
	if (rs >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    rs = locinfo_pcspr(lip) ;
	}
#endif /* COMMENT */

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


static int procuserinfo_hostname(PROGINFO *pip)
{
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs ;
	char		hbuf[MAXHOSTNAMELEN+1] ;
	cchar		*nn = pip->nodename ;
	cchar		*dn = pip->domainname ;
	if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	    cchar	**vpp = &pip->hostname ;
	    rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (procuserinfo_hostname) */


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


static int procourconf_begin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procourconf_begin: ent\n") ;
#endif

	if ((rs = procfindconf(pip)) > 0) {
	    const int	size = sizeof(CONFIG) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        const int	ic = lip->intconf ;
	        cchar		*cfname = pip->cfname ;
	        pip->config = p ;
	        if ((rs = config_start(pip->config,pip,cfname,ic)) >= 0) {
	            pip->open.config = TRUE ;
	        } /* end if (config_start) */
	        if (rs < 0) {
	            uc_free(pip->config) ;
	            pip->config = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (procfindconf) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->config != NULL) {
	    if (pip->open.config) {
	        pip->open.config = FALSE ;
	        rs1 = config_finish(pip->config) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procourconf_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_end) */


static int procourdefs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	cchar		**envv = pip->envv ;

	if (pip->logsize == 0) pip->logsize = LOGSIZE ;

	if (pip->intpoll == 0) pip->intpoll = TO_POLL ;

	if (lip->msfname == NULL) {
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARMSFNAME)) != NULL) {
	        lip->final.msfname = TRUE ;
	        lip->have.msfname = TRUE ;
	        lip->msfname = cp ;
	    }
	}

	if (rs >= 0) {
	    rs = locinfo_defs(lip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procourdefs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourdefs) */


static int procbackdefs(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->pidfname == NULL) {
	    cchar	**envv = pip->envv ;
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARPIDFNAME)) != NULL) {
	        pip->final.pidfname = TRUE ;
	        pip->have.pidfname = TRUE ;
	        pip->pidfname = cp ;
	    }
	}

	return rs ;
}
/* end subroutine (procbackdefs) */


static int procdaemondefs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		**envv = pip->envv ;
	cchar		*cp ;

	if (pip->pidfname == NULL) {
	    if ((cp = getourenv(envv,VARPIDFNAME)) != NULL) {
	        pip->final.pidfname = TRUE ;
	        pip->have.pidfname = TRUE ;
	        pip->pidfname = cp ;
	    }
	}

	if (lip->reqfname == NULL) {
	    if ((cp = getourenv(envv,VARREQFNAME)) != NULL) {
	        lip->final.reqfname = TRUE ;
	        lip->have.reqfname = TRUE ;
	        lip->reqfname = cp ;
	    }
	}

	if (pip->intpoll == 0) pip->intpoll = TO_POLL ;

	if (pip->intidle == 0) pip->intidle = TO_IDLE ;

	if (pip->intlock == 0) pip->intlock = TO_LOCK ;

	if (pip->intmark == 0) pip->intmark = TO_MARK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("pcsmain: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.logprog) ;
#endif
#if	CF_DEBUGS
	debugprintf("pcsmain: daemon=%u logging=%u\n",
	    pip->f.daemon,pip->have.logprog) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (pip->intpoll >= 0) {
	        fmt = "%s: mspoll=%u\n" ;
	    } else {
	        fmt = "%s: mspoll=Inf\n" ;
	    }
	    shio_printf(pip->efp,fmt,pn,pip->intpoll) ;
	    shio_printf(pip->efp,"%s: intspeed=%u\n",pn,lip->intspeed) ;
	}

	return rs ;
}
/* end subroutine (procdaemondefs) */


static int procpidfname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		pfl = -1 ;
	int		f_changed = FALSE ;
	cchar		*pfp ;
	char		rundname[MAXPATHLEN+1] ;
	char		cname[MAXNAMELEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procpidfname: ent\n") ;
#endif

	pfp = pip->pidfname ;
	if ((pfp != NULL) && (pfp[0] == '+')) {
	    cchar	*sn = pip->searchname ;

	    f_changed = TRUE ;
	    if ((rs = mkpath2(rundname,pip->pr,RUNDNAME)) >= 0) {
	        struct ustat	sb ;
	        const int	rsn = SR_NOENT ;
	        if ((rs = uc_stat(rundname,&sb)) >= 0) {
	            if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
	        } else if (rs == rsn) {
	            rs = mkdirs(rundname,0777) ;
	        }
	    } /* end if (mkpath) */

	    if (rs >= 0) {
	        cchar	*nn = pip->nodename ;
	        if ((rs = snsds(cname,MAXNAMELEN,nn,sn)) >= 0) {
	            pfp = tmpfname ;
	            rs = mkpath2(tmpfname,rundname,cname) ;
	            pfl = rs ;
	        }
	    }

	} /* end if (creating a default PID file-name) */

	if ((rs >= 0) && (pfp != NULL) && (pfp[0] == '-')) {
	    pfp = NULL ;
	    pip->have.pidfname = FALSE ;
	    pip->f.pidfname = FALSE ;
	    f_changed = FALSE ;
	}

	if ((rs >= 0) && (pfp != NULL) && f_changed) {
	    cchar	**vpp = &pip->pidfname ;
	    rs = proginfo_setentry(pip,vpp,pfp,pfl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procpidfname: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (procpidfname) */


static int process(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/process: ent f_cmds=%u\n",lip->f.cmds) ;
#endif

	if ((rs = proguserlist_begin(pip)) >= 0) {
	    if ((rs = locinfo_msfile(lip)) >= 0) {
	        if (pip->f.background || pip->f.daemon) {
	            if ((rs = procbackdefs(pip)) >= 0) {
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
	            } /* end if (procbackdefs) */
		} else if (lip->f.cmds) {
		    rs = procourcmds(pip,ofn) ;
	        } else {
	            rs = procregular(pip) ;
	        }
	    } /* end if (locinfo_msfile) */
	    rs1 = proguserlist_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (proguserlist) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsmain/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


static int procbackinfo(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*mntfname = lip->mntfname ;

	    if (pip->pidfname != NULL) {
	        shio_printf(pip->efp,"%s: pid=%s\n",pn,pip->pidfname) ;
	    }

	    shio_printf(pip->efp,"%s: intpoll=%u\n",
	        pn,
	        pip->intpoll) ;

	    shio_printf(pip->efp,"%s: intmark=%u\n",
	        pn,
	        pip->intmark) ;

	    shio_printf(pip->efp,"%s: intrun=%u\n",
	        pn,
	        pip->intrun) ;

	    if (mntfname != NULL)
	        shio_printf(pip->efp,"%s: mntfile=%s\n",
	            pip->progname,mntfname) ;

	    shio_flush(pip->efp) ;
	} /* end if (debugging information) */

	if (rs >= 0) {
	    rs = loginfo(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("pcsmain/procbackinfo: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbackinfo) */


static int procback(PROGINFO *pip)
{
	int		rs ;

	if (pip->open.logprog) {
	    logprintf(pip,"mode=background") ;
	    logflush(pip) ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: mode=background\n",pip->progname) ;
	    shio_flush(pip->efp) ;
	}

	if ((rs = procbackcheck(pip)) >= 0) {
	    rs = procbacks(pip) ;
	}

	return rs ;
}
/* end subroutine (procback) */


static int procbackcheck(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsmain/procbackcheck: ent\n") ;
#endif

	if ((rs = locinfo_lockbegin(lip)) >= 0) {
	    {
	        rs = procmntcheck(pip) ;
	    }
	    rs1 = locinfo_lockend(lip) ;
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
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (lip->mntfname != NULL) {
	    struct ustat	usb ;
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
	} /* end if (mntfname) */
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
	    debugprintf("main/procbacks: ent\n") ;
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

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procbacks: prog=%t\n",ebuf,el) ;
#endif

	    if ((rs = prgetprogpath(pip->pr,pbuf,ebuf,el)) > 0) {
	        pf = pbuf ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procbacks: mid rs=%d\n",rs) ;
#endif

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
	    debugprintf("main/procbacks: ret rs=%d\n",rs) ;
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
	            cchar	*fmt ;
	            pid = rs ;
	            if (pip->open.logprog) {
	                fmt = "backgrounded (%u)" ;
	                logprintf(pip,fmt,pid) ;
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

	    for (i = 0 ; i < 6 ; i += 1) {
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
	            v = pip->intpoll ;
	            if (v > 0) np = "intpoll" ;
	            break ;
	        case 3:
	            v = lip->intconf ;
	            if (v > 0) np = "intconf" ;
	            break ;
	        case 4:
	            v = lip->intspeed ;
	            if (v > 0) np = "intspeed" ;
	            break ;
	        case 5:
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
	        for (i = 0 ; i < 4 ; i += 1) {
	            np = NULL ;
	            switch (i) {
	            case 0:
	                if (pip->pidfname != NULL) {
	                    np = "pidfile" ;
	                    vp = pip->pidfname ;
	                }
	                break ;
		    case 1:
	                if (lip->reqfname != NULL) {
	                    np = "reqfile" ;
	                    vp = lip->reqfname ;
	                }
	                break ;
	            case 2:
	                if (lip->mntfname != NULL) {
	                    np = "mntfile" ;
	                    vp = lip->mntfname ;
	                }
	                break ;
	            case 3:
	                if (lip->msfname != NULL) {
	                    np = "msfile" ;
	                    vp = lip->msfname ;
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
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procdaemon: ent\n") ;
#endif

	if (pip->open.logprog) {
	    logprintf(pip,"mode=daemon") ;
	}
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: mode=daemon\n",pn) ;
	}

	if ((rs = procdaemondefs(pip)) >= 0) {
	    if ((rs = procdaemoncheck(pip)) >= 0) {
	        LOCINFO		*lip = pip->lip ;
	        if ((rs = locinfo_defdaemon(lip)) >= 0) {
	            if ((rs = locinfo_lockbegin(lip)) >= 0) {
	                {
	                    if ((rs = procservice(pip)) >= 0) {
	                        c = rs ;
			    }
	                }
	                rs1 = locinfo_lockend(lip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (lock) */
	        } /* end if (locinfo_defdaemon) */
	    } /* end if (procdaemoncheck) */
	} /* end if (procdaemondefs) */

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: daemon exiting (%d)\n",pn,rs) ;
	}
	if (pip->open.logprog) {
	    char	timebuf[TIMEBUFLEN+1] ;
	    timestr_logz(pip->daytime,timebuf) ;
	    logprintf(pip,"%s exiting c=%u (%d)",timebuf,c,rs) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdaemon) */


static int procdaemoncheck(PROGINFO *pip)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsmain/procdaemoncheck: ent\n") ;
#endif

	rs = procmntcheck(pip) ;

	return rs ;
}
/* end subroutine (procdaemoncheck) */


static int procourcmds(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("procourcmds: ent\n") ;
#endif
	if ((rs = locinfo_reqfname(lip)) >= 0) {
	    rs = pcscmd(pip,ofn) ;
	} /* end if (locinfo_reqfname) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("procourcmds: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procourcmds) */


static int procregular(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("pcsmain/procregular: ent\n") ;
	    debugprintf("pcsmain/procregular: f_main=%u\n",lip->f.maint) ;
	    debugprintf("pcsmain/procregular: intmain=%u\n",lip->intmaint) ;
	    debugprintf("pcsmain/procregular: intclient=%u\n",lip->intclient) ;
	}
#endif

	if ((rs = locinfo_defreg(lip)) >= 0) {
	    if ((rs = locinfo_tmpourdir(lip)) >= 0) {
	        rs = locinfo_dirmaint(lip) ;
	        c = 1 ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsmain/procregular: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procregular) */


static int procservice(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	CONFIG		*cop = (CONFIG *) pip->config ;
	const int	to_poll = TO_POLL ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("pcsmain/procservice: ent msfname=%s\n",lip->msfname) ;
	    debugprintf("pcsmain/procservice: intrun=%d\n",pip->intrun) ;
	}
#endif

	if ((rs = pcsadj_begin(pip)) >= 0) {
	    struct pollfd	fds[2] ;
	    const int		mto = (to_poll*POLLMULT) ;
	    int			nfds = 0 ;
	    cchar		*pn = pip->progname ;
	    cchar		*fmt ;

	    fds[nfds].fd = lip->rfd ;
	    fds[nfds].events = (POLLIN | POLLPRI | POLLERR) ;
	    nfds += 1 ;

	    while (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) 
	    debugprintf("pcsmain/procservice: poll\n") ;
#endif

	        if ((rs = u_poll(fds,nfds,mto)) > 0) {
	            int	i ;
	            pip->daytime = time(NULL) ;
	            for (i = 0 ; (rs >= 0) && (i < nfds) ; i += 1) {
	                const int	pfd = fds[i].fd ;
	                const int	re = fds[i].revents ;
	                if ((pfd == lip->rfd) && (re != 0)) {
			    rs = pcsadj_req(pip,re) ;
			    c += rs ;
	                }
	            } /* end for */
	        } else {
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) 
	    debugprintf("pcsmain/procservice: poll() rs=%d\n",rs) ;
#endif
	            pip->daytime = time(NULL) ;
	            if (rs == SR_INTR) rs = SR_OK ;
	        }

	        if ((rs >= 0) && (pip->intrun > 0)) {
		    const int	intrun = pip->intrun ;
	            int	f = ((pip->daytime - lip->ti_start) >= intrun) ;
	            if (f) break ;
	        }

	        if ((rs >= 0) && pip->open.config && ((c & 3) == 0)) {
	            if ((rs = config_check(cop)) > 0) {
	                if (pip->open.logprog) {
	                    char	tbuf[TIMEBUFLEN + 1] ;
	                    timestr_logz(pip->daytime,tbuf) ;
	                    fmt = "%s re-configuration" ;
	                    logprintf(pip,fmt,tbuf) ;
	                }
	            } /* end if (config_check) */
	        }

		if ((rs >= 0) && ((c & 7) == 1)) {
		    rs = locinfo_lockcheck(lip) ;
		}

	        if ((rs >= 0) && pip->open.logprog && ((c & 7) == 1)) {
	            if ((pip->daytime - lip->ti_marklog) >= pip->intmark) {
	                int	rem = 0 ;
	                lip->ti_marklog = pip->daytime ;
	                if (pip->intrun > 0) {
	                    rem = (pip->intrun - (pip->daytime-lip->ti_start)) ;
	                }
	                logmark(pip,rem) ;
	            } /* end if */
	        } /* end if */

/* handle the special "exit" condition */

	        if ((rs >= 0) && (lip->cmd[0] != '\0')) {
	            rs = procfcmd(pip) ;
	            if (rs > 0) break ;
	        }

		if (rs >= 0) {
		    if ((rs = locinfo_isreqexit(lip)) > 0) break ;
		}

		if (rs >= 0) {
		    rs = locinfo_dirmaint(lip) ;
		}

	        if (rs >= 0) {
	            logflush(pip) ;
	        }

		if (rs >= 0) {
		    if ((rs = lib_issig(SIGTSTP)) > 0) {
			char	tbuf[TIMEBUFLEN+1] ;
			timestr_logz(pip->daytime,tbuf) ;
			fmt = "%s: %s command suspended\n" ;
			shio_printf(pip->efp,fmt,pn,tbuf) ;
			shio_flush(pip->efp) ;
	                rs = uc_raise(SIGSTOP) ;
			pip->daytime = time(NULL) ;
			timestr_logz(pip->daytime,tbuf) ;
			fmt = "%s: %s command resumed\n" ;
			shio_printf(pip->efp,fmt,pn,tbuf) ;
		    }
		}
#if	CF_DEBUG
		if (DEBUGLEVEL(4)) 
	    	debugprintf("pcsmain/procservice: while-bot rs=%d\n",rs) ;
#endif

		if (rs >= 0) rs = lib_sigquit() ;
		if (rs >= 0) rs = lib_sigterm() ;
		if (rs >= 0) rs = lib_sigintr() ;
	    } /* end while (daemon-loop) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("pcsmain/procservice: for-after rs=%d c=%d\n",
			rs,c) ;
#endif /* CF_DEBUG */

	    rs1 = pcsadj_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pcsadj) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("pcsmain/procservice: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procservice) */


static int procfcmd(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f_exit = FALSE ;

	if (lip->cmd[0] != '\0') {
	    const int	ci = matostr(cmds,3,lip->cmd,-1) ;

	    switch (ci) {
	    case cmd_exit:
	        f_exit = TRUE ;
	        break ;
	    case cmd_mark:
	        {
	            int	rem = 0 ;
	            if (pip->intrun > 0) {
	                rem = (pip->intrun - (pip->daytime-lip->ti_start)) ;
	            }
	            logmark(pip,rem) ;
	        }
	        break ;
	    case cmd_report:
	        logreport(pip) ;
	        break ;
	    default:
	        loginvalidcmd(pip,lip->cmd) ;
	        break ;
	    } /* end switch */
	    lip->cmd[0] = '\0' ;

	} /* end if (non-nul) */

	return (rs >= 0) ? f_exit : rs ;
}
/* end if (procfcmd) */


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


