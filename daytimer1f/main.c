/* main */

/* main subroutine for the day timer program */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_TERMDEVICE	1
#define	CF_TTYNAMER	1
#define	CF_EXPERIMENT	0


/* revision history :

	= 1988-02-01, David A­D­ Morano

	This subroutine was originally written.


	= 1988-02-20, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We execute the program with an invocation as :

	$ daytimer [[mailfile] offset] [-o offset] [-sV] [-t timeout]


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mkdev.h>
#include	<sys/utsname.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"mailfiles.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)
#define	BUFLEN		MAXPATHLEN

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	LINENAMELEN
#define	LINENAMELEN	MAXPATHLEN
#endif


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	termdevice(char *,int,int) ;
extern int	isdigitlatin(int) ;

extern int	daytimer(struct proginfo *,struct userinfo *,LFM *,
			time_t,time_t,time_t,int,MAILFILES *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	daytimer_makedate[] ;


/* local structures */


/* forward references */

void	helpfile() ;


/* global data */


/* local variables */

static char *const argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct ustat	sb ;

	bfile		errfile, *efp = &errfile ;
	bfile		pidfile ;

	MAILFILES	mfs ;

	LFM		lk ;

	USERINFO	u ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	ex = EX_INFO ;
	int	rs, len, sl  ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help ;
	int	f_statdisplay ;
	int	timeoff = DECF_OFFSET ;
	int	timeout = DECF_TIMEOUT ;
	int	refresh = DECF_REFRESH ;
	int	tfd = FD_STDOUT ;
	int	fd_debug ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	linenamebuf[MAXPATHLEN + 1] ;
	char	lockfname[MAXPATHLEN + 1] ;
	char	pidfname[MAXPATHLEN + 1] ;
	char	timebuf[100] ;
	char	*lockdir = LOCKDIR ;
	char	*piddir = PIDDIR ;
	char	*logfname = NULL ;
	char	*linename = NULL ;
	char	*termdev = NULL ;
	char	*mailfile = NULL ;
	char	*mailpath = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0) {
	    u_close(2) ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

	pip->efp = efp ;
	pip->verboselevel = 1 ;
	pip->debuglevel = 0 ;
	pip->programroot = NULL ;
	pip->helpfile = NULL ;
	pip->pidfname = pidfname ;

	pip->f.quiet = FALSE ;
	pip->f.lockfile = TRUE ;
	pip->f.lockopen = FALSE ;
	pip->f.pidfile = TRUE ;

	f_statdisplay = FALSE ;
	f_help = FALSE ;


/* process program arguments */

	for (i = 0 ; i < NARGPRESENT ; i += 1) argpresent[i] = 0 ;

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
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (cfdeci(argp + 1,argl - 1,&timeoff))
	                    goto badargval ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

	                if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->programroot = argp ;

	                        }

	                        break ;

/* debug level */
	                    case argopt_debug:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            eprintf("main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

					rs = cfdec(avp,avl,
	                                &pip->debuglevel) ; 

					if (rs < 0)
	                                goto badargval ;

				    }
	                        }

	                        break ;

	                    case argopt_version:
	                        f_version = TRUE ;
	                        break ;

	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

/* help file */
	                    case argopt_help:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->helpfile = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

/* log file */
	                    case argopt_log:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					logfname = avp ;

	                        }

	                        break ;

/* display the time this program was last "made" */
	                    case argopt_makedate:
	                        f_makedate = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    eprintf("main: got an option key letter\n") ;
#endif

	                    while (aol--) {

#if	CF_DEBUGS
	                        eprintf("main: option key letters\n") ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdec(avp,avl, &pip->debuglevel) ;

					if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* terminal device */
	                        case 'd':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                termdev = argp ;

	                            break ;

/* set the lock file directory */
	                        case 'l':
	                            pip->f.lockfile = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    lockdir = avp ;

	                            }

	                            break ;

/* the mail path */
	                        case 'm':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                mailpath = argp ;

	                            break ;

/* time offset */
	                        case 'o':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&timeoff) ;

					if (rs < 0)
	                                    goto badargval ;

				    }

	                            break ;

/* observe PID file */
	                        case 'p':
	                            pip->f.pidfile = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    piddir = avp ;

	                            }

	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 's':
	                            f_statdisplay = TRUE ;
	                            break ;

/* set the display refresh interval */
	                        case 'r':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&refresh) ;

					if (rs < 0)
	                                goto badargval ;

				    }

	                            break ;

/* time out for screen blanking */
	                        case 't':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

					rs = cfdeci(argp,argl,&timeout) ;

					if (rs < 0)
	                                goto badargval ;

				    }

	                            break ;

	                        case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
					;;

	                        default:
	                            f_usage = TRUE ;
					ex = EX_USAGE ;
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        aop += 1 ;

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


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%d\n",
	        pip->progname,pip->debuglevel) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    bprintf(efp,"%s: built %s\n",
	        pip->progname,daytimer_makedate) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto exit ;


	if (f_help) {

	    if (pip->helpfile == NULL) {

	        sl = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->programroot,HELPFNAME) ;

	        pip->helpfile = (char *) mallocstrn(buf,sl) ;

	    }

	    helpfile(pip->helpfile,pip->efp) ;

	    goto exit ;

	} /* end if */


/* get our program root (if we have one) */

	if (pip->programroot == NULL)
	    pip->programroot = getenv(PROGRAMROOTVAR1) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(PROGRAMROOTVAR2) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv("HOME") ;

	if (pip->programroot == NULL)
	    pip->programroot = PROGRAMROOT ;


/* processing the invocation arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf(
	                    "main: positional argument i=%d pan=%d arg=%s\n",
	                    i,pan,argv[i]) ;
#endif


	            switch (pan) {

	            case 0:
	                mailfile = argv[i] ;
	                break ;

	            case 1:
	                if (argv[i][0] != '\0') {

	                    if (cfdeci(argv[i],-1,&timeoff) < 0)
	                        goto badargval ;

	                }

	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: out of checking for positional arguments\n") ;
#endif


/* are we an interactively logged in user ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: do we have a terminal ?\n") ;
#endif

	linenamebuf[0] = '\0' ;
	if ((termdev != NULL) && (termdev[0] != '\0')) {

	    if (termdev[0] != '/')
	        bufprintf(linenamebuf,MAXPATHLEN,"/dev/%s",termdev) ;

	    else
	        strcpy(linenamebuf,termdev) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: 1 linenamebuf=%s\n",linenamebuf) ;
#endif

	    tfd = u_open(linenamebuf,O_RDWR,0666) ;

	} else
	    tfd = FD_STDOUT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: TFD=%d\n",tfd) ;
#endif

	if (! isatty(tfd))
	    goto badnotterm ;


/* we use our terminal line name for a lock of some sort */

	if (pip->f.lockfile || pip->f.pidfile) {

	    if (linenamebuf[0] == '\0') {


#if	CF_TERMDEVICE
	        rs = termdevice(linenamebuf,LINENAMELEN,FD_STDOUT) ;
#else
#if	CF_TTYNAMER
	        if ((rs = ttyname_r(FD_STDOUT,linenamebuf,LINENAMELEN)) != 0)
	            rs = (- rs) ;

#if	CF_DEBUG
	        eprintf("main: ttyname_r rs=%d\n",rs) ;
#endif
#else
	        rs = -1 ;
	        if ((cp = ttyname(FD_STDOUT)) == NULL) {

	            rs = 0 ;
	            strwcpy(linenamebuf,cp,LINENAMELEN) ;

	        }
#endif
#endif /* CF_TERMDEVICE */

	        if (rs < 0)
	            goto badline ;

	    } /* end if (getting our line device name from FD) */

/* create the lock file name from our terminal device name */

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        eprintf("main: 2 linename=%s\n",
	            linenamebuf) ;

	    }
#endif /* CF_DEBUG */

/* take off the first 5 characters and change '/' to underscores */

	    linename = linenamebuf + 5 ;
	    for (cp = linename ; *cp ; cp += 1)
	        if (*cp == '/') *cp = '_' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: linename=%s\n",
	            linename) ;
#endif

	} /* end if (terminal device) */


/* who are we ? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->pid = u.pid ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: logged in as \"%s\" on terminal %s\n",
	        pip->progname,u.username,linenamebuf) ;


	pip->sid = u_getsid(0) ;

	u_time(&daytime) ;


/* do we have a log file ? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {

	    len = bufprintf(buf,BUFLEN,"%s/%s",pip->programroot,logfname) ;

	    logfname = mallocstrn(buf,len) ;

	}

/* make a log entry */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: logfile=%s\n",
	        logfname) ;
#endif

	if ((rs = logfile_open(&pip->lh,logfname,0,0666,u.logid)) >= 0) {

		struct utsname	un ;


#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        eprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	        timestr_log(daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

		(void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
		un.sysname,un.release) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    else if (u.mailname != NULL)
	        sprintf(buf,"(%s)",u.mailname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
		u.nodename,u.username,buf) ;

	} else {

	    if (pip->debuglevel > 0) {

	        bprintf(pip->efp,
	            "%s: logfile=%s\n",
	            pip->progname,logfname) ;

	        bprintf(pip->efp,
	            "%s: could not open the log file (rs %d)\n",
	            pip->progname,rs) ;

	    }

	} /* end if (opened a log) */


/* before we go too far, are we supposed to observe a lock file ? */

	lockfname[0] = '\0' ;
	if (pip->f.lockfile) {

	    mode_t	umask_old ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: trying to create a lock file\n") ;
#endif

/* create the LOCK directory if necessary */

	    if (strchr(lockdir,'/') != NULL)
	        sl = bufprintf(lockfname,MAXPATHLEN,"%s",
	            lockdir) ;

	    else
	        sl = bufprintf(lockfname,MAXPATHLEN,"%s/%s",
	            u.homedir,
	            lockdir) ;

	    while ((sl > 0) && (lockfname[sl - 1] == '/')) 
		sl -= 1 ;

	    lockfname[sl] = '\0' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: lockfile, dir=%s\n",
	            lockfname) ;
#endif

	    umask_old = umask(0000) ;

	    if ((rs = mkdirs(lockfname,0777)) < 0) {

	        logfile_printf(&pip->lh,
			"could not create lock file directory\n") ;

	        goto badlockdir ;
	    }

	    (void) umask(umask_old) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: lockfile, trying file\n") ;
#endif

	    bufprintf(lockfname + sl,MAXPATHLEN - sl,"/%s",
	        linename) ;

	    logfile_printf(&pip->lh,"lockfile=%s\n",lockfname) ;

	    rs = lfm_init(&lk,lockfname,LFM_TRECORD,LOCKTIMEOUT,
			u.nodename,u.username,BANNER) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
			"could not capture the lockfile (rs %d)\n",
	            rs) ;

	        goto badlock1 ;
	    }

	    pip->f.lockopen = TRUE ;
	    lfm_printf(&lk,"%-14s %s\n",
	        pip->progname,
	        VERSION) ;

	    lfm_printf(&lk,"d=%s %s!%s pid=%d\n",
	        u.domainname,
	        u.nodename,
	        u.username,
	        u.pid) ;

	    lfm_flush(&lk) ;

	} /* end if (capturing lock file) */


/* before we go too far, are we the only one on this PID mutex ? */

	pidfname[0] = '\0' ;
	if (pip->f.pidfile) {

	    mode_t	umask_old ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: trying to create a PID file\n") ;
#endif

/* create the PID directory if necessary */

	    if (piddir[0] == '/')
	        sl = bufprintf(pidfname,MAXPATHLEN,"%s",
	            piddir) ;

	    else
	        sl = bufprintf(pidfname,MAXPATHLEN,"%s/%s",
	            u.homedir,
	            piddir) ;

	    while ((sl > 0) && (pidfname[sl - 1] == '/')) 
		sl -= 1 ;

	    pidfname[sl] = '\0' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: PID lock, piddir=%s\n",
	            pidfname) ;
#endif

	    umask_old = umask(0000) ;

	    if ((rs = mkdirs(pidfname,0777)) < 0)
	        goto badpiddir ;

	    (void) umask(umask_old) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: made the dir, rs=%d\n",rs) ;
#endif

	    bufprintf(pidfname + sl,MAXPATHLEN - sl,"/%s",
	        linename) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    if ((rs = bopen(&pidfile,pidfname,"rwc",0664)) < 0)
	        goto badpidopen ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: open, rs=%d\n",rs) ;
#endif

/* capture the PID file (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we captured the PID lock, rs=%d\n",
	            rs) ;
#endif

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",u.pid) ;

	    bprintf(&pidfile,"%s!%s\n", u.nodename, u.username) ;

	    bprintf(&pidfile,"%s %s\n",
	        timestrlog(daytime,timebuf),
	        BANNER) ;

	    bprintf(&pidfile,"%-14s %s\n",
	        pip->progname,
	        VERSION) ;

	    bprintf(&pidfile,"d=%s pid=%d\n",
	        u.domainname,
	        u.pid) ;

	    bflush(&pidfile) ;

	    pip->pfp = &pidfile ;

	} /* end if (we have a mutex PID file) */


/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    eprintf("main: calling 'daytimer' f_lock=%d f_pid=%d\n",
	        pip->f.lockfile,pip->f.pidfile) ;

	    eprintf("main: refresh=%d timeout=%d\n",
	        refresh,timeout) ;

	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0)
		bprintf(efp,"%s: timeout=%d\n",
			pip->progname, timeout) ;


	mailfiles_init(&mfs) ;


#if	CF_EXPERIMENT

	if (mailfile != NULL) {

		mailfiles_add(&mfs,mailfile,-1) ;

	} else if ((cp = getenv("MAILPATH")) != NULL) {

		mailfiles_parse(&mfs,cp) ;

	} else if ((cp = getenv("MAIL")) != NULL)
		mailfiles_add(&mfs,cp,-1) ;

#endif /* CF_EXPERIMENT */

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

		MAILFILES_ENT	*ep ;


		rs = mailfiles_count(&mfs) ;

		eprintf("main: mailfiles count=%d\n",rs) ;

		for (i = 0 ; mailfiles_get(&mfs,i,&ep) >= 0 ; i += 1) {

			if (ep == NULL) continue ;

			eprintf("main: mf=%s\n",ep->mailfile) ;

		}

	}
#endif /* CF_DEBUG */

/* do it */

	rs = daytimer(pip,&u,&lk,timeoff,timeout,refresh,f_statdisplay,&mfs) ;

	mailfiles_free(&mfs) ;


#ifdef	COMMENT
	logfile_printf(&pip->lh,"exiting (rs %d)\n",rs) ;
#endif



/* close off and get out ! */
exit:
done:
ret3:
	if (pip->f.pidfile && (pidfname != NULL) && (pidfname[0] != '\0'))
	    u_unlink(pidfname) ;

ret2:
	if (pip->f.lockopen)
	    lfm_free(&lk) ;

ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [mailfile [offset]] [-sV] [-t timeout] [-offset]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,
		"\t[-d terminal_device]\n",
	    pip->progname,pip->progname) ;

	goto exit ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto badret ;

badnotterm:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: user is not interactively logged in\n",
	        pip->progname) ;

	ex = EX_DATAERR ;
	goto badret ;

baduser:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badline:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: could not get login terminal line, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badlockdir:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: could not create the necessary lock directory, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badlock1:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: could not create or capture the lock file, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpiddir:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not create the PID directory, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpidopen:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: could not create the PID file, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpidlock:
	if (pip->debuglevel > 0)
	    bprintf(efp,
	        "%s: could not lock the PID file, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badret:
	goto done ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



VOID helpfile(f,ofp)
char	f[] ;
bfile	*ofp ;
{
	bfile	file, *ifp = &file ;

	char	buf[BUFLEN + 1] ;


	if ((f == NULL) || (f[0] == '\0')) return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyfile(ifp,ofp,buf,BUFLEN) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



