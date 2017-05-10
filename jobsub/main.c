/* main */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* get the 'exec(2)' name */
#define	CF_SPERM	1		/* use 'sperm()' */
#define	CF_FINDMAILER	1		/* find a mailer program */


/* revision history:

	= 2001-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a built-in command to the KSH shell. It should also be able to
        be made into a stand-alone program without much (if almost any)
        difficulty, but I have not done that yet (we already have a MSU program
        out there).

        Note that special care needed to be taken with the child processes
        because we cannot let them ever return normally! They cannot return
        since they would be returning to a KSH program that thinks it is alive
        (!) and that geneally causes some sort of problem or another. That is
        just some weird thing asking for trouble. So we have to take care to
        force child processes to exit explicitly. Child processes are only
        created when run in "daemon" mode.

	Implementation note:

	It is difficult to close files when run as a SHELL builtin!
	We want to close files when we run in the background, but when
	running as a SHELL builtin, we cannot close file descriptors
	untill after we fork (else we corrupt the enclosing SHELL).
	However, we want to keep the files associated with persistent
	objects open across the fork.  This problem is under review.
	Currently, there is not an adequate self-contained solution.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<bfile.h>
#include	<paramfile.h>
#include	<logfile.h>
#include	<kinfo.h>
#include	<lfm.h>
#include	<spawnproc.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"shio.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif

#define	NENV		200


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	vstrkeycmp(void *,void *) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	getrootdname(char *,int,const char *,const char *) ;
extern int	batch(struct proginfo *,void *,vecstr *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_loga(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

#if	(! CF_SFIO)
extern char	**environ ;
#endif


/* local structures */

struct session {
	pid_t	sid ;
	char	termdev[MAXPATHLEN + 1] ;
} ;

struct errormap {
	int	rs, ex ;
} ;


/* forward references */

int	setfname(struct proginfo *,char *,const char *,int,
		int,const char *,const char *,const char *) ;
int	xfile(struct proginfo *,const char *) ;

static void	sighand_int(int) ;

static int	usage(struct proginfo *) ;
static int	serverstart(struct proginfo *) ;
static int	serverpid(struct proginfo *) ;
static int	prepenv(vechand *,vecstr *,const char *,const char *) ;


/* local variables */

static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	0
} ;

static const int	sigints[] = {
	SIGUSR1,
	SIGUSR2,
	SIGINT,
	SIGTERM,
	SIGPOLL,
	0
} ;

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"LOGFILE",
	"of",
	"servererr",
	"serverprog",
	"jobname",
	"db",
	"speed",
	"zerospeed",
	"caf",
	"disable",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_logfile,
	argopt_of,
	argopt_servererr,
	argopt_serverprog,
	argopt_jobname,
	argopt_db,
	argopt_speed,
	argopt_zerospeed,
	argopt_caf,
	argopt_disable,
	argopt_overlast
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"cmd",
	"loglen",
	"pidfile",
	"logfile",
	"runint",
	"pollint",
	"markint",
	"lockint",
	"speedint",
	NULL
} ;

enum params {
	param_cmd,
	param_loglen,
	param_pidfile,
	param_logfile,
	param_runint,
	param_pollint,
	param_markint,
	param_lockint,
	param_speedint,
	param_overlast
} ;

static const struct errormap	errormaps[] = {
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
	{ 0, 0 }
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	struct session	sinfo ;

	struct ustat	sb ;
	struct ustat	*sbp = &sb ;

	sigset_t	oldsigmask, newsigmask ;

	SHIO	errfile ;
	SHIO	outfile ;

	vecstr	flist ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1, n, i, j ;
	int	size ;
	int	sl, cl, ml, pl ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_child = FALSE ;
	int	f_caf = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	username[USERNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*mailaddr = NULL ;
	const char	*serverprog = NULL ;
	const char	*serverefname = NULL ;
	const char	*sp, *cp ;


	if_int = 0 ;

	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	memset(sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;

	uc_sigsetempty(&newsigmask) ;

/* ignore these signals */

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = SIG_IGN ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigignores[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */

/* interrupt on these signals */

	for (i = 0 ; sigints[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = sighand_int ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigints[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;
	if (cp != NULL) {
	    if (cfdeci(cp,-1,&fd_debug) >= 0) {
	        debugsetfd(fd_debug) ;
	    } else
	        fd_debug = debugopen(cp) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && 0
	for (i = 0 ; envv[i] != NULL ; i += 1)
	    debugprintf("main: envv[%u]=%s\n",i,envv[i]) ;
#endif

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	pip->verboselevel = 1 ;

	if (efname == NULL) efname = getenv(VARSTDERRFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

#if	CF_DEBUGS
	debugprintf("main: efname=%s shio_open() rs=%d\n",cp,rs) ;
#endif

/* initialize */

	memset(&sinfo,0,sizeof(struct session)) ;

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;

	pip->loglen = -1 ;
	pip->speedint = -1 ;
	pip->pollint = -1 ;
	pip->lockint = -1 ;
	pip->markint = -1 ;
	pip->runint = -1 ;
	pip->disint = -1 ;

	pip->f.quiet = FALSE ;
	pip->f.speed = FALSE ;
	pip->f.zerospeed = FALSE ;

	ids_load(&pip->ids) ;

/* start parsing the arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            if ((argl - 1) > 0)
	                rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

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

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_logfile:
	                    pip->have.logfile = TRUE ;
	                    pip->final.logfile = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = setfname(pip,pip->logfname,avp,avl,
	                                TRUE, NULL,LOGFNAME,"") ;

	                    }

	                    break ;

/* output file */
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

/* server error file */
	                case argopt_servererr:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            serverefname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            serverefname = argp ;

	                    }

	                    break ;

/* server program file */
	                case argopt_serverprog:
	                    pip->have.server = TRUE ;
	                    pip->final.server = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            serverprog = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            serverprog = argp ;

	                    }

	                    break ;

/* job name */
	                case argopt_jobname:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->jobname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->jobname = argp ;

	                    }

	                    break ;

	                case argopt_caf:
	                    f_caf = TRUE ;
	                    break ;

/* disable interval */
	                case argopt_disable:
	                    pip->f.disable = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {

	                            rs = cfdecti(avp,avl,
	                                &pip->disint) ;

	                        }

	                    }

	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    shio_printf(pip->efp,
	                        "%s: option (%s) not supported\n",
	                        pip->progname,akp) ;

	                } /* end switch */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

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

	                    case 'C':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            configfname = argp ;

	                        break ;

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                            }
	                        }

	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* job grade */
	                    case 'g':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = sncpy1(pip->jobgrade,1,argp) ;

	                        break ;

	                    case 'j':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->jobname = argp ;

	                        break ;

/* case mail address */
	                    case 'm':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                mailaddr = avp ;

	                        }

	                        break ;

/* queue name */
	                    case 'q':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->queuename = argp ;

	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        shio_printf(pip->efp,
	                            "%s: unknown option - %c\n",
	                            pip->progname,*akp) ;

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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (pip->debuglevel > 0) {

	    shio_printf(pip->efp,"%s: debuglevel=%u \n",
	        pip->progname,pip->debuglevel) ;

	}

	if (f_version) {

	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

	    if (pr == NULL) {

	        char	rootdname[MAXPATHLEN + 1] ;


	        if (nodename[0] == '\0')
	            getnodedomain(nodename,domainname) ;

	        rs = getrootdname(rootdname,MAXPATHLEN,domainname,VARPRLOCAL) ;

	        if (rs > 0)
	            proginfo_setprogroot(pip,rootdname,-1) ;

	    }

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->pr == NULL) && 
	        (pip->progdname != NULL)) {

	        proginfo_rootprogdname(pip) ;

	    }

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}

	if (pip->debuglevel > 0) {

	    shio_printf(pip->efp,
	        "%s: pr=%s\n",pip->progname,pip->pr) ;

	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

/* help file */

	if (f_help)
	    goto help ;


/* argument defaults */

	if (pip->queuename == NULL) {

		if ((cp = getenv(VARQUEUE)) != NULL)
			pip->queuename = cp ;

	}

	if (pip->queuename == NULL)
	    pip->queuename = QUEUENAME ;

	if ((! isalnum(pip->queuename[0])) || 
	    (strchr(pip->queuename,'/') != NULL) ||
	    (strlen(pip->queuename) > MAXNAMELEN)) {
	    rs = SR_INVALID ;
	    goto badqueue ;
	}

	if (pip->runint >= 0)
	    pip->have.runint = TRUE ;

	if ((pip->pollint < 0) && (argvalue > 0))
	    pip->pollint = argvalue ;

	if (pip->pollint >= 0)
	    pip->have.pollint = TRUE ;

/* continue with prepatory initialization */

	pip->daytime = time(NULL) ;

	pip->pid = getpid() ;

	pip->uid = getuid() ;

	pip->gid = getgid() ;

	pip->nodename = nodename ;
	pip->domainname = domainname ;
	if (nodename[0] == '\0')
	    getnodedomain(nodename,domainname) ;

	pip->username = username ;
	getusername(username,USERNAMELEN,-1) ;

/* create a user mail address if we don't already have one */

	cl = -1 ;
	if (mailaddr == NULL) {

	    mailaddr = tmpfname ;
	    cl = sncpy3(tmpfname,MAXPATHLEN,username,"@",domainname) ;

	}

	rs = proginfo_setentry(pip,&pip->mailaddr,mailaddr,cl) ;

/* job name ? */

	if (pip->jobname == NULL)
	    pip->jobname = pip->mailaddr ;

/* load find anything that we don't already have */

	if (serverprog != NULL) {

	    rs = mkpath3(tmpfname,pip->pr,"sbin",serverprog) ;

	    if ((rs > 0) && (xfile(pip,tmpfname) < 0))
	        rs = mkpath3(tmpfname,pip->pr,"bin",serverprog) ;

#if	CF_DEBUG
	    if ((rs < 0) || ((rs = xfile(pip,tmpfname)) < 0)) {

	        rs = findfilepath(NULL,tmpfname,serverprog,X_OK) ;

	        if (rs == 0)
	            rs = mkpath1(tmpfname, serverprog) ;

	    }
#endif /* CF_DEBUG */

	    pl = rs ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->prog_server,tmpfname,pl) ;

	} /* end subroutine (server program) */

	if (serverefname != NULL) {

	    rs = proginfo_setentry(pip,&pip->serverefname,serverefname,-1) ;

	}

/* find and open a configuration file (if there is one) */

	if (configfname == NULL)
	    configfname = CONFIGFNAME ;

	rs1 = progconfig_init(pip,configfname) ;

	if (rs1 >= 0) {
	    pip->open.configfile = TRUE ;
	}

/* server program */

	if (pip->prog_server == NULL) {

	    serverprog = PROGSERVER ;
	    rs = mkpath3(tmpfname,pip->pr,"sbin",serverprog) ;

	    if ((rs > 0) && (xfile(pip,tmpfname) < 0))
	        rs = mkpath3(tmpfname,pip->pr,"bin",serverprog) ;

#if	CF_DEBUG && 0
	    if ((rs < 0) || (xfile(pip,tmpfname) < 0)) {

	        rs = findfilepath(NULL,tmpfname,serverprog,X_OK) ;

	        if (rs == 0)
	            rs = mkpath1(tmpfname, serverprog) ;

	    }
#endif /* CF_DEBUG */

	    pl = rs ;
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->prog_server,tmpfname,pl) ;

	    pip->have.server = TRUE ;
	    pip->final.server = TRUE ;

	} /* end if (server program) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: prog_server=%s\n",pip->prog_server) ;
#endif

	if ((rs >= 0) && (pip->prog_server == NULL))
	    rs = SR_NOENT ;

	if ((rs < 0) || 
	    ((rs = xfile(pip,pip->prog_server)) < 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: failed prog_server rs=%d\n",rs) ;
#endif

	    ex = EX_OSFILE ;
	    shio_printf(pip->efp,
	        "%s: server program is not available (%d)\n",
	        pip->progname,rs) ;

	    goto ret2 ;
	}

/* mailer program */

	if (pip->prog_mailer == NULL) {

	    sp = PROGMAILER ;
	    rs = mkpath3(tmpfname,pip->pr,"sbin",sp) ;

	    if ((rs > 0) && (xfile(pip,tmpfname) < 0))
	        rs = mkpath3(tmpfname,pip->pr,"bin",sp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: prog_mailer 1 tmpfname=%s\n",tmpfname) ;
#endif

#if	CF_FINDMAILER || CF_DEBUG
	    if ((rs < 0) || (xfile(pip,tmpfname) < 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: xfile() rs=%d\n",xfile(pip,tmpfname)) ;
#endif

	        rs = findfilepath(NULL,tmpfname,sp,X_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: findfilepath() rs=%d\n",rs) ;
#endif

	        if (rs == 0)
	            rs = mkpath1(tmpfname,sp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: prog_mailer 2 tmpfname=%s\n",tmpfname) ;
#endif

	    }
#endif /* CF_DEBUG */

	    pl = rs ;
	    if (rs >= 0) {

	        rs = proginfo_setentry(pip,&pip->prog_mailer,tmpfname,pl) ;

	        pip->have.mailer = (rs >= 0) ;

	    }

	} /* end if (mailer program) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: prog_mailer=%s\n",pip->prog_mailer) ;
#endif

	if ((rs >= 0) && (pip->prog_mailer == NULL))
	    rs = SR_NOENT ;

	if ((rs < 0) || 
	    ((rs = xfile(pip,pip->prog_mailer)) < 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: failed prog_mailer rs=%d\n",rs) ;
#endif

	    ex = EX_OSFILE ;
	    shio_printf(pip->efp,
	        "%s: mailer program is not available (%d)\n",
	        pip->progname,rs) ;

	    goto ret2 ;
	}

/* server spool directory */

	if (pip->spooldname[0] == '\0') {

	    rs = mkpath4(pip->spooldname,pip->pr,SPOOLDNAME,
	        pip->searchname,pip->nodename) ;

	    if ((rs >= 0) && (u_stat(pip->spooldname,sbp) < 0))
	        rs = mkdirs(pip->spooldname,0775) ;

	    if (rs < 0) {
	        ex = EX_CONFIG ;
	        shio_printf(pip->efp,
	            "%s: configuration error with spool are (%d)\n",
	            pip->progname,rs) ;

	        goto ret2 ;
	    }

	} /* end if (spooldname) */

	pip->have.spooldir = TRUE ;
	pip->final.spooldir = TRUE ;

/* verify that the queuename given us is configured */

	mkpath2(tmpfname,pip->spooldname,pip->queuename) ;

	if ((rs = u_stat(tmpfname,sbp)) < 0)
	    goto badqueue ;

	rs = SR_NOTDIR ;
	if (! S_ISDIR(sbp->st_mode))
	    goto badqueue ;

	rs = sperm(&pip->ids,sbp,X_OK) ;
	if (rs < 0)
	    goto badqueue ;

/* server PID file */

	if (pip->pidfname[0] == '\0') {

	    mkfnamesuf1(tmpfname,pip->nodename,pip->searchname) ;

	    if (pip->pr[0] != '/') {

		proginfo_pwd(pip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pwd=%s\n",pip->pwd) ;
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: rundname=%s\n",RUNDNAME) ;
	    debugprintf("main: tmpfname=%s\n",tmpfname) ;
	}
#endif

	        mkpath4(pip->pidfname,pip->pwd,pip->pr,RUNDNAME,tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: past bad spot\n") ;
#endif

	    } else
	        mkpath3(pip->pidfname,pip->pr,RUNDNAME,tmpfname) ;

	} /* end if (creating PID file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pidfname=%s\n",pip->pidfname) ;
#endif

/* create the PID directory if we have to */

	cl = sfdirname(pip->pidfname,-1,&cp) ;

	ml = MIN(MAXPATHLEN,cl) ;
	strwcpy(tmpfname,cp,ml) ;

	rs1 = u_stat(tmpfname,sbp) ;

	if ((rs1 >= 0) && (! S_ISDIR(sbp->st_mode))) {
		rs = SR_NOTDIR ;
		goto badpiddir ;
	}

	if (rs1 < 0) {

	    rs = mkdirs(tmpfname,0775) ;

	    if (rs < 0)
		goto badpiddir ;

	}

/* other parameters */

	if (pip->runint < 1)
	    pip->runint = RUNINT ;

	if (pip->pollint < 0)
	    pip->pollint = POLLINT ;

	if (pip->markint < 0)
	    pip->markint = MARKINT ;

	if (pip->lockint < 0)
	    pip->lockint = TO_LOCK ;

	if (pip->speedint < 0)
	    pip->speedint = TO_SPEED ;

/* log ID */

	snsd(pip->logid,LOGIDLEN,nodename,(uint) pip->pid) ;

/* log file */

	if ((pip->logfname[0] == '\0') || (pip->logfname[0] == '+'))
	    mkpath3(pip->logfname,pip->pr,LOGDNAME,LOGFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: logfname=%s logid=%s\n",
	        pip->logfname,pip->logid) ;
#endif

	if (pip->logfname[0] != '-') {

	    pip->have.logfile = TRUE ;
	    rs1 = logfile_open(&pip->lh,pip->logfname,0,0666,pip->logid) ;

	    if (rs1 >= 0) {

	        pip->open.logfile = TRUE ;
	        if (pip->loglen > 0)
	            logfile_checksize(&pip->lh,pip->loglen) ;

	        logfile_printf(&pip->lh,"%s %s",
	            timestr_logz(pip->daytime,timebuf),
	            pip->banner) ;

	        logfile_printf(&pip->lh,"%-14s %s",
	            pip->progname,pip->version) ;

	        logfile_printf(&pip->lh,"%s!%s",
	            nodename,username) ;

	    } /* end if (opened log file) */

	} /* end if (have logging) */

/* batch up this submission */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	rs = shio_open(&outfile,ofname,"wct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,"%s: cannot open output (%d)\n",
	        pip->progname,rs) ;

	    goto ret4 ;
	}

/* loop through the positional arguments */

	if ((rs = vecstr_start(&flist,10,0)) >= 0) {

	    pan = 0 ;

	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && BATST(argpresent,ai) ;
	        f = f || (ai > ai_pos) ;
	        if (! f) continue ;

	        pan += 1 ;
	        rs = vecstr_add(&flist,argv[ai],-1) ;

	        if (rs < 0)
	            break ;

	    } /* end for */

	    if ((rs >= 0) && (pan == 0)) {

	        rs = vecstr_add(&flist,"-",1) ;

	    } /* end if (no explicit files were present) */

	    if (rs >= 0) {

	        rs = batch(pip,&outfile,&flist) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: batch() rs=%d\n",rs) ;
#endif

	    }

	    vecstr_finish(&flist) ;

	} /* end block */

	shio_close(&outfile) ;

/* signal the JOBSUB server if we need to */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: signal the daemon rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    pid_t	pid_server, pid_child ;


	    rs = serverpid(pip) ;

	    pid_server = rs ;
	    if (rs >= 0)
	        rs = u_kill(pid_server,SIGALRM) ;

	    if (rs < 0) {

	        rs = serverstart(pip) ;

	        pid_server = rs ;
	        if (rs >= 0) {

	            if (pip->debuglevel > 0)
	                shio_printf(pip->efp,
	                    "%s: server started PID=%u\n",
	                    pip->progname,((uint) pid_server)) ;

		    if (pip->open.logfile)
			logfile_printf(&pip->lh,
			    "server started PID=%u\n",
			    ((uint) pid_server)) ;

	            rs = uc_fork() ;
	            pid_child = rs ;
	            if (pid_child == 0) {

	                for (i = 0 ; i < NOFILE ; i += 1)
	                    u_close(i) ;

	                setsid() ;

	                sleep(5) ;

	                rs = u_kill(pid_server,SIGALRM) ;

	                uc_exit(EX_OK) ;

	            } /* end if (child process) */

	        } else {

	            shio_printf(pip->efp,
	                "%s: could not start server but job is queued (%d)\n",
	                pip->progname,rs) ;

	            rs = SR_OK ;
	        }

	    } /* end if (needed to start a new server) */

	} /* end if (interact w/ server) */

/* get out */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: after daemon spawn rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    ex = EX_DATAERR ;
	    for (i = 0 ; errormaps[i].rs != 0 ; i += 1) {

	        if (errormaps[i].rs == rs)
	            break ;

	    }

	    if (errormaps[i].rs != 0)
	        ex = errormaps[i].ex ;

	    switch (rs) {

	    case SR_ALREADY:
	    case SR_AGAIN:
	        if ((! pip->f.quiet) && (pip->efp != NULL))
	            shio_printf(pip->efp,
	                "%s: existing lock (%d)\n",
	                pip->progname,rs) ;

	        if (pip->open.logfile)
	            logfile_printf(&pip->lh,
	                "existing lock (%d)",rs) ;

	        break ;

	    default:
	        if ((! pip->f.quiet) && (pip->efp != NULL))
	            shio_printf(pip->efp,
	                "%s: failed (%d)\n",
	                pip->progname,rs) ;

	        if (pip->open.logfile)
	            logfile_printf(&pip->lh,
	                "failed (%d)",rs) ;

	    } /* end switch */

	} else
	    ex = (if_int) ? EX_INTR : EX_OK ;

#ifdef	COMMENT
	if (pip->open.logfile) {

	    logfile_printf(&pip->lh,"exiting (%d) ex=%u",
	        rs,ex) ;

	}
#endif /* COMMENT */

/* we are done */
done:
ret4:
	if (pip->debuglevel > 0) {

	    if (pip->efp != NULL)
	        shio_printf(pip->efp,"%s: program exiting ex=%d\n",
	            pip->progname,ex) ;

	}

ret3:
	if (pip->open.logfile)
	    logfile_close(&pip->lh) ;

ret2:
	if (pip->open.configfile)
	    progconfig_free(pip) ;

/* early return thing */
retearly:
ret1:
	if (pip->efp != NULL)
	    shio_close(pip->efp) ;

ret0:
	ids_release(&pip->ids) ;

	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

/* if child => exit, needed since return doesn't lead to exit ! */

	if (f_child)
	    uc_exit(ex) ;

/* restore and get out */

	j = 0 ;
	for (i = 0 ; sigints[i] != 0 ; i += 1)
	    u_sigaction(sigints[i],(sao + j++),NULL) ;

	for (i = 0 ; sigignores[i] != 0 ; i += 1)
	    u_sigaction(sigignores[i],(sao + j++),NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

	return ex ;

/* the information type thing */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:

#if	CF_SFIO
	printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badpiddir:
	ex = EX_OSERR ;
	shio_printf(pip->efp,"%s: could not create PID directory (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badqueue:
	ex = EX_CONFIG ;
	shio_printf(pip->efp,"%s: bad queue or not configured (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


/* calculate a file name */

/****

	Description:

	Sets a filename from a configuration parameter.

	Arguments:

	pip		program information
	fname		filename buffer to set
	ebuf		buffer to analyze for extracting filename
	el		length of buffer to analyze
	f_def		should we use a default value
	dname		default directory name
	name		default base filename
	suf		default filename suffix

	Returns:

	<0		error
	>=0		length of created filename

****/

int setfname(pip,fname,ebuf,el,f_def,dname,name,suf)
struct proginfo	*pip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int	rs = 0 ;
	int	ml ;

	char	tmpname[MAXNAMELEN + 1], *np ;


	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = (char *) name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = (char *) tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }

	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname, pip->pr,np) ;
	    } else
	        rs = mkpath1(fname, np) ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = (char *) ebuf ;
	    if (el >= 0) {

	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;

	    }

	    if (ebuf[0] != '/') {

	        if (strchr(np,'/') != NULL) {

	            rs = mkpath2(fname,pip->pr,np) ;

	        } else {

	            if ((dname != NULL) && (dname[0] != '\0'))
	                rs = mkpath3(fname,pip->pr,dname,np) ;

	            else
	                rs = mkpath2(fname,pip->pr,np) ;

	        }

	    } else
	        rs = mkpath1(fname,np) ;

	}

	return rs ;
}
/* end subroutine (setfname) */


int xfile(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct ustat	sb ;
	struct ustat	*sbp = &sb ;

	int	rs ;


	rs = u_stat(fname,sbp) ;

	if (rs >= 0) {

	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sbp->st_mode)) {

#if	CF_SPERM
	        rs = sperm(&pip->ids,sbp,X_OK) ;
#else
	        rs = perm(fname,-1,-1,NULL,X_OK) ;
#endif

	    }
	}

	return rs ;
}
/* end subroutine (xfile) */



/* LOCAL SUBROUTINES */



static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;


	shio_printf(pip->efp,
	    "%s: USAGE> %s [shellscript(s)] [-q queuename] [-m ema]\n",
	    pip->progname,pip->progname) ;

	rs = shio_printf(pip->efp,"%s: \t[-j jobname] [-?V] [-Dv]\n",
	    pip->progname) ;

	return rs ;
}
/* end subroutine (usage) */


/* start the server program */
static int serverstart(pip)
struct proginfo	*pip ;
{
	struct ustat	sb ;

	vechand	envs ;

	vecstr	tmpstore ;

	int	rs, rs1, i ;
	int	opts ;
	int	ml, cl ;
	int	f_envs ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	digbuf[DIGBUFLEN + 1] ;
	char	*cp ;
	char	*av[2] ;
	char	**ev ;


	cl = sfdirname(pip->pidfname,-1,&cp) ;

	ml = MIN(MAXPATHLEN,cl) ;
	strwcpy(tmpfname,cp,ml) ;

	rs1 = u_stat(tmpfname,&sb) ;

	if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode))) {
		rs = SR_NOTDIR ;
		goto ret0 ;
	}

	if (rs1 < 0) {

	    rs = mkdirs(tmpfname,0660) ;

	    if (rs < 0)
		goto ret0 ;
	}

/* prepapre to spawn off server */

	av[0] = PROGSERVER ;
	av[1] = NULL ;

	opts = VECHAND_OSORTED | VECHAND_OCOMPACT ;
	rs = vechand_start(&envs,NENV,opts) ;

	f_envs = (rs >= 0) ;
	if (rs >= 0)
	    rs = vecstr_start(&tmpstore,NENV,0) ;

	if (rs >= 0) {

	    rs = prepenv(&envs,&tmpstore,
	        VARMAILER,pip->prog_mailer) ;

	    if (rs >= 0)
	        rs = prepenv(&envs,&tmpstore,
	            VARSPOOLDNAME,pip->spooldname) ;

	    if (rs >= 0)
	        rs = prepenv(&envs,&tmpstore,
	            VARPIDFNAME,pip->pidfname) ;

	    if ((rs >= 0) && (pip->logfname[0] != '\0'))
	        rs = prepenv(&envs,&tmpstore,
	            VARLOGFNAME,pip->logfname) ;

	    if ((rs >= 0) && (pip->serverefname != NULL))
	        rs = prepenv(&envs,&tmpstore,
	            VARSTDERRFNAME,pip->serverefname) ;

	    if (rs >= 0) {

		rs = ctdecui(digbuf,DIGBUFLEN,pip->pollint) ;

		if (rs >= 0)
	            rs = prepenv(&envs,&tmpstore,
	                VARPOLLINT,digbuf) ;

	    }

	    if (rs >= 0) {

	        for (i = 0 ; pip->envv[i] != NULL ; i += 1) {

	            rs1 = vechand_search(&envs,pip->envv[i],
	                vstrkeycmp,NULL) ;

	            if (rs1 == SR_NOTFOUND)
	                vechand_add(&envs,pip->envv[i]) ;

	        } /* end for */
	    }

	    if (rs >= 0)
	        rs = vechand_getvec(&envs,&ev) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: vechand_getvec() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG && 0
	    if (DEBUGLEVEL(2)) {
	        for (i = 0 ; ev[i] != NULL ; i += 1)
	            debugprintf("main: ev[%u]=%s\n",i,ev[i]) ;
	    }
#endif

	    if (rs >= 0) {

	        struct spawnproc	psa ;


	        memset(&psa,0,sizeof(struct spawnproc)) ;

	        psa.disp[0] = SPAWNPROC_DCLOSE ;
	        psa.disp[1] = SPAWNPROC_DCLOSE ;
	        psa.disp[2] = SPAWNPROC_DINHERIT ;
	        rs = spawnproc(&psa,pip->prog_server,av,ev) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: spawnproc() rs=%d\n",rs) ;
#endif

	    }

	    vecstr_finish(&tmpstore) ;

	} /* end if */

	if (f_envs)
	    vechand_finish(&envs) ;

ret0:
	return rs ;
}
/* end subroutine (serverstart) */


/* get the server PID (for signalling) */
static int serverpid(pip)
struct proginfo	*pip ;
{
	bfile	pidfile ;

	pid_t	pid_server ;

	uint	uiw ;

	int	rs, rs1, i ;
	int		len ;

	char	linebuf[LINEBUFLEN + 1] ;


	if ((rs = bopen(&pidfile,pip->pidfname,"r",0666)) >= 0) {

	    rs = breadline(&pidfile,linebuf,LINEBUFLEN) ;

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        linebuf[--len] = '\0' ;

	    bclose(&pidfile) ;

	} /* end if */

/* either signal the existing server or start a new one */

	if (rs >= 0) {

	    rs = cfdecui(linebuf,len,&uiw) ;

	    pid_server = uiw ;

	}

	return (rs >= 0) ? ((int) pid_server) : rs ;
}
/* end subroutine (serverpid) */


/* environment variable processing */
static int prepenv(envp,tsp,key,value)
vechand		*envp ;
vecstr		*tsp ;
const char	key[] ;
const char	value[] ;
{
	int	rs ;
	int	i ;

	char	*cp ;


	rs = vecstr_envadd(tsp,key,value,-1) ;
	i = rs ;
	if (rs >= 0) {

	    rs = vecstr_get(tsp,i,&cp) ;

	    if (rs >= 0)
	        rs = vechand_add(envp,cp) ;

	}

	return rs ;
}
/* end subroutine (prepenv) */



