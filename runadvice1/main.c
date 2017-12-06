/* main (Run Advice) */

/* program to run ADVICE on varying paramter sets */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	- 1993-10-01, David A­D­ Morano
	This program was originally written.

	- 1996-02-01, David A­D­ Morano
        This program was pretty extensively modified to take much more flexible
        combinations of user supplied paramters.

*/

/* Copyright © 1993-1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will run the SysCAD Advice program under controlled
        substitution of various parameters in the ADVICE input and circuit
        files.

	Synopsis:

	$ runadvice [runfile [outfile]] [-DV] [-o output]
		[-proc process] [-temp temperature]
		[-timestep step_time] [-timelen length_time]
		[-param POW=3.3]


	Program flow:

	+ The 'main' routine calls 'slave' if it determines that
	it is running in SLAVE mode.

	+ The 'main' routine calls 'runlocal' to run a local ADVICE.

	+ The 'main' routine calls 'runremote' to run a remote ADVICE.

	+ The 'runlocal' subroutine calls 'perform' to do the actual run.

	+ The 'slave' subroutine calls 'perform' to do the actual run.

	+ The 'runremote' subroutine arranges to run a remote 'slave'.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"varsub.h"
#include	"paramopt.h"
#include	"configfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		((2*LINEBUFLEN)+MAXPATHLEN)
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdouble() ;
extern int	bufprintf(const char *,...) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern int	vsainit(), vsaadd(), vsaget(), vsafree() ;

extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;

extern int	runinit(), runcount(), runadd(), runwait(), rundel() ;
extern int	machineinit(), machineadd(), machineupdate() ;
extern int	machinebest(), machinemin() ;

extern char	*timestr_log(time_t,char *) ;

extern void	vsaprint() ;
extern void	fixdisplay() ;


/* forward references */

static int	loadok() ;

static void	int_signal() ;
static void	int_pipe() ;


/* external variables */


/* global data */

struct global		g ;

USERINFO		u ;


/* local data structures */

struct defparam {
	char	*paramname ;
	char	*paramvalue ;
} ;

static const struct defparam	dpa[] = {
	{ "PROC", DEF_PROC },
	{ "TEMP", DEF_TEMP },
	{ "TIMESTEP", DEF_TIMESTEP },
	{ "TIMELEN", DEF_TIMELEN },
	{ "POW", DEF_POW },
	{ "TECH", DEF_TECHNOLOGY },
	{ NULL, NULL }
} ;

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ADVICE",
	"param",
	"proc",
	"pow",
	"timestep",
	"timelen",
	"temp",
	"CONFIG",
	"SLAVE",
	"ROOT",
	NULL,
} ;


#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ADVICE	3
#define	ARGOPT_PARAM	4
#define	ARGOPT_PROC	5
#define	ARGOPT_POW	6
#define	ARGOPT_TIMESTEP	7
#define	ARGOPT_TIMELEN	8
#define	ARGOPT_TEMP	9
#define	ARGOPT_CONFIG	10
#define	ARGOPT_SLAVE	11
#define	ARGOPT_ROOT	12


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct sigaction	ss ;
	struct tm		ts, *timep ;
	struct machine		*mp ;
	struct paramname	*pp ;
	struct paramvalue	*vp ;
	struct confighead	ch ;
	bfile		infile, *ifp = &infile ;
	bfile		runfile, *rfp = &runfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	bfile		*fpa[3] ;
	sigset_t	signalmask ;
	time_t		daytime = 0 ;
	machinehead	mh ;
	vecitem		rh ;
	varsub		vsh ;
	double		dvalue ;
	long		loadaverage, loadspare ;

	const int	llen = LINEBUFLEN ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j ;
	int	rs = SR_OK ;
	int	len, l ;
	int	argnum = -1 ;
	int	n ;
	int	childstat ;
	int	nslave = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_puttmp = TRUE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_removerun = FALSE ;
	int	f_rundone ;
	int	f_runremote ;
	int	f_interactive = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*configfname = NULL ;
	const char	*cp, *sp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXNAMELEN + 1] ;
	char	tmprunfname[MAXPATHLEN + 1] ;
	char	tmpconfname[MAXPATHLEN + 1] ;
	char	tmpparamfname[MAXPATHLEN + 1] ;
	char	tmpmaifname[MAXPATHLEN + 1] ;
	char	tmpofname[MAXPATHLEN + 1] ;
	char	cmd[(MAXPATHLEN * 2) + 1] ;

	memset(&g,0,sizeof(struct global)) ;

	sfbasename(argv[0],-1,&cp) ;
	g.progname = cp ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) < 0) 
		bcontrol(efp,BC_LINEBUF,0) ;

	g.f.debug = FALSE ;
	g.f.verbose = FALSE ;

/* some other global initialization */

	g.f.machines = FALSE ;

	g.programroot = getenv("RUNADVICE_PROGRAMROOT") ;

	if (g.programroot == NULL)
		g.programroot = getenv("PROGRAMROOT") ;

	if (g.programroot == NULL)
		g.programroot = PROGRAMROOT ;

	
	g.envv = envv ;
	g.cwd = NULL ;
	if (((cp = getenv(VARPWD)) != NULL) && (access(cp,X_OK) == 0))
		g.cwd = mallocstrw(cp,-1) ;

	if (g.cwd == NULL)
	    g.cwd = getcwd(NULL,0) ;

	g.runfname = DEF_RUNFNAME ;
	g.ofname = DEF_STDOUTFNAME ;
#ifdef	COMMENT
	g.confname = DEF_CONCKT ;
	g.paramfname = DEF_PARAMS ;
	g.maifname = DEF_MAINCKT ;
#else
	g.confname = NULL ;
	g.paramfname = NULL ;
	g.maifname = NULL ;
#endif

	g.efp = efp ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.plist = NULL ;
	g.tmpdir = NULL ;

	g.prog_advice = PROG_ADVICE ;
	if ((cp = getenv("RUNADVICE_ADVICE")) != NULL)
	    g.prog_advice = cp ;

/* other initialization */

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

#ifdef	COMMENT
	configfname = "config.ra" ;
#endif
	if ((cp = getenv("RUNADVICE_CONFIG")) != NULL)
	    configfname = cp ;


/* process program arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

			argnum = 0 ;
	                if (((argl - 1) > 0) && 
				(cfdeci(argp + 1,argl - 1,&argnum) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.tmpdir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.tmpdir = argp ;

	                        }

	                        break ;

/* version */
	                    case ARGOPT_VERSION:
#if	CF_DEBUGS
	                        debugprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal) goto badargextra ;

	                        break ;

/* verbose mode */
	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* what is the ADVICE program to run */
	                    case ARGOPT_ADVICE:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.prog_advice = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.prog_advice = argp ;

	                        }

	                        break ;

/* configuration file */
	                    case ARGOPT_CONFIG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) configfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

	                        }

	                        break ;

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* slave mode operation */
	                    case ARGOPT_SLAVE:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				nslave = -1 ;
	                            if ((avl > 0) &&
					(cfdeci(avp,avl,&nslave) < 0))
	                                goto badargvalue ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

				nslave = -1 ;
					if ((argl > 0) &&
	                            (cfdeci(argp,argl,&nslave) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

/* we have a parameter */
	                    case ARGOPT_PARAM:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloadu(&g.plist,
	                                    avp,avl) ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloadu(&g.plist,
	                                    argp,argl) ;

	                        }

	                        break ;

/* process */
	                    case ARGOPT_PROC:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloads(&g.plist,
	                                    "PROC",avp,avl) ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloads(&g.plist,
	                                    "PROC",argp,argl) ;

	                        }

	                        break ;

/* temperature */
	                    case ARGOPT_TEMP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloads(&g.plist,
	                                    "TEMP",avp,avl) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloads(&g.plist,
	                                    "TEMP",argp,argl) ;

	                        }

	                        break ;

/* power (pow) */
	                    case ARGOPT_POW:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloads(&g.plist,
	                                    "POW",avp,avl) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloads(&g.plist,
	                                    "POW",argp,argl) ;

	                        }

	                        break ;

/* time step */
	                    case ARGOPT_TIMESTEP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloads(&g.plist,
	                                    "TIMESTEP",avp,avl) ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloads(&g.plist,
	                                    "TIMESTEP",argp,argl) ;

	                        }

	                        break ;

/* time length */
	                    case ARGOPT_TIMELEN:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                paramloads(&g.plist,
	                                    "TIMELEN",avp,avl) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramloads(&g.plist,
	                                    "TIMELEN",argp,argl) ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            g.progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

/* debug */
	                        case 'D':
	                            g.f.debug = TRUE ;
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                debugprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
						avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
					(cfdeci(avp,avl,&g.debuglevel) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* input run file */
	                        case 'r':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.runfname = argp ;

	                            break ;

/* output file */
	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                g.ofname = argp ;

	                            break ;

/* length of time */
	                        case 'l':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    paramloads(&g.plist,"TIMELEN",
						avp,avl) ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl) {
	                                    paramloads(&g.plist,
	                                        "TIMELEN",argp,argl) ;
	                                }
	                            }

	                            break ;

/* time step */
	                        case 's':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) paramloads(&g.plist,
	                                    "TIMESTEP",avp,avl) ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl) {
	                                    paramloads(&g.plist,
	                                        "TIMESTEP",argp,argl) ;
	                                }
	                            }

	                            break ;

/* temperature */
	                        case 't':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    paramloads(&g.plist,"TEMP",
						avp,avl) ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl) {
	                                    paramloads(&g.plist,
	                                        "TEMP",argp,argl) ;
	                                }
	                            }

	                            break ;

/* processing */
	                        case 'p':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    paramloads(&g.plist,"PROC",
						avp,avl) ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl) {
	                                    paramloads(&g.plist,
	                                        "PROC",argp,argl) ;
	                                }
	                            }

	                            break ;

/* verbose mode */
	                        case 'v':
	                            g.f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                g.progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (g.f.debug) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    g.progname) ;

	if (f_version) bprintf(efp,"%s: version %s\n",
	    g.progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (g.f.debug || (g.debuglevel > 0))
	    bprintf(g.efp,"%s: debug level %d\n",
	        g.progname,g.debuglevel) ;

/* load the positional arguments */

#if	CF_DEBUG
	if (g.f.debug) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.f.debug) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            g.runfname = (char *) argv[i] ;
	            break ;

	        case 1:
	            g.ofname = (char *) argv[i] ;
	            break ;

	        default:
	            bprintf(g.efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


/* set up the signal handlers */

	g.f_signal = FALSE ;
	uc_sigsetempty(&signalmask) ;

	ss.sa_handler = int_signal ;
#ifdef	SYSV
	ss.sa_sigaction = int_signal ;
#endif
	ss.sa_mask = signalmask ;
#ifdef	SYSV
	ss.sa_flags = SA_RESTART ;
#else
	ss.sa_flags = 0 ;
#endif

	sigaction(SIGINT,&ss,NULL) ;

	sigaction(SIGTERM,&ss,NULL) ;

	sigaction(SIGHUP,&ss,NULL) ;

/* handle bad pipe writes a little differently */

	ss.sa_handler = int_pipe ;
#ifdef	SYSV
	ss.sa_sigaction = int_pipe ;
#endif
	sigaction(SIGPIPE,&ss,NULL) ;

	bflush(g.efp) ;

/* other general initialization */

	userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

/* open the program output file */

	rs = bopen(ofp,BFILE_STDOUT,"wca",0666) ;
	if (rs < 0)
		goto badoutopen ;

	bcontrol(ofp,BC_LINEBUF,0) ;

/* this is a hack and not really supported by the library interface */

	bcontrol(ofp,BC_SETFL,O_APPEND) ;


/* get the log file name */

	mkpath3(tmpfname,g.programroot,"log",LOGFNAME) ;

	g.logfname = mallocstrw(tmpfname,-1) ;

/* if we are a slave, leave now (take a program control flow detour) */

	if (nslave > -1) {

#if	CF_DEBUG
	debugprintf("main: we are a slave=%d\n",nslave) ;
#endif


/* make sure that we are a "real" slave program */

	    if ((cp = getenv("RUNMODE")) == NULL) goto badslave ;

/* fix up where the TMPDIR is supposed to be (as a slave) */

	    if (g.tmpdir == NULL)
	        g.tmpdir = getenv("TMPDIR") ;

	    if (g.tmpdir == NULL)
	        g.tmpdir = "/tmp" ;

/* open the program's standard input to talk to parent */

	    g.f.machines = TRUE ;
	    if ((rs = bopen(ifp,BFILE_STDIN,"r",0666)) < 0)
	        goto badinopen ;

/* do it */

	    rs = slave(nslave,ifp,u.pid) ;

	    bclose(ifp) ;

	daytime = time(NULL) ;

	logfile_printf(&g.lh,"%s S=%03d exiting\n",
	    timestr_log(daytime,buf),nslave) ;

#if	CF_DEBUG
	debugprintf("main: slave=%d is exiting rs=%d\n",nslave,rs) ;
#endif

	    goto completed ;

	} /* end if (slave processing) */


/* make the first log entry */

	g.logid = u.logid ;
	if ((rs = logfile_open(&g.lh,g.logfname,0,0666,g.logid)) < 0)
	    bprintf(g.efp,"%s: could not open the log file (rs %d)\n",
	        g.progname,rs) ;

	daytime = (time_t) time(NULL) ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &daytime) ;
#endif

	logfile_printf(&g.lh,"%02d%02d%02d %02d%02d:%02d M     %-14s %s/%s\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec,
	    g.progname,(g.f.sysv_ct) ? "SYSV" : "BSD",VERSION) ;

	buf[0] = '\0' ;
	if (u.mailname != NULL) 
		bufprintf(buf,BUFLEN,"(%s)",u.mailname) ;

	logfile_printf(&g.lh,"%s!%s %s\n",
	    u.nodename,u.username,buf) ;

/* non-slave initialization */

	fixdisplay(u.nodename) ;

	if (isatty(1)) 
		f_interactive = TRUE ;

	logfile_printf(&g.lh,"mode=%s\n",
	    f_interactive ? "interative" : "batch") ;


/* initialize a machine list */

	if ((rs = machineinit(&mh,12)) > 0)
	    goto badmachine ;


/* process a configuration file, if there is one */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to evaluate config file \"%s\"\n",
	        configfname) ;
#endif

	if (configfname == NULL) {

		configfname = "config.ra" ;
		if (access(configfname,R_OK) != 0) {

			mkpath2(buf, g.programroot, "lib/runadvice/config.ra") ;

			configfname = mallocstrw(buf,-1) ;

		}
	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to evaluate config file \"%s\"\n",
	        configfname) ;
#endif

	if ((configfname != NULL) &&
		(access(configfname,R_OK) == 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: we got a config file \"%s\"\n",
	            configfname) ;
#endif

	    if ((rs = configread(&ch,configfname)) >= 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: we read a config file \n") ;
#endif

/* handle substitutable parameters */

	        for (i = 0 ; 
	            (rs = vecstrget(&ch.defaults,i,&sp)) >= 0 ; i += 1) {

	            if (sp == NULL) continue ;

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("main: we got a parameter \"%s\"\n",sp) ;
#endif

	            if ((cp = strchr(sp,'=')) != NULL) {

	                *cp++ = '\0' ;
	                if (paramfind(&g.plist,sp,&pp) < 0)
	                    paramloads(&g.plist,sp,cp,-1) ;

	            }

	        } /* end for */

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to handle exports\n") ;
#endif

/* handle environment variables */

	        for (i = 0 ; 
	            (rs = vecstrget(&ch.exports,i,&sp)) >= 0 ; i += 1) {

	            if (sp == NULL) continue ;

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("main: we got an export \"%s\"\n",sp) ;
#endif

	            strcpy(buf,sp) ;

	            if ((cp = strchr(buf,'=')) != NULL) {

	                *cp++ = '\0' ;
	                if (getenv(buf) == NULL) {

	                    cp = mallocstrw(sp,-1) ;

	                    putenv(cp) ;

	                }
	            }

	        } /* end for */

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to handle TMPDIR \n") ;
#endif

/* handle TMPDIR */

	        if ((g.tmpdir == NULL) && (ch.tmpdir != NULL) &&
	            ((cp = strchr(ch.tmpdir,'=')) != NULL))
	            g.tmpdir = mallocstrw((cp + 1),-1) ;

/* what about a default ADVICE program ? */

	        if ((g.prog_advice == NULL) && (ch.advice != NULL) &&
	            ((cp = strchr(ch.advice,'=')) != NULL))
	            g.prog_advice = mallocstrw((cp + 1),-1) ;

/* ADVICE files */

	        if ((g.confname == NULL) && (ch.control != NULL) &&
	            ((cp = strchr(ch.control,'=')) != NULL))
	            g.confname = mallocstrw((cp + 1),-1) ;

	        if ((g.paramfname == NULL) && (ch.params != NULL) &&
	            ((cp = strchr(ch.params,'=')) != NULL))
	            g.paramfname = mallocstrw((cp + 1),-1) ;

	        if ((g.maifname == NULL) && (ch.main != NULL) &&
	            ((cp = strchr(ch.main,'=')) != NULL))
	            g.maifname = mallocstrw((cp + 1),-1) ;


/* did we get any "machine" entries ? */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: got %d machine entries\n",
	                ch.machines.i) ;
#endif

	        for (i = 0 ; 
	            vecstrget(&ch.machines,i,&sp) >= 0 ; i += 1) {

	            if (sp == NULL) continue ;

	            if ((cp = strchr(sp,'=')) != NULL) {

	                *cp++ = '\0' ;
#if	CF_DEBUG
	                    if (g.debuglevel > 1)
	                        debugprintf("main: string=%s\n",
	                            cp) ;
#endif

#if	CF_DEBUG
		debugprintf("main: float=%6.3f\n",(double) 2.51) ;
#endif

	                if ((*cp != '\0') &&
	                    (cfdouble(cp,-1,&dvalue) >= 0)) {

#if	CF_DEBUG
	                    if (g.debuglevel > 1)
	                        debugprintf("main: processed machine %s %s\n",
	                            sp,cp) ;
#endif

#if	CF_DEBUG
		debugprintf("main: dvalue=%6.3f\n",(double) dvalue) ;
#endif

	                    if ((rs = machineadd(&mh,sp,(double) dvalue)) < 0)
	                        bprintf(g.efp,
	                            "%s: failed on machine \"%s\" (rs %d)\n",
	                            g.progname,sp,rs) ;

	                    else
	                        g.f.machines = TRUE ;

#if	CF_DEBUG	
	if ((g.debuglevel > 1) && (rs >= 0)) 
		debugprintf("main: m=%s a=%d\n",sp,
		((struct machine *) (mh.va[rs]))->sp->avenrun[0]) ;
#endif

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

	                } /* end if (machine entry has a good loadave) */

	            } /* end if (have a well formatted machine entry) */

	        } /* end for (looping through machine entries) */

	    } else {

	        bprintf(g.efp,
	            "%s: error in processing the configuration file \"%s\"\n",
	            g.progname,configfname) ;

	        bprintf(g.efp,
	            "%s: configuration file error, rs=%d, srs=%d, line=%d\n",
	            g.progname,rs,ch.srs,ch.badline) ;

	    } /* end if (processing a configuration file) */

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: freeing up config file stuff\n") ;
#endif

	    configfree(&ch) ;

	} /* end if (have a configuration file) */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: where is TMPDIR \n") ;
#endif


#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* where is the TMP directory */

	if (g.tmpdir == NULL) {

	    g.tmpdir = "/tmp" ;
	    if ((cp = getenv(VARTMPDNAME)) != NULL) {

	        g.tmpdir = cp ;
	        f_puttmp = FALSE ;
	    }
	}

	if (f_puttmp) {

	    bufprintf(buf,BUFLEN,"TMPDIR=%s",g.tmpdir) ;

	    cp = mallocstrw(buf,-1) ;

	    if (putenv(cp) != 0) 
		bprintf(g.efp,
	        "%s: could not add TMPDIR to environment\n",
	        g.progname) ;

	}

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* what is the "run" file that we will be using ? */

	if ((g.runfname == NULL) || (g.runfname[0] == '-') || 
	    (g.runfname[0] == '\0')) {

	    rs = bopen(ifp,BFILE_STDIN,"r",0666) ;
	    if (rs < 0)
	        goto badinopen ;

	    cp = "run1" ;
#ifdef	COMMENT
	    mkpath2(tmpfname, g.tmpdir, "raXXXXXXXX.adv") ;
#else
	    mkpath1(tmpfname,"raXXXXXXXX.adv") ;
#endif

	    rs = mktmpfile( g.runfname, 0664, tmpfname) ;
	    if (rs < 0)
	        goto badtmpfile ;

	    f_removerun = TRUE ;
	    if ((rs = bopen(rfp,g.runfname,"r",0666)) < 0)
	        goto badrunopen ;

/* copy the whole file over */

	    while ((len = bcopyblock(ifp,rfp,1024)) > 0) ;

	    bclose(ifp) ;

	    bclose(rfp) ;

	    if (len < 0) goto badcopy ;

	} /* end if (reading standard input as the "run" file ) */

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* perform a final check on the ADVICE data output file name */

	if ((g.ofname == NULL) || (g.ofname[0] == '\0') || 
	    (g.ofname[0] == '-'))
	    g.ofname = DEF_OUTFILE ;


/* file name defaults */

	if (g.confname == NULL)
	    g.confname = DEF_CONCKT ;

	if (g.paramfname == NULL)
	    g.paramfname = DEF_PARAMS ;

	if (g.maifname == NULL)
	    g.maifname = DEF_MAINCKT ;

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* add default parameters where they were not specified by the user */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to add default parameters\n") ;
#endif

	for (i = 0 ; dpa[i].paramname != NULL ; i += 1) {

	    if (paramfind(&g.plist,dpa[i].paramname,&pp) < 0) {

	        paramload(&g.plist,dpa[i].paramname,dpa[i].paramvalue) ;

	    }

	} /* end for */

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* print out the accumulated run parameter information */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to print out variables\n") ;
#endif

	    n = 1 ;
	    i = 0 ;
	    for (pp = g.plist ; pp != NULL ; pp = pp->next) {

	        if (pp->top != NULL) pp->current = pp->top ;

	if (g.f.verbose || (g.debuglevel > 1))
	        bprintf(efp,"%s: p=\"%s\"\n",
	            g.progname,pp->name) ;

	        j = 0 ;
	        for (vp = pp->top ; vp != NULL ; vp = vp->next) {

	if (g.f.verbose || (g.debuglevel > 1))
	            bprintf(efp,"%s: value \"%s\"\n",
	                g.progname,vp->value) ;

	            j += 1 ;

	        } /* end for (inner) */

	        i += 1 ;
	        n = n * j ;

	    } /* end for (outer) */

	if (g.f.verbose || (g.debuglevel > 1))
	    bprintf(efp,"%s: parameters=%d, combinations=%d\n",
	        g.progname,i,n) ;

	bflush(g.efp) ;

	logfile_printf(&g.lh,"combinations %d\n",n) ;

/* get the total number of parameters */

	g.nparams = 0 ;
	for (pp = g.plist ; pp != NULL ; pp = pp->next)
	    g.nparams += 1 ;

#if	CF_DEBUG && 0
	                    if (g.debuglevel > 1)
	                        debugprintf("main: a.f_rsh=%d\n",
	                            ((struct machine *) (mh.va[0]))->f.rsh) ;
#endif

/* print out the machine status information */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to print out machine information\n") ;
#endif

	if (g.f.verbose || (g.debuglevel > 1)) {

	    if (g.f.machines) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: machine elems %d\n",mh.i) ;
#endif

	        for (i = 0 ; vecitem_get(&mh,i,&mp) >= 0 ; i += 1) {

	            if (mp == NULL) continue ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: machine pointer is non-null\n") ;
#endif

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: machine %s\n",mp->hostname) ;
#endif

	            if (! mp->f.rstat)
	                bprintf(g.efp,"%s: machine \"%s\" may be down\n",
	                    g.progname,mp->hostname) ;

	            else if (g.debuglevel > 1) {

	                bprintf(g.efp,
	                    "%s: machine \"%s\" loadaverage (scaled) %d\n",
	                    g.progname,mp->hostname,mp->sp->avenrun[0]) ;

	            }

	            if (! mp->f.freepages) bprintf(g.efp,
	                "%s: machine \"%s\" has possible permissions problem\n",
	                g.progname,mp->hostname) ;

	            else if (g.debuglevel > 1)
	                bprintf(g.efp,
	                    "%s: machine \"%s\" free pages %d\n",
	                    g.progname,mp->hostname,mp->freepages) ;


	        } /* end for */

	    } else
	        bprintf(g.efp,
	            "%s: using the local machine for execution(s)\n",
	            g.progname) ;

	} /* end if (print machine information) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: machine information printed\n") ;
#endif

	bflush(g.efp) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to loop\n") ;
#endif

/* initialize the run header structure */

	if ((rs = runinit(&rh,10)) < 0) goto badruninit ;


/* now we generate slaves! */

	nslave = 0 ;

/* go through the loops */
loop:
	if (g.f_signal) goto signaled ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: loop count %d\n",nslave) ;
#endif

	f_rundone = FALSE ;
	f_runremote = TRUE ;
	if (g.f.machines) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: running remote ?\n") ;
#endif

/* find the best machine to use */

	    if (f_runremote && 
		(machinebest(&mh,&mp,&loadspare,&loadaverage) < 0))
	        f_runremote = FALSE ;

#if	CF_DEBUG
	else
		debugprintf("main: best host=%s\n",mp->hostname) ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: after best f_runremote=%d\n",f_runremote) ;
#endif

	    if (f_runremote && 
		((! loadok(f_interactive,loadspare)) ||
		(runcount(&rh) >= MAXJOBS))) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: inside not suitable f_i=%d loadspare=%ld count=%d\n",
		f_interactive,loadspare,runcount(&rh)) ;
#endif

/* wait for one of the existing runs to finish or time out */

	        if (runwait(&rh,12) < 0) {

/* only execute this if there were none out to start with */

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: no runs were outstanding\n") ;
#endif

	            for (i = 0 ; i < 15 ; i += 1) {

	                if (g.f_signal) goto signaled ;

	                sleep(2) ;

	            }

	        } /* end if (no runs were outstanding) */

/* update the machine stats */

	        sleep(2) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: about to update the machine status\n") ;
#endif

	        machineupdate(&mh) ;

	        goto loop ;

	    } /* end if (no suitable machine is ready) */

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: after not suitable f_rundone=%d\n",f_rundone) ;
#endif

/* if we found a machine that is ready, pump it! (with a run) */

	    if (f_runremote) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: about to 'runremote'\n") ;
#endif

	        f_rundone = TRUE ;
	        if ((rs = runremote(nslave,mp->hostname)) < 0)
	            goto badrun ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: returned from 'runremote' rs=%d\n",rs) ;
#endif

/* the spawn PID is in variable 'rs' */

	        runadd(&rh,mp->hostname,rs) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: returned from 'runadd'\n") ;
#endif

/* bias this machine negatively */

		machinemin(&mh,mp->hostname,loadaverage + 256,30) ;

		sleep(2) ;

	        machineupdate(&mh) ;

	    } /* end if (actually running remotely) */

	} /* end if (running remotely) */

/* if all else fails, run it locally */

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: before local f_rundone=%d\n",f_rundone) ;
#endif

	if (! f_rundone) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: running local\n") ;
#endif

	logfile_printf(&g.lh,"running locally on node=%s\n",u.nodename) ;

	    if ((rs = runlocal(nslave,u.pid)) < 0) goto badrun ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: back from local\n") ;
#endif

	} /* end if (running locally) */


/* increment to the next run parameters, and check for completion */

	nslave += 1 ;
	if ((rs = paramincr(g.plist)) >= 0) goto loop ;


/* wait for stuff to complete if we have stuff out */

	if (g.f.machines) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: waiting for runs to complete\n") ;
#endif

	    while ((rs = runwait(&rh,3600)) > 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: inside waiting loop, still waiting -> rs=%d\n",rs) ;
#endif

		sleep(2) ;

	        if (g.f_signal) goto signaled ;

		}

	} /* end if (waiting for outstanding runs to complete) */

/* end of looping on parameters */


	logfile_printf(&g.lh,"%s M     exiting\n",
	    timestr_log(daytime,buf)) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: completing\n") ;
#endif


/* end of looping of the parameters */
completed:
	if ((g.debuglevel > 0) || g.f.verbose)
	    bprintf(g.efp,"%s: completed\n",g.progname) ;

signaled:
	if (g.f_signal) {

	logfile_printf(&g.lh,"%s M     exiting due to a signal\n",
	    timestr_log(daytime,buf)) ;

	    bprintf(g.efp,"%s: caught a signal\n",
	        g.progname) ;

	}

	if (f_removerun) unlink(g.runfname) ;

/* and the temporary files */

	if (tmprunfname[0] != '\0') unlink(tmprunfname) ;

	if (tmpconfname[0] != '\0') unlink(tmpconfname) ;

	if (tmpparamfname[0] != '\0') unlink(tmpparamfname) ;

	if (tmpmaifname[0] != '\0') unlink(tmpmaifname) ;

	if (tmpofname[0] != '\0') unlink(tmpofname) ;

exit:
	bclose(g.ofp) ;

earlyret:
	bclose(g.efp) ;

	return OK ;

badret:
	if (f_removerun) unlink(g.runfname) ;

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

/* program invocation usage */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [runfile [outfile]]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t\t[-o outputfile] [-DV]\n") ;

	bprintf(efp,
	    "\t\t[-timestep timestep(s)] [-timelen timelen(s)]\n") ;

	bprintf(efp,
	    "\t\t[-temp temperature(s)] [-proc process(es)]\n") ;

	bprintf(efp,"\t\t[-pow voltage]\n") ;

	bprintf(efp,"\n") ;

	bprintf(efp,
	    "\trunfile\t\t\tADVICE run file\n") ;

	bprintf(efp,
	    "\toutfile\t\t\toutput file\n") ;

	bprintf(efp,
	    "\t-proc process_value_list\n") ;

	bprintf(efp,
	    "\t-temp termperature_value_list\n") ;

	bprintf(efp,
	    "\t-pow power_supply_voltage_value_list\n") ;

	bprintf(efp,
	    "\t-D\t\t\tdebugging flag\n") ;

	bprintf(efp,
	    "\t-V\t\t\tprogram version\n") ;

	bprintf(efp,"\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad input argument specified\n",
	    g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: extra argument supplied when not expected\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badrunopen:
	bprintf(efp,"%s: could not open run file \"%s\" (rs %d)\n",
	    g.progname,g.runfname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    g.progname,g.ofname,rs) ;

	goto badret ;

badalloc:
	bprintf(efp,
		"%s: couldn't allocate 'varsub' data structure (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badrun:
	bprintf(efp,
		"%s: a run attempt returned bad (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(efp,"%s: could not open input file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badtmpfile:
	bprintf(efp,"%s: could not create temporary (%s) file (rs %d)\n",
	    g.progname,cp,rs) ;

	goto badret ;

badsubfile:
	bprintf(efp,"%s: bad file substitution (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badcopy:
	bprintf(g.efp,
	    "%s: bad copy of input file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badspawn:
	bprintf(g.efp,"%s: could not spawn ADVICE program (rs %d)\n",
	    g.progname,rs) ;

	goto completed ;

badmachine:
	bprintf(g.efp,
	    "%s: could not initialize the machine list (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badruninit:
	bprintf(g.efp,
	    "%s: could not initialize the runlist (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badslave:
	bprintf(g.efp,"%s: this was not a properly executed SLAVE\n") ;

	goto completed ;
}
/* end subroutine (main) */


/* signal handlers */
static void int_signal(signo)
int	signo ;
{

	g.f_signal = TRUE ;

}


static void int_pipe(signo)
int	signo ;
{

	g.f_pipe = TRUE ;

}
/* end signal handlers */


/* is the provided load OK ? */
static int loadok(f_i,spare)
int		f_i ;
long		spare ;
{

	if (f_i) {

		if (spare > (-2 * 256)) return TRUE ;

	} else {

		if (spare > 0) return TRUE ;

	}

	return FALSE ;
}
/* end subroutine (loadok) */




