/* main (RSLOWD) */

/* Remote Slow Daemon (RSLOWD) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This is the third or fourth, and hopefully last, major
	implementation of this program.  This program is the server-daemon
	side of a general purpose transport facility.  The 'rslow'
	program is the client half of the thinpip-> This program
	waits for an incoming job and then tries to execute the job by
	looking it up in a job service table.  All, apparently, previous
	versions of this program were really very poor hacks and were
	susceptible to the many little quirks of various UNIX systems
	or UNIX versions.  This program has been rewritten a number of
	times already in the hopes of "getting it right" but all of the
	previous rdebugwrites were largely based on the previously bad
	version to start with.	This version is a complete rdebugwrite
	from scratch !!  This version is a hack also but maybe it will
	last a little longer than the others?


	Environment variables:

		PROGRAMROOT	root of program files

	Synopsis:

	$ rslowd [-C conf] [-polltime] [directory_path] [srvtab] [-V?]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<time.h>
#include	<ftw.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<getxusername.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"jobdb.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"srvfile.h"


/* defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	USERINFO_LEN	(2 * 1024)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getpwd(char *,int) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */

static char	*filereadable() ;


/* local global variabes */


/* local structures */

/* define command option words */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	NULL,
} ;

#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ROOT	3
#define	ARGOPT_LOGFILE	4
#define	ARGOPT_CONFIG	5


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	JOBDB		jobs ;

	bfile		errfile, *efp = &errfile ;
	bfile		logfile ;
	bfile		pidfile ;

	struct ustat		sb ;

	struct proginfo		g, *pip = &g ;

	struct userinfo		u ;

	struct configfile	cf ;

	struct vecelem		sf ;

	struct srventry		*sep ;

	time_t	daytime ;

	struct group		ge, *gp ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k, l ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, srs, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	pwd[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*pr = NULL ;
	char	*configfname = NULL ;
	char	*logfname = NULL ;
	char	*cp ;
	char	*sp ;


	if (u_fstat(3,&sb) >= 0) 
		debugsetfd(3) ;

	if (bopen(efp,BFILE_ERR,"wca",0666) < 0) 
		return BAD ;

	(void) memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	for (i = 0 ; i < 3 ; i += 1) {

	    if (u_fstat(i,&sb) != 0) 
			u_open("/dev/null",O_RDWR,0666) ;

	}


/* initialize */

	pip->f.quiet = FALSE ;
	pip->f.verbose = FALSE ;
	pip->f.interrupt = FALSE ;

	pip->debuglevel = 0 ;

	pip->lfp = &logfile ;

	pip->polltime = -1 ;

	pip->programroot = NULL ;
	pip->username = NULL ;
	pip->groupname = NULL ;
	pip->srvtab = NULL ;
	pip->directory = NULL ;
	pip->interrupt = NULL ;
	pip->lockfile = NULL ;
	pip->pidfile = NULL ;
	pip->tmpdname = DEFTMPDIR ;
	pip->workdir = DEFTMPDIR ;



/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdec(argp + 1,argl - 1,&pip->polltime) < 0))
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

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) pip->tmpdname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->tmpdname = argp ;

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
	                        pip->f.verbose = TRUE ;
	                        break ;

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->programroot = argp ;

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

/* log file */
	                    case ARGOPT_LOGFILE:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) logfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) logfname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                debugprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&pip->debuglevel) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->pidfile = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

	                        akp += 1 ;

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
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 1) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    pip->progname) ;

	if (f_version) bprintf(efp,"%s: version %s\n",
	    pip->progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (pip->debuglevel > 1)
	    bprintf(efp,"%s: debug level %d\n",
	        pip->progname,pip->debuglevel) ;

/* load the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            pip->directory = (char *) argv[i] ;
	            break ;

	        case 1:
	            pip->srvtab = (char *) argv[i] ;
	            break ;

	        default:
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */

/* miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: miscellaneous \n") ;
#endif



/* get our program root */

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv("PROGRAMROOT") ;

	if (pip->programroot == NULL)
	    pip->programroot = PROGRAMROOT ;


/* prepare to store configuration variable lists */

	if ((rs = vecstrinit(&pip->exports,10,0)) < 0) goto badjobinit ;

	if ((rs = vecstrinit(&pip->paths,10,0)) < 0) {

	    vecstrfree(&pip->exports) ;

	    goto badjobinit ;
	}


/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: checking for configuration file\n") ;
#endif

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        "etc/rslowd",DEFCONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        "etc/rslowd",DEFCONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        "etc",DEFCONFIGFILE1) ;

	if (configfname == NULL) {

	    configfname = DEFCONFIGFILE1 ;
	    if (access(configfname,R_OK) < 0)
	        configfname = DEFCONFIGFILE2 ;

	}

/* read in the configuration file if we have one */

	if (access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a configuration file\n") ;
#endif

	    if ((rs = configinit(&cf,configfname)) < 0) goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a good configuration file\n") ;
#endif

	    if ((cf.directory != NULL) && (pip->directory == NULL)) {

	        pip->directory = cf.directory ;
	        cf.directory = NULL ;

	    }

	    if ((cf.interrupt != NULL) && (pip->interrupt == NULL)) {

	        pip->interrupt = cf.interrupt ;
	        cf.interrupt = NULL ;

	    }

	    if ((cf.workdir != NULL) && (pip->workdir == NULL)) {

	        pip->workdir = cf.workdir ;
	        cf.workdir = NULL ;

	    }

	    if ((cf.srvtab != NULL) && (pip->srvtab == NULL)) {

	        pip->srvtab = cf.srvtab ;
	        cf.srvtab = NULL ;

	    }

	    if ((cf.pidfile != NULL) && (pip->pidfile == NULL)) {

	        pip->pidfile = cf.pidfile ;
	        cf.pidfile = NULL ;

	    }

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

	        logfname = cf.logfname ;
	        cf.logfname = NULL ;

	    }

	    if ((cf.polltime > 1) && (pip->polltime < 1))
	        pip->polltime = cf.polltime ;

	    for (i = 0 ; (rs = vecstrget(&cf.exports,i,&sp)) >= 0 ; i += 1)
	        vecstradd(&pip->exports,sp,-1) ;

	    for (i = 0 ; (rs = vecstrget(&cf.paths,i,&sp)) >= 0 ; i += 1)
	        vecstradd(&pip->paths,sp,-1) ;

	    configfree(&cf) ;

	} /* end if */

/* before we go too far, are we the only one on this PID mutex? */

	if (pip->pidfile != NULL) {

	    if (pip->pidfile[0] == '\0') pip->pidfile = DEFPIDFILE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a PIDFILE=%s\n",pip->pidfile) ;
#endif

	    if ((rs = bopen(&pidfile,pip->pidfile,"rwc",0664)) < 0)
	        goto badpidopen ;

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",getpid()) ;

	    bclose(&pidfile) ;

	} /* end if (we have a mutex PID file) */

/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("main: dir=%s\n",pip->directory) ;

	    debugprintf("main: srvtab=%s\n",pip->srvtab) ;

	    debugprintf("main: polltime=%d\n",pip->polltime) ;

	}
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: checking program parameters\n") ;
#endif

	if ((pip->directory == NULL) || (pip->directory[0] == '\0'))
	    pip->directory = DEFDIRECTORY ;

	if (pip->workdir == NULL)
	    pip->workdir = DEFWORKDIR ;

	else if (pip->workdir[0] == '\0')
	    pip->workdir = "." ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((cp = getenv("TMPDIR")) != NULL)
	        pip->tmpdname = cp ;

	    else
	        pip->tmpdname = DEFTMPDIR ;

	}

	if (pip->interrupt != NULL) {

	    if (pip->interrupt[0] != '\0') {

	        if ((access(pip->interrupt,R_OK) < 0) ||
	            (stat(pip->interrupt,&sb) < 0) ||
	            (! S_ISFIFO(sb.st_mode)) || (! S_ISCHR(sb.st_mode))) {

	            unlink(pip->interrupt) ;

	            mktmpfile(pip->interrupt,0666 | S_IFIFO,tmpfname) ;

	            u_chmod(pip->interrupt,0666) ;

	        } /* end if */

	    } else
	        pip->interrupt = DEFINTERRUPT ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: intfile=%s\n",pip->interrupt) ;
#endif

	    if ((access(pip->interrupt,R_OK) >= 0) &&
	        (stat(pip->interrupt,&sb) >= 0) &&
	        (S_ISFIFO(sb.st_mode) || S_ISCHR(sb.st_mode))) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: accessible intfile=%s\n",
			pip->interrupt) ;
#endif

	        pip->f.interrupt = TRUE ;

	    }

	} /* end if (interrupt preparation) */

	if (pip->polltime < 2) pip->polltime = DEFPOLLTIME ;

	pip->maxjobs = DEFMAXJOBS ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("main: dir=%s\n",pip->directory) ;
	    debugprintf("main: polltime=%d\n",pip->polltime) ;
	}
#endif

/* open the system report log file */

#ifdef	COMMENT
	pip->f.log = FALSE ;
	if ((rs = bopen(pip->lfp,logfname,"wca",0664)) >= 0) {

	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	    daytime = time(0L) ;

	    bprintf(lfp,"%s: %s %s started\n",
	        pip->progname,timestr_lodaytime,timebuf),
	        BANNER) ;

	    bflush(pip->lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* can we access the working directory? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: access working directory \"%s\"\n",
		pip->workdir) ;
#endif

	if ((access(pip->workdir,X_OK) < 0) ||
	    (access(pip->workdir,R_OK) < 0)) goto badworking ;


/* what about the queue directory */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: get queue directory \"%s\"\n",pip->directory) ;
#endif

	if (pip->directory[0] != '/') {

	    rs = getpwd(pwd,MAXPATHLEN) ;

	    if (rs < 0) goto badqueue ;

	    mkpath2(tmpfname,pwd,pip->directory) ;

	    pip->directory = malloc_str(tmpfname) ;

	}


/* initialize the job table */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: initializing the job DB \n") ;
#endif

	if (jobdb_init(&jobs,pip->maxjobs,pip->tmpdname) < 0)
	    goto badjobinit ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: intialized the job DB\n") ;
#endif


/* do we have an activity log file? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname != NULL) && (logfname[0] != '\0'))
	    rs = logfile_open(&pip->lh,logfname,0,0666,DEFLOGID) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs < 0) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        pip->programroot,DEFLOGFILE) ;

	    rs = BAD ;
	    if (access(tmpfname,W_OK) >= 0)
	        rs = logfile_open(&pip->lh,tmpfname,0,0666,DEFLOGID) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	}

	userbuf[0] = '\0' ;
	if (rs >= 0) {

	    userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	    pip->nodename = u.nodename ;
	    pip->domainname = u.domainname ;
	    pip->username = u.username ;

	    time(&daytime) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    rs = logfile_printf(&pip->lh,"\n") ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: made log entry, rs=%d\n",rs) ;
#endif

	    logfile_printf(&pip->lh,"\n") ;

	    logfile_printf(&pip->lh,"%s %s started\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	} else {

	    getnodedomain(nodename,domainname) ;

	    pip->nodename = nodename ;
	    pip->domainname = domainname ;

	    getusername(buf,BUFLEN,-1) ;

	    pip->username = malloc_str(buf) ;

	} /* end if (we have a log file) */

	pip->gid = getgid() ;

#ifdef	SYSV
	gp = (struct group *) getgrgid_r(pip->gid,
	    &ge,buf,BUFLEN) ;
#else
	gp = getgrgid(pip->gid) ;
#endif

	if (gp == NULL) {

	    cp = buf ;
	    sprintf(buf,"GUEST-%d",(int) pip->gid) ;

	} else
	    cp = gp->gr_name ;

	pip->groupname = malloc_str(cp) ;


/* process the server table file */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: checking for service table file, srvtab=%s\n",
	    pip->srvtab) ;
#endif

	if ((pip->srvtab == NULL) || (pip->srvtab[0] == '\0')) {

	    cp = filereadable(tmpfname,
	        pip->programroot,"etc/rslowd",DEFSRVFILE1) ;

	    if (cp == NULL)
	        cp = filereadable(tmpfname,
	            pip->programroot,"etc/rslowd",DEFSRVFILE2) ;

	    if (cp == NULL)
	        cp = filereadable(tmpfname,
	            pip->programroot,"etc",DEFSRVFILE1) ;

	    if (cp == NULL)
	        cp = filereadable(tmpfname,
	            NULL,NULL,DEFSRVFILE1) ;

	    if (cp == NULL)
	        cp = filereadable(tmpfname,
	            NULL,NULL,DEFSRVFILE2) ;

	    if (cp != NULL)
	        pip->srvtab = mallocstr(cp) ;

	} /* end if (looking for service table files) */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: service table file=%s\n",pip->srvtab) ;
#endif

	pip->f.srvtab = FALSE ;
	if (access(pip->srvtab,R_OK) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a service table file\n") ;
#endif

	    pip->f.srvtab = TRUE ;
	    if ((rs = srvinit(&sf,pip->srvtab)) < 0) goto badsrv ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a good service table file\n") ;
#endif

#if	CF_DEBUG
	    for (i = 0 ; (rs = vecelemget(&sf,i,&sep)) >= 0 ; i += 1) {

	        if (pip->debuglevel > 1) {

	            debugprintf("main: got a service entry\n") ;

	            debugprintf("main: srvtab service=%s\n",sep->service) ;

	            debugprintf("main: srvtab program=%s\n",sep->program) ;

	            debugprintf("main: srvtab args=%s\n",sep->args) ;

	            debugprintf("main: srvtab username=%s\n",sep->username) ;

	            debugprintf("main: srvtab groupname=%s\n",sep->groupname) ;

	        }

#ifdef	COMMENT
	        if (sep->program != NULL) {

	            if (access(sep->program,X_OK) < 0)
	                goto badsrv ;

	        }
#endif /* COMMENT */

	    } /* end for */
#endif /* CF_DEBUG */

	    logfile_printf(&pip->lh,"srvtab=%s\n",
	        pip->srvtab) ;

	} /* end if (have a service table file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: done w/ srvtab file ?\n") ;
#endif

	if (! pip->f.srvtab) goto badnosrv ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: about to initialize the job table\n") ;
#endif

/* become a daemon program */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: become a daemon?\n") ;
#endif

	bclose(efp) ;

	open("/dev/null",O_RDONLY,0666) ;

	if (pip->debuglevel == 0) {

	    for (i = 0 ; i < 3 ; i += 1) {

			close(i) ;

			open("/dev/null",O_RDONLY,0666) ;

		} /* end for */

	    rs = uc_fork() ;

	    if (rs < 0) {
	        logfile_printf(&pip->lh,"cannot fork daemon (%d)\n",rs) ;
	        uc_exit(1) ;
	    }

	    if (rs > 0)
		uc_exit(0) ;

	    setsid() ;

	} /* end if (becoming a daemon) */

/* we start ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: starting to watch for new jobs\n") ;
#endif

	pip->pid = getpid() ;

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;


	sprintf(buf,"%d.%s",pip->pid,DEFLOGID) ;

	pip->logid = malloc_str(buf) ;

	logfile_setid(&pip->lh,pip->logid) ;

	if (rs == 0)
	    logfile_printf(&pip->lh,"backgrounded\n") ;


/* before we go too far, are we the only one on this PID mutex? */

	if ((pip->pidfile != NULL) && (pip->pidfile[0] != '\0')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a PIDFILE=%s\n",pip->pidfile) ;
#endif

	    if ((srs = bopen(&pidfile,pip->pidfile,"rwc",0664)) < 0)
	        goto badpidfile2 ;

/* capture the lock (if we can) */

	    if ((srs = bcontrol(&pidfile,BC_LOCK,2)) < 0)
	        goto badpidfile2 ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",pip->pid) ;

	    if (userbuf[0] != '\0')
	        bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	            u.nodename,u.domainname,
	            u.username,
	            pip->pid) ;

	    bflush(&pidfile) ;

/* we leave the file open as our mutex lock ! */

	    logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfile) ;

	    logfile_printf(&pip->lh,"PID mutex captured\n") ;

	    bcontrol(&pidfile,BC_STAT,&sb) ;

	    logfile_printf(&pip->lh,"pidfile device=%ld inode=%ld\n",
	        sb.st_dev,sb.st_ino) ;

	    pip->pidfp = &pidfile ;

	} /* end if (we have a mutex PID file) */


	if (userbuf[0] != '\0') {

	    time(&daytime) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&pip->lh,"os=%s pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",pip->pid) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    logfile_printf(&pip->lh,"dir=%s\n",pip->directory) ;

	    if (pip->f.interrupt)
	        logfile_printf(&pip->lh,"intfile=%s\n",
	            pip->interrupt) ;

	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestr_log(daytime,timebuf)) ;

	} /* end if (making log entries) */


	srs = watch(&sf,&jobs) ;


/* we are done */
daemonret:

#if	CF_DEBUG
	if (pip->debuglevel < 0)
	    bprintf(efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfree(&cf) ;

	vecelemfree(&sf) ;

	jobdb_free(&jobs) ;

	if (pip->lfp != NULL) bclose(pip->lfp) ;

	return srs ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bclose(efp) ;

	return BAD ;

badretlog:
	bclose(pip->lfp) ;

	goto badret ;

/* USAGE> dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-polltime] [directory] [srvtab] [-?v] ",
	    pip->progname,pip->progname) ;

	bprintf(efp,"[-D[=n]]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badret ;

badworking:
	bprintf(efp,"%s: could not access the working directory \"%s\"\n",
	    pip->progname,pip->workdir) ;

	goto badret ;

badqueue:
	bprintf(efp,"%s: could not process the queue directory\n",
	    pip->progname) ;

	goto badret ;

badjobinit:
	bprintf(efp,"%s: could not initialize list structures (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badsrv:
	bprintf(efp,"%s: bad service table file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badnosrv:
	bprintf(efp,"%s: no service table file specified\n",
	    pip->progname) ;

	goto badret ;

baddir:
	bprintf(efp,"%s: bad working directory specified (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	bprintf(efp,
	    "%s: could not open the PID file \"%s\" rs=%d\n",
	    pip->progname,pip->pidfile,rs) ;

	goto badret ;

badpidlock:
	if (! pip->f.quiet) {

	    bprintf(efp,
	        "%s: could not lock the PID file \"%s\" rs=%d\n",
	        pip->progname,pip->pidfile,rs) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(efp,"%s: pidfile> %W",
	            pip->progname,
	            buf,len) ;

	    }

	    bclose(&pidfile) ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto badret ;

badpidfile2:
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret ;

}
/* end subroutine (main) */


/* is a file readable */
static char *filereadable(tmpfname,dir1,dir2,fname)
char	tmpfname[], dir1[], dir2[], fname[] ;
{


	if ((dir1 != NULL) && (dir2 != NULL))
	    sprintf(tmpfname,"%s/%s/%s",
	        dir1,dir2,fname) ;

	else if (dir1 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir1,fname) ;

	else if (dir2 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir2,fname) ;

	else
	    strcpy(tmpfname,fname) ;

	if (u_access(tmpfname,R_OK) >= 0)
	    return tmpfname ;

	return NULL ;
}
/* end subroutine (filereadable) */



