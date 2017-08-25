/* main */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* get the 'exec(2)' name */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  

	= 1998-06-01, David A­D­ Morano
	I enhanced the program a little to print out some other information.

	= 1999-03-01, David A­D­ Morano
	I enhanced the program a little to to do something (I forget what).

	= 2004-01-10, David A­D­ Morano
	This subroutine was originally written.  
        The KSH program switched to using a fakey "large file" (64-bit fake-out
        mode) compilation mode on Solaris. This required some checking to see if
        any references to 'u_stat()' had to be updated to work with the new KSH.
        Although we call 'u_stat()' here, its structure is not passed to other
        subroutines expecting the regular 32-bit structure.

	= 2005-04-20, David A­D­ Morano
        I changed the program so that the configuration file is consulted even
        if the program is not run in daemon-mode. Previously, the configuration
        file was only consulted when run in daemon-mode. The thinking was that
        running the program in regular (non-daemon) mode should be quick. The
        problem is that the MS file had to be guessed without the aid of
        consulting the configuration file. Although not a problem in most
        practice, it was not aesthetically appealing. It meant that if the
        administrator changed the MS file in the configuration file, it also had
        to be changed by specifying it explicitly at invocation in
        non-daemon-mode of the program. This is the source of some confusion
        (which the world really doesn't need). So now the configuration is
        always consulted. The single one-time invocation is still fast enough
        for the non-smoker aged under 40! :-) :-)

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This is a built-in command to the KSH shell.  It should also
	be able to be made into a stand-alone program without much
	(if almost any) difficulty, but I have not done that yet (we
	already have a MSU program out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally !  They cannot
	return since they would be returning to a KSH program that thinks
	it is alive (!) and that geneally causes some sort of problem or
	another.  That is just some weird thing asking for trouble.  So we
	have to take care to force child processes to exit explicitly.
	Child processes are only created when run in "daemon" mode.

	Synopsis:

	$ msu [-speed[=<name>]] [-zerospeed] [-msfile <file>]


	Implementation note:

	It is difficult to close files when run as a SHELL builtin !
	We want to close files when we run in the background, but when
	running as a SHELL builtin, we cannot close file descriptors
	untill after we fork (else we corrupt the enclosing SHELL).
	However, we want to keep the files associated with persistent
	objects open across the fork.  This problem is under review.
	Currently, there is not an adequate self-contained solution.


*****************************************************************************/


#include	<envstandards.h>

#if	defined(SFIO) || defined(KSHBUILTIN)
#undef	CF_SFIO
#define	CF_SFIO	1
#endif


#if	CF_SFIO
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
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"msflag.h"
#include	"stat32.h"

#include	"shio.h"



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

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
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
extern int	matstr(const char **,char *,int) ;
extern int	matstr2(const char **,char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	getrootdname(char *,int,const char *,const char *) ;
extern int	watch(struct proginfo *,LFM *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

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


/* local variables */

int			if_int ;
int			if_child ;

static const int	sigblocks[] = {
	SIGHUP,
	SIGURG,
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
	SIGCHLD,
	0
} ;

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"LOGFILE",
	"of",
	"slfile",
	"slpoll",
	"ddir",
	"caf",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_logfile,
	argopt_of,
	argopt_slfile,
	argopt_slpoll,
	argopt_ddir,
	argopt_caf,
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

	struct ustat32	sb ;

	struct ustat	*sbp = (struct ustat *) &sb ;

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
	int	sl, cl, ml ;
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
	char	mailaddr[MAILADDRLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*configfname = NULL ;
	const char	*ofname = NULL ;
	const char	*slfname = NULL ;
	const char	*ddname = NULL ;
	const char	*cp ;


	if_int = 0 ;
	if_child = 0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

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

	proginfo_start(pip,environ,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;


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
	pip->markint = -1 ;
	pip->pollint = -1 ;
	pip->lockint = -1 ;
	pip->pidint = -1 ;
	pip->runint = -1 ;
	pip->disint = -1 ;

	pip->f.quiet = FALSE ;
	pip->f.daemon = FALSE ;


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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            if ((argl - 1) > 0)
	                rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

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

/* do we have a keyword or only key letters ? */

	            if ((kwi = matstr2(argopts,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 1 ;
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

/* SL file name */
	                case argopt_slfile:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            slfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            slfname = argp ;

	                    }

	                    break ;

/* Data Directory name */
	                case argopt_ddir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ddname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ddname = argp ;

	                    }

	                    break ;

/* SL poll interval */
	                case argopt_slpoll:
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

	                case argopt_caf:
	                    f_caf = TRUE ;
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

/* background */
	                    case 'b':
	                        pip->f.background = TRUE ;
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

	                                } else if (tolower(*avp) == 'i') {
	                                    pip->runint = INT_MAX ;
	                                } else
	                                    rs = SR_INVALID ;

	                            }
	                        }

	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 1 ;
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

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    slfname = argv[ai] ;
	    BACLR(argpresent,ai) ;
		pip->final.slfile = TRUE ;
	    break ;

	} /* end for */

	if (slfname == NULL) {

	    if ((cp = getenv(VARSLFNAME)) != NULL) {
		pip->final.slfile = TRUE ;
	        slfname = cp ;
	    }

	}

	if (slfname != NULL) {

	    pip->have.slfile = TRUE ;
	    mkpath1(pip->slfname,slfname) ;

	}

/* data directory */

	if (ddname == NULL) {

	    if ((cp = getenv(VARDDNAME)) != NULL) {
		pip->final.ddir = TRUE ;
	        ddname = cp ;
	    }

	}

	if (ddname != NULL) {

	    pip->have.ddir = TRUE ;
	    mkpath1(pip->ddname,ddname) ;

	}

/* everything else */

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

	pip->mailaddr = mailaddr ;
	if (mailaddr[0] == '\0')
		sncpy3(mailaddr,MAILADDRLEN,username,"@",domainname) ;

/* find and open a configuration file (if there is one) */

	if (configfname == NULL)
	    configfname = CONFIGFNAME ;

	rs1 = progconfig_init(pip,configfname) ;

	if (rs1 >= 0) {
	    pip->open.configfile = TRUE ;
	}

/* find anything that we don't already have */

	if (pip->slfname[0] == '\0') {

	    mkpath3(pip->slfname,pip->pr,"var",SLFNAME) ;

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
	    pip->lockint = LOCKINT ;

	if (pip->pidint < 0)
	    pip->pidint = PIDINT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.logfile) ;
#endif

/* logging is normally only for daemon mode */

	if (pip->have.logfile) {

/* log ID */

	    snsd(pip->logid,LOGIDLEN,nodename,(uint) pip->pid) ;

/* log file */

	    if (pip->logfname[0] == '\0')
	        mkpath3(pip->logfname,pip->pr,LOGDNAME,LOGFNAME) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: logfname=%s logid=%s\n",
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

/* can we open the SL file ? */

	rs = perm(pip->slfname,-1,-1,NULL,(R_OK | W_OK)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: perm() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (pip->debuglevel > 0)
	        shio_printf(pip->efp,
	            "%s: sl=%s\n", pip->progname,pip->slfname) ;

	    if (pip->open.logfile)
	        logfile_printf(&pip->lh,"sl=%s", pip->slfname) ;

	    if (pip->f.daemon) {

	        LFM	pidlock ;

		pid_t	pid ;
	        int	cs ;


		if (pip->f.background) {

	        if (pip->open.logfile)
	            logfile_flush(&pip->lh) ;

	        shio_flush(pip->efp) ;

	        rs = uc_fork() ;

	        pid = (pid_t) rs ;
	        if (rs > 0)
	            u_waitpid(pid,&cs,0) ;

		} else
			pid = 0 ;

	        if (pid == 0) {

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
	    	        rs1 = logfile_open(&pip->lh,lf,0,0666,lid) ;
			pip->open.logfile = (rs1 >= 0) ;
		    }

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
	                        debugprintf("main: daemon log\n") ;
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
	                    debugprintf("main: pidname=%s\n",pip->pidfname) ;
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

			    pip->open.pidfile = (rs >= 0) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("main: lfm_start() rs=%d\n",rs) ;
#endif

	                } /* end if */

	                if (rs >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("main: daemon msupdate()\n") ;
#endif

	                    rs = watch(pip,&pidlock) ;

	                    if (rs == SR_AGAIN)
	                        rs = SR_OK ;

	                    if (pip->open.pidfile)
	                        lfm_finish(&pidlock) ;

	                } /* end if (got lock) */

	            } /* end if (grandchild) */

	        } /* end if (child) */

	    } else
	        rs = watch(pip,NULL) ;

	} /* end if (have a SL file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: daemon=%u child=%u\n",
	            pip->f.daemon,f_child) ;
#endif

	    if ((! pip->f.daemon) || f_child) {

	        shio_printf(pip->efp, "%s: entries=%u\n",
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
	    ex = (if_int) ? EX_INTR : EX_OK ;

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

	switch (sn) {

	case SIGCHLD:
		if_child = TRUE ;
		break ;

	default:
		if_int = TRUE ;

	} /* end switch */

}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [slfile] [-slfile file] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,"%s: \t[-slpoll <int>] [-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



