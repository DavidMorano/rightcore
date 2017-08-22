/* main */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_CPUSPEED	1		/* calculate CPU speed */


/* revision history:

	= 89/03/01, David A­D­ Morano

	This subroutine was originally written.  


	= 98/06/01, David A­D­ Morano

	I enhanced the program a little to print out some other
	information.


	= 99/03/01, David A­D­ Morano

	I enhanced the program a little to to do something (I forget
	what).


	= 04/01/10, David A­D­ Morano

	The KSH program switched to using a fakey "large file" (64-bit
	fake-out mode) compilation mode on Solaris.  This required
	some checking to see if any references to 'u_stat()' had to be
	updated to work with the new KSH.  Although we call 'u_stat()'
	here, its structure is not passed to other subroutines expecting
	the regular 32-bit structure.


	= 05/04/20, David A­D­ Morano

	I changed the program so that the configuration file is consulted
	even if the program is not run in daemon-mode.	Previously, the
	configuration file was only consulted when run in daemon-mode.
	The thinking was that running the program in regular (non-daemon)
	mode should be quick.  The problem is that the MS file had to
	be guessed without the aid of consulting the configuration file.
	Although not a problem in most practice, it was not aesthetically
	appealing.  It meant that if the administrator changed the MS file
	in the configuration file, it also had to be changed by specifying
	it explicitly at invocation in non-daemon-mode of the program.
	This is the source of some confusion (which the world really
	doesn't need).	So now the configuration is always consulted.
	The single one-time invocation is still fast enough for the
	non-smoker aged under 40! :-) :-)


*/


/**************************************************************************

	This is a built-in command to the KSH shell.  It should also
	be able to be made into a stand-alone program without much
	(if almost any) difficulty, but I have not done that yet (we
	already have a MSU program out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot
	return since they would be returning to a KSH program that thinks
	it is alive (!) and that geneally causes some sort of problem or
	another.  That is just some weird thing asking for trouble.  So we
	have to take care to force child processes to exit explicitly.
	Child processes are only created when run in "daemon" mode.

	Execute as :

	$ msu [-speed[=<name>]] [-zerospeed] [-msfile <file>]


	Implementation note:

	It is difficult to close files when run as a SHELL builtin!
	We want to close files when we run in the background, but when
	running as a SHELL builtin, we cannot close file descriptors
	untill after we fork (else we corrupt the enclosing SHELL).
	However, we want to keep the files associated with persistent
	objects open across the fork.  This problem is under review.
	Currently, there is not an adequate self-contained solution.


*****************************************************************************/


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
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<logfile.h>
#include	<msfile.h>
#include	<kinfo.h>
#include	<lfm.h>
#include	<getxusername.h>
#include	<getutmpent.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"config.h"
#include	"defs.h"
#include	"msflag.h"
#include	"stat32.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	DEVTTY
#define	DEVTTY		"/dev/tty"
#endif


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
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	getrootdname(char *,int,const char *,const char *) ;
extern int	batch(struct proginfo *,void *,vecstr *) ;

#if	CF_CPUSPEED
extern int	cpuspeed(const char *,const char *,int) ;
#endif

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

static void	sighand_int(int) ;

static int	usage(struct proginfo *) ;
static int	msupdate(struct proginfo *,LFM *) ;


/* local variables */

static volatile int	if_exit ;
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
	"jobname",
	"db",
	"msfile",
	"mspoll",
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
	argopt_jobname,
	argopt_db,
	argopt_msfile,
	argopt_mspoll,
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
	"msfile",
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
	param_msfile,
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
	{ SR_INTR, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	struct session	sinfo ;

	struct ustat32	sb ;

	struct ustat	*sbp = (struct ustat *) &sb ;

	sigset_t	oldsigmask, newsigmask ;

	SHIO	errfile ;
	SHIO	outfile ;

	vecstr	flist ;

	int	argr, argl, aol, akl, avl ;
	int	ai, ai_max, ai_pos, pan, kwi ;
	int	argvalue = -1 ;
	int	rs, rs1, n, i, j ;
	int	size ;
	int	sl, cl, ml ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_child = FALSE ;
	int	f_caf = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	username[USERNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	mailaddr[MAILADDRLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*pr = NULL ;
	char	*configfname = NULL ;
	char	*outfname = NULL ;
	char	*msfname = NULL ;
	char	*cp ;


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

	    if (cfdeci(cp,-1,&fd_debug) >= 0)
	        debugsetfd(fd_debug) ;

	    else
	        fd_debug = debugopen(cp) ;

	}
#endif /* CF_DEBUGS */


	proginfo_start(pip,environ,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if ((cp = getenv(VARSTDERRFNAME)) == NULL)
	    cp = STDERRFNAME ;

	rs1 = shio_open(&errfile,cp,"wca",0666) ;

	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CLINEBUF,0) ;

	}


/* initialize */

	memset(&sinfo,0,sizeof(struct session)) ;

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	mailaddr[0] = '\0' ;

	pip->loglen = -1 ;
	pip->speedint = -1 ;
	pip->pollint = -1 ;
	pip->lockint = -1 ;
	pip->markint = -1 ;
	pip->runint = -1 ;
	pip->disint = -1 ;

	pip->f.quiet = FALSE ;
	pip->f.daemon = FALSE ;
	pip->f.speed = FALSE ;
	pip->f.zerospeed = FALSE ;


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

/* do we have a keyword or only key letters? */

	            if ((kwi = matpstr(argopts,2,akp,akl)) >= 0) {

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
	                            outfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            outfname = argp ;

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

/* MS file name */
	                case argopt_db:
	                case argopt_msfile:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            msfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            msfname = argp ;

	                    }

	                    break ;

/* MS poll interval */
	                case argopt_mspoll:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {

	                            rs = cfdecti(avp,avl,
	                                &pip->pollint) ;

	                        }

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) {

	                            rs = cfdecti(argp,argl,
	                                &pip->pollint) ;

	                        }
	                    }

	                    break ;

	                case argopt_speed:
	                    pip->f.speed = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            strwcpy(pip->speedname,avp,
	                                MIN(avl,MAXNAMELEN)) ;

	                    }

	                    break ;

	                case argopt_zerospeed:
	                    pip->f.zerospeed = TRUE ;
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

/* daemon mode */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                if (isdigit(*avp)) {

	                                    rs = cfdecti(avp,avl,
	                                        &pip->runint) ;

	                                } else if (tolower(*avp) == 'i')
	                                    pip->runint = INT_MAX ;

	                                else
	                                    rs = SR_INVALID ;

	                            }
	                        }

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
					rs = sncpy1(mailaddr,MAILADDRLEN,avp) ;

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

#if	CF_DEBUGS
	debugprintf("main: finished parsing arguments\n") ;
#endif

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

	if (pip->queuename == NULL)
		pip->queuename = QUEUENAME ;

	if ((! isalnum(pip->queuename[0])) || 
		(strchr(pip->queuename,'/') != NULL) ||
		(strlen(pip->queuename) > MAXNAMELEN)) {
		rs = SR_INVALID ;
		goto badqueue ;
	}

	if (msfname == NULL) {

	    if ((cp = getenv(VARMSFNAME)) != NULL) {
		pip->final.msfile = TRUE ;
	        msfname = cp ;
	    }

	}

	if (msfname != NULL) {

	    pip->have.msfile = TRUE ;
	    mkpath1(pip->msfname,msfname) ;

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

/* create a mail address if we don't already have one */

	if (mailaddr[0] == '\0')
		sncpy3(mailaddr,MAILADDRLEN,username,"@",domainname) ;

	pip->mailaddr = mailaddr ;

/* job name? */

	if (pip->jobname == NULL)
		pip->jobname = pip->mailaddr ;

/* find and open a configuration file (if there is one) */

	if (configfname == NULL)
	    configfname = CONFIGFNAME ;

	rs1 = progconfig_init(pip,configfname) ;

	if (rs1 >= 0) {
	    pip->open.configfile = TRUE ;
	}

/* find anything that we don't already have */

	if (pip->msfname[0] == '\0') {

	    mkpath3(pip->msfname,pip->pr,"var",MSFNAME) ;

	}

	if (pip->f.daemon && (pip->pidfname[0] == '\0')) {

	    mkfnamesuf1(tmpfname,pip->nodename,PIDFNAME) ;

	    mkpath3(pip->pidfname,pip->pr,RUNDNAME,tmpfname) ;

	}

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

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.logfile) ;
#endif


/* logging is normally only for daemon mode */

	if (pip->have.logfile) {

/* log ID */

	    snsd(pip->logid,LOGIDLEN,nodename,pip->pid) ;

/* log file */

	    if (pip->logfname[0] == '\0')
	        mkpath3(pip->logfname,pip->pr,LOGDNAME,LOGFNAME) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_msu: logfname=%s logid=%s\n",
	            pip->logfname,pip->logid) ;
#endif

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

	    }

	} /* end if (logging) */

/* batch up this submission */

	if ((outfname == NULL) || (outfname[0] == '\0'))
	    outfname = STDOUTFNAME ;

	rs = shio_open(&outfile,outfname,"wct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: cannot open output (%d)\n",
		pip->progname,rs) ;

	    goto ret4 ;
	}


	if ((rs = vecstr_start(&flist,10,0)) >= 0) {

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    rs = vecstr_add(&flist,argv[ai],-1) ;

	    if (rs < 0)
		break ;

	} /* end for */

	if (rs >= 0)
	rs = batch(pip,&outfile,&flist) ;

	    vecstr_finish(&flist) ;

	} /* end block */

	shio_close(&outfile) ;


/* HERE */

/* can we open the MS file? */

	rs = perm(pip->msfname,-1,-1,NULL,(R_OK | W_OK)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu: perm() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (pip->debuglevel > 0)
	        shio_printf(pip->efp,
	            "%s: ms=%s\n", pip->progname,pip->msfname) ;

	    if (pip->open.logfile)
	        logfile_printf(&pip->lh, "ms=%s", pip->msfname) ;

	    if (pip->f.daemon) {

	        LFM	pidlock ;

	        int	pid, cs ;


	        if (pip->open.logfile)
	            logfile_flush(&pip->lh) ;

	        shio_flush(pip->efp) ;

	        rs = uc_fork() ;
	        pid = rs ;
	        if (rs > 0)
	            u_waitpid(pid,&cs,0) ;

	        if (rs == 0) {

	            sinfo.sid = getsid((pid_t) 0) ;

	            rs1 = getutmpline(sinfo.termdev,MAXPATHLEN,sinfo.sid) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_msu: getutmpline() "
				"rs=%d termdev=%s\n",
	                    rs1,sinfo.termdev) ;
#endif

	            if (rs1 < 0)
	                sncpy1(sinfo.termdev,MAXPATHLEN,DEVTTY) ;

#ifdef	COMMENT
		    shio_close(pip->efp) ;
#endif

	            memset(pip->efp,0,sizeof(SHIO)) ;

	            if (f_caf) {

	                pip->efp = NULL ;

		    if (pip->open.logfile)
			logfile_close(&pip->lh) ;

		    if (pip->have.configfile)
			progconfig_free(pip) ;

		    for (i = 0 ; i < NOFILE ; i += 1) {

			if (i != fd_debug)
				u_close(i) ;

		    }

		    if (pip->have.configfile) {

			rs1 = progconfig_init(pip,configfname) ;

			pip->open.configfile = (rs1 >= 0) ;
		    }

		    if (pip->have.logfile) {
			const char	*lf = pip->logfname ;
			const char	*lid = pip->logid ;
	    	        rs1 = logfile_open(&pip->lh,lf,0,0666,lod) ;
			pip->open.logfile = (rs1 >= 0) ;
		    }

	            } else {

	                if (sinfo.termdev[0] != '\0') {

	                    rs1 = shio_open(pip->efp,sinfo.termdev,"w",0666) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("b_msu: shio_open() rs=%d termdev=%s\n",
	                            rs1,sinfo.termdev) ;
#endif

	                }

	                if ((sinfo.termdev[0] == '\0') || (rs1 < 0))
	                    pip->efp = NULL ;

	            }

	            u_setsid() ;

	            rs = uc_fork() ;

	            if (rs != 0) {
	                ex = (rs >= 0) ? EX_OK : EX_OSERR ;
	                uc_exit(ex) ;
	            } /* end if (parent exits) */

	            if (rs == 0) {
	                f_child = TRUE ;
	                pip->pid = getpid() ;

	                if (pip->debuglevel > 0) {

	                    shio_printf(pip->efp,"%s: pidlock=%s\n",
	                        pip->progname,
	                        pip->pidfname) ;

	                    shio_printf(pip->efp,"%s: runint=%u\n",
	                        pip->progname,
	                        pip->runint) ;

	                    shio_printf(pip->efp,"%s: pollint=%u\n",
	                        pip->progname,
	                        pip->pollint) ;

	                    shio_flush(pip->efp) ;

	                }

	                if (pip->open.logfile) {

	                    long	lw ;


#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_msu: daemon log\n") ;
#endif

	                    lw = pip->runint ;
	                    logfile_printf(&pip->lh,
	                        "runint=%s pollint=%u",
	                        timestr_elapsed(lw,timebuf),
	                        pip->pollint) ;

	                    logfile_printf(&pip->lh,
	                        "pid=%u",((uint) pip->pid)) ;

	                    logfile_flush(&pip->lh) ;

	                } /* end if (some logging) */

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_msu: pidname=%s\n",pip->pidfname) ;
#endif

	                rs = SR_OK ;
	                if (pip->pidfname[0] != '\0') {

	                    cl = sfdirname(pip->pidfname,-1,&cp) ;

	                    ml = MIN(MAXPATHLEN,cl) ;
	                    strwcpy(tmpfname,cp,ml) ;

	                    if (u_stat(tmpfname,sbp) < 0)
	                        mkdirs(tmpfname,777) ;

	                    pip->have.pidfile = TRUE ;
	                    rs = lfm_start(&pidlock,pip->pidfname,
	                        LFM_TRECORD, pip->lockint,NULL,
	                        pip->nodename,pip->username,pip->banner) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_msu: lfm_start() rs=%d\n",rs) ;
#endif

	                } /* end if */

	                if (rs >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_msu: daemon msupdate()\n") ;
#endif

	                    rs = msupdate(pip,&pidlock) ;

	                    if (rs == SR_AGAIN)
	                        rs = SR_OK ;

	                    if (pip->have.pidfile)
	                        lfm_finish(&pidlock) ;

	                } /* end if (got lock) */

	            } /* end if (grandchild) */

	        } /* end if (child) */

	    } else
	        rs = msupdate(pip,NULL) ;

	} /* end if (have a MS file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_msu: daemon=%u child=%u\n",
	            pip->f.daemon,f_child) ;
#endif

	    if ((! pip->f.daemon) || f_child) {

	        shio_printf(pip->efp, "%s: MS updates=%u\n",
	            pip->progname,rs) ;

	    }
	}

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
	                "%s: could not perform update (%d)\n",
	                pip->progname,rs) ;

	        if (pip->open.logfile)
	            logfile_printf(&pip->lh,
	                "could not perform update (%d)",rs) ;

	    } /* end switch */

	} else
	    ex = (if_int) ? EX_TERM : EX_OK ;

	if (pip->open.logfile) {

	    if ((! pip->f.daemon) || f_child)
	        logfile_printf(&pip->lh,"exiting (%d) ex=%u",
	            rs,ex) ;

	}

/* we are done */
done:
ret4:
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {

	    if (pip->f.daemon) {

	        shio_printf(pip->efp,"%s: program (%s) exiting ex=%d\n",
	            pip->progname,((f_child) ? "child" : "parent"),ex) ;

	    } else
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
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG) && 0
	debugclose() ;
#endif

/* if child => exit, needed since return doesn't lead to exit! */

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

badqueue:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: bad queue name given (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


/* calculate a file name */
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
	    "%s: USAGE> %s [-speed[=name]] [-msfile file] [-zerospeed]\n",
	    pip->progname,pip->progname) ;

	rs = shio_printf(pip->efp,"%s: \t[-mspoll <int>] [-?V] [-Dv]\n",
	    pip->progname) ;

	return rs ;
}
/* end subroutine (usage) */


static int msupdate(pip,lp)
struct proginfo	*pip ;
LFM		*lp ;
{
	struct pollfd	fds[2] ;

	MSFILE		ms ;

	MSFILE_ENT	e, etmp ;

	KINFO		ki ;

	KINFO_DATA	d ;

	time_t	ti_start ;
	time_t	ti_log ;

	long	lw ;

	uint	ppm ;
	uint	pagesize = getpagesize() ;

	int	rs, rs1, i, c ;
	int	nfds, oflags ;
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: runint=%s\n",
	        timestr_elapsed((time_t) pip->runint,timebuf)) ;
#endif

	nfds = 0 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = 0 ;
	fds[nfds].revents = 0 ;

	oflags = O_RDWR ;
	rs = msfile_open(&ms,pip->msfname,oflags,0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: msfile_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	pip->daytime = time(NULL) ;

	ti_log = pip->daytime ;
	ti_start = pip->daytime ;

	rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&e) ;

	if (rs1 < 0) {

	    memset(&e,0,sizeof(MSFILE_ENT)) ;

	    strwcpy(e.nodename,
	        pip->nodename,MSFILEE_LNODENAME) ;

	}

/* get some updated information */

	if (! pip->f.zerospeed) {

#if	CF_CPUSPEED
	    f = (e.boottime == 0) || (e.speed == 0) || pip->f.speed ;

	    if (! f)
	        f = (e.stime == 0) || 
	            ((pip->daytime - e.stime) >= pip->speedint) ;

	    if (f) {

	        if (pip->debuglevel > 0)
	            shio_printf(pip->efp,
	                "%s: speed recalculation is indicated\n",
	                pip->progname) ;

	        shio_flush(pip->efp) ;

	        rs1 = cpuspeed(pip->pr,pip->speedname,0) ;

	        pip->daytime = time(NULL) ;

	        if (rs1 < 0) {

	            if ((! pip->f.quiet) && (pip->efp != NULL)) {

	                shio_printf(pip->efp,
	                    "%s: speed name=%s\n",
	                    pip->progname,pip->speedname) ;

	                shio_printf(pip->efp,
	                    "%s: speed subsystem is not available (%d)\n",
	                    pip->progname,rs1) ;

	            }

	        } else {
	            e.speed = rs1 ;
	            e.stime = pip->daytime ;
	        }

	    } /* end if (needed speed update) */
#endif /* CF_CPUSPEED */

	} else {

	    e.speed = 0 ;
	    e.stime = pip->daytime ;

	}

/* were we requested to do a disable? */

	if (pip->f.disable) {

	    e.flags |= MSFLAG_MDISABLED ;
	    if (pip->disint > 0)
	        e.dtime = pip->daytime + pip->disint ;

	}

/* do some load-ave updates */

	c = 0 ;
	rs = kinfo_open(&ki,pip->daytime) ;

	if (rs >= 0) {

	    while (! if_int) {

	        LFM_CHECK	ci ;


	        lw = pip->runint - (pip->daytime - ti_start) ;
	        if (lw <= 0)
	            break ;

	        if (pip->f.daemon) {

	            if (pip->have.pidfile && (lp != NULL)) {

	                rs = lfm_check(lp,&ci,pip->daytime) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_msu/msupdate: lfm_check() rs=%d\n",rs) ;
	                    if (rs < 0) {
	                        debugprintf("b_msu/msupdate: pid=%d\n",ci.pid) ;
	                        debugprintf("b_msu/msupdate: node=%s\n",
					ci.nodename) ;
	                        debugprintf("b_msu/msupdate: user=%s\n",
					ci.username) ;
	                        debugprintf("b_msu/msupdate: banner=%s\n",
					ci.banner) ;
	                    }
	                }
#endif /* CF_DEBUG */

	            } /* end if (had a lock file) */

#ifdef	COMMENT
	            if ((rs < 0) && (rs != SR_AGAIN))
	                break ;

	            if (rs == SR_AGAIN) {
	                rs = SR_OK ;
	                break ;
	            }
#else
	            if (rs < 0)
	                break ;
#endif /* COMMENT */

	        } /* end if (daemon mode) */

/* continue with the update */

	        rs = kinfo_sysmisc(&ki,pip->daytime,&d) ;

	        e.boottime = (uint) d.boottime ;
	        e.ncpu = d.ncpu ;
	        e.nproc = d.nproc ;

	        e.la[0] = d.la_1min ;
	        e.la[1] = d.la_5min ;
	        e.la[2] = d.la_15min ;

/* calculate pages-per-megabyte */

	        ppm = (1024 * 1024) / pagesize ;

/* OK, now calculate the megabytes of each type of memory */

#if	defined(SOLARIS)
	        rs1 = uc_sysconf(_SC_PHYS_PAGES,&lw) ;

		e.pmtotal = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmtotal = (lw / ppm) ;

	        rs1 = uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

		e.pmavail = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmavail = (lw / ppm) ;
#else
		e.pmavail = 1 ;
#endif /* defined(SOLARIS) */

/* finish off with time stamps */

	        e.atime = (uint) 0 ;
	        e.utime = (uint) pip->daytime ;

/* write-back the update */

	        rs = msfile_update(&ms,pip->daytime,&e) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msu/msupdate: msfile_update() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	        c += 1 ;

	        if (! pip->f.daemon)
	            break ;

/* sleep for daemon mode */

	        for (i = 0 ; (! if_int) && (i < pip->pollint) ; i += 1) {

	            rs1 = u_poll(fds,nfds,POLLINTMULT) ;

	            if (rs1 < 0)
	                if_int = TRUE ;

	        } /* end for */

	        if (if_int)
	            break ;

	        pip->daytime = time(NULL) ;

/* maintenance */

	        if ((c & 15) == 3)
	            kinfo_check(&ki,pip->daytime) ;

	        if (pip->have.configfile && ((c & 3) == 4))
	            progconfig_check(pip) ;

	        if ((c & 15) == 5)
	            msfile_check(&ms,pip->daytime) ;

	        if (pip->open.logfile && ((c & 7) == 1)) {

	            if ((pip->daytime - ti_log) >= pip->markint) {

	                ti_log = pip->daytime ;
	                lw = labs(pip->runint - (pip->daytime - ti_start)) ;

	                logfile_printf(&pip->lh,
	                    "%s mark> %s",
	                    timestr_logz(pip->daytime,timebuf),
	                    pip->nodename) ;

	                logfile_printf(&pip->lh,
	                    "remaining=%s",
	                    timestr_elapsed(lw,timebuf)) ;

	                logfile_flush(&pip->lh) ;

	            }
	        }

/* periodically close and rdebugopen the log file (precaution?) */

	        if ((c & 31) == 1) {

		    if (pip->open.logfile) {

		        pip->open.logfile = FALSE ;
	                logfile_close(&pip->lh) ;

		    }

		    if (pip->have.logfile) {
			const char	*lf = pip->logfname ;
			const char	*lid = pip->logid ;
	                rs1 = logfile_open(&pip->lh,lf,0,0666,lid) ;
	                pip->open.logfile = (rs1 >= 0) ;
		    }

	        } /* end if (logging) */

	        if (pip->cmd[0] != '\0') {

	            if (strcmp(pip->cmd,"exit") == 0)
	                break ;

	        }

/* get a flesh copy of the entry */

	        rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&etmp) ;

	        if (rs1 >= 0)
	            e = etmp ;

/* check disabled state only in daemon mode */

	        if ((e.dtime != 0) && (pip->daytime >= e.dtime)) {

	            e.dtime = 0 ;
	            e.flags &= (~ MSFLAG_MDISABLED) ;

	        }

	    } /* end while (end time not reached) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("b_msu/msupdate: white-out if_int=%u\n",
	            if_int) ;
	        debugprintf("b_msu/msupdate: start=%s\n",
	            timestr_log(ti_start,timebuf)) ;
	        debugprintf("b_msu/msupdate: now=%s\n",
	            timestr_log(pip->daytime,timebuf)) ;
	    }
#endif

	    kinfo_close(&ki) ;

	} /* end if (opened kernel channel) */

	msfile_close(&ms) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: ret rs=%d c=%u\n",rs,c) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msupdate) */



