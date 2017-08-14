/* cmail */

/* program to deliver company email */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0
#define	CF_REXECTEST	0


/* revision history:

	- 1998-12-01, David A.D. Morano
	This program completely (I think) can replace any existing
	'rslow' programs.  Some elements of some previous 'rslow' 
	programs may have been used but it is all mixed in now.

	- 1992-12-21, David A.D. Morano
	Added code to create directories in the UUCPPUBLIC
	area so that any soft links pointing into here (for user 'pcs')
	will point to something that exists !


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:
	$ cmail [-f from_address]
		[-i input] address [address ...] < input


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<pwd.h>
#include	<grp.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<time.h>
#include	<errno.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define		NPARG		200


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;


/* define command option words */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	NULL,
} ;

#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ROOT	3
#define	ARGOPT_LOGFILE	4


/* external subroutines */

extern char	*putheap() ;
extern char	*strshrink() ;
extern char	*strbasename() ;
extern char	*ns_mailname() ;


/* forward subroutines */

static int	make_intfile() ;
static int	delete_jobfile() ;
static int	avail_uucp() ;
static int	avail_rcp() ;
static int	avail_rexec() ;
static int	checkuucppublic() ;

static void	dump() ;


/* external variables */


/* local variables */

struct global	g ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		jobfile, *jfp = &jobfile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	bfile		*fpa[3], file0, file1, file2  ;

	struct ustat	sb ;

	struct tm	*timep ;

	struct passwd	*pp ;

	struct group	*gp ;

	offset_t	clen ;

	pid_t	cpid ;

	int	argr, argl, aol, akl, avl ;
	int	kwi, npa, ai, i ;
	int	rs, len, l ;
	int	ifd = 0 ;
	int	argnum = 0 ;
	int	jlen ;
	int	transport = 0 ;
	int	waittime = WAITTIME ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_exitargs = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_verbose = FALSE ;
	int	f_output = FALSE ;
	int	f_noinput = FALSE ;
	int	f_localfile = FALSE ;
	int	f_clen = FALSE ;
	int	f_kick = FALSE ;
	int	f_jobfilepath = FALSE ;
	int	f_uucp = FALSE ;
	int	f_mail = FALSE ;
	int	f_queueonly = FALSE ;
	int	f_interrupt = FALSE ;
	int	jfd, rfd, efd ;
	int	childstat ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*progname ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*jobfname = NULL ;
	char	nodename[1024], domainname[1024] ;
	const char	*jobid ;
	const char	*cmd_uucico ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	const char	*address_errors = NULL ;
	const char	*address_sender = NULL ;
	const char	*address_from = NULL ;
	const char	*address_error = NULL ;
	const char	*address_reply = NULL ;
	const char	*name_from = NULL ;
	const char	*name_to = NULL ;
	const char	*user = NULL ;
	const char	*kickhost = NULL ;
	const char	*tmpdir = NULL ;
	const char	*queuespec = NULL ;
	const char	*servicename = NULL ;
	const char	*queue_machine, *queue_path ;
	const char	*local_path = DEFQUEUEPATH ;
	const char	*copy_path = NULL ;
	const char	*cp, *cp1, *cp2 ;
	char	cmdbuf[(2 * MAXPATHLEN) + 1] ;
	char	ahostname[2048 + 1] ;
	char	*ahost = ahostname ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0) return BAD ;

	bcontrol(efp,BC_LINEBUF,0) ;

/* standard output */

	if (bopen(ofp,BFILE_STDOUT,"wct",0666) < 0) return BAD ;

/* miscellaneous early stuff */

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	g.progname = progname ;
	g.logfname = NULL ;
	g.efp = efp ;
	g.debuglevel = 0 ;
	g.f.log = FALSE ;

/* parse arguments */

	npa = 0 ;			/* number of positional so far */
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optplus || f_optminus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdec(argp + 1,argl - 1,&argnum) < 0)
	                    goto badarg ;

	                if (argnum < 0) argnum = 0 ;

	            } else {

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

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

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
	                        f_verbose = TRUE ;
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

/* LOGFILE */
	                    case ARGOPT_LOGFILE:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.logfname = avp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        bprintf(efp,
	                            "%s: unknown argument keyword \"%s\"\n",
	                            progname,akp) ;

	                        f_exitargs = TRUE ;
	                        f_usage = TRUE ;

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
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&g.debuglevel) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* directory to create job file in */
	                        case 'd':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    tmpdir = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    tmpdir = argp ;

	                            }

	                            break ;

/* input file name */
	                        case 'i':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    ifname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    ifname = argp ;

	                            }

	                            break ;

/* mail address of submitter for errors ("error-to" address) */
	                        case 'e':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    address_error = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    address_error = argp ;

	                            }

	                            break ;

/* mail address of submitter for errors ("FROM" address) */
	                        case 'f':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    address_from = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    address_from = argp ;

	                            }

	                            break ;

/* send mail back on job completion (sent as "read confirmation" option) */
	                        case 'm':
	                            f_mail = TRUE ;
	                            break ;

/* mail address of person to receive execution confirmation reply */
	                        case 'r':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    address_reply = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    address_reply = argp ;

	                            }

	                            break ;

/* wait time between 'mkjobfile' attempts */
	                        case 't':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[i++] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if ((argl > 0) &&
	                                (cfdec(argp,argl,&waittime) < 0))
	                                goto badarg ;

	                            break ;

/* specify user */
	                        case 'u':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    user = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    user = argp ;

	                            }

	                            break ;

/* machine to kick for client/server UUCP machine cluster configurations */
	                        case 'k':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    kickhost = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    kickhost = argp ;

	                            }

	                            break ;

/* write the assembled job to a user specified file */
	                        case 'o':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    ofname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    ofname = argp ;

	                            }

	                            f_output = TRUE ;
	                            break ;

/* no input mode (command only) */
	                        case 'n':
	                            f_noinput = TRUE ;
	                            break ;

/* check job queue address specification to confirm a possible local path */
	                        case 'l':
	                            f_localfile = TRUE ;
	                            break ;

/* print JOB FILE PATH out to standard output */
	                        case 'p':
	                            f_jobfilepath = TRUE ;
	                            break ;

	                        case 'v':
	                            f_verbose = TRUE ;
	                            break ;

/* force UUCP over RCP for transport if available */
	                        case 'U':
	                            f_uucp = TRUE ;
	                            break ;

/* send data to the interrupt pipe ? */
	                        case 'I':
	                            f_interrupt = TRUE ;
	                            break ;

/* only queue up if using UUCP */
	                        case 'R':
	                            f_queueonly = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            npa += 1 ;

	        } /* end if */

	    } else {

	        switch (npa) {

	        case 0:
	            if (argl > 0)
	                queuespec = argp ;

	            break ;

	        case 1:
	            servicename = argp ;
	            ai = i + 1 ;
	            f_exitargs = TRUE ;
	            break ;

	        default:
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                progname) ;

	        } /* end switch */

	        npa += 1 ;

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        progname) ;

	if (f_usage) goto usage ;

	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debug level %d\n",
	        progname,g.debuglevel) ;

	rs = OK ;
	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        progname,VERSION) ;

	    goto earlyret ;
	}



#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: f_exitargs %d - ai %d\n",f_exitargs,ai) ;
#endif

/* get a TMPDIR */

	if (tmpdir == NULL) {

	    if ((tmpdir = getenv("TMPDIR")) == NULL)
	        tmpdir = TMPDIR ;

	}

/* get our program root directory */

	if ((g.programroot = getenv("PROGRAMROOT")) == NULL)
	    g.programroot = PROGRAMROOT ;


/* perform miscellaneous functions */

	userinfo(&u,NULL) ;


/* get "mailhost" name */

	g.mailhost = MAILHOST ;


/* get the current time-of-day */

	time(&g.daytime) ;

	timep = localtime(&g.daytime) ;


/* set some flags */

#ifdef	SYSV
	g.f.sysvct = TRUE ;
#else
	g.f.sysvct = TRUE ;
#endif

	g.f.sysvrt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) g.f.sysvrt = TRUE ;


/* create a log ID */

	sprintf(buf,"%s%d",g.nodename,g.pid) ;

	g.logid = putheap(buf) ;

/* find the log file */

	if (g.logfname == NULL) {

	    sprintf(buf,"%s/%s",g.programroot,LOGFILE) ;

	    g.logfname = putheap(buf) ;

	}

/* open the log file (if we can) */

	if ((g.logfname != NULL) &&
		(logfile_open(&g.lh,g.logfname,0,0666,g.logid) >= 0)) {

	    g.f.log = TRUE ;
	    buf[0] = '\0' ;
	    if (g.mailname != NULL) sprintf(buf,"(%s)",g.mailname) ;

	    logfile_printf(&g.lh,"%02d%02d%02d %02d%02d:%02d %-14s %s/%s\n",
	        timep->tm_year,
	        timep->tm_mon + 1,
	        timep->tm_mday,
	        timep->tm_hour,
	        timep->tm_min,
	        timep->tm_sec,
	        g.progname,VERSION,
	        (g.f.sysvct) ? "SYSV" : "BSD") ;

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        g.nodename,g.username,buf) ;

	    logfile_printf(&g.lh,"os=%s domain=%s\n",
	        (g.f.sysvrt) ? "SYSV" : "BSD",
	        g.domainname) ;

	} /* end if (opened log file) */


/* check arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to check arguments\n",
	        progname) ;
#endif

	if (waittime < 1) waittime = 1 ;


/* check out the various addresses that we may be using */

	if (address_reply != NULL) {

	    address_reply = strshrink(address_reply) ;

	    if (((cp = strpbrk(address_reply,"!\\@%/=")) == NULL) &&
	        ((pp = getpwnam(address_reply)) != NULL)) {

	        name_to = ns_mailname(pp->pw_gecos) ;

	        if ((l = strlen(address_reply)) > (BUFLEN - SAVESIZE))
	            address_reply[BUFLEN - SAVESIZE] = '\0' ;

	        sprintf(buf,"%s <%s!%s>",name_to,g.nodename,
	            address_reply) ;

	        address_reply = putheap(buf) ;

	    }

	if (g.f.log)
	    logfile_printf(&g.lh,"reply=%s\n",address_reply) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    if (address_reply)
	        debugprintf("main: address_reply=%s\n",address_reply) ;

	}
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    debugprintf("main: done with address to\n") ;

	}
#endif

/* find a suitable "address_from" */

	if (address_from != NULL) {

	    address_from = strshrink(address_from) ;

	    if (((cp = strpbrk(address_from,"!\\@%/=")) == NULL) &&
	        ((pp = getpwnam(address_from)) != NULL)) {

	        name_from = ns_mailname(pp->pw_gecos) ;

	        if ((l = strlen(address_from)) > (BUFLEN - SAVESIZE))
	            address_from[BUFLEN - SAVESIZE] = '\0' ;

	        sprintf(buf,"%s <%s!%s>",name_from,g.nodename,
	            address_from) ;

	        address_from = putheap(buf) ;

	    }

	} else {

	    sprintf(buf,"%s <%s!%s>",
	        g.mailname,g.mailhost,g.username) ;

	    address_from = putheap(buf) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    if (address_from)
	        debugprintf("main: %s\n",address_from) ;

	}
#endif

/* 
	NOTE on the supposed "username" in the 'user' variable :

	If we do NOT validate that the user really exists on this machine,
	then we should not try to create a mail address with her real
	name in it !
*/

	if (user != NULL) {

	    sprintf(buf,"%s!%s",g.nodename,user) ;

	    address_sender = address_from ;
	    address_from = putheap(buf) ;

	} else
	    user = g.username ;

/* make some log entries */

	logfile_printf(&g.lh,"host=%s.%s user=%s(%d)\n",
	    (u.nodename != NULL) ? u.nodename : "*unknown*",
	    (u.domainname != NULL) ? u.domainname : "*unknown*",
	    (user != NULL) ? user : "*unknown*",u.uid) ;

/* continue address preparation */

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    debugprintf("main: user > %s\n",(user == NULL) ? "" : user) ;

	}
#endif

	if (address_from == NULL) {

	    sprintf(buf,"%s!%s",u.nodename,user) ;

	    address_from = putheap(buf) ;

	}

	if (g.debuglevel > 0) {

	    bprintf(efp,"%s: done with address from \"%s\"\n",
	        progname,address_from) ;

	}

	if (address_from != NULL)
	    logfile_printf(&g.lh,"from=\"%s\"\n",address_from) ;

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    debugprintf("main: about to create a job file\n") ;

	    debugprintf("main: w/ tmpdir > %s\n",tmpdir) ;

	}
#endif

/* create a job ID file in TMPDIR */

	for (i = 0 ; i < 10 ; i += 1) {

	    buf[0] = '\0' ;
	    if ((rs = mkjobfile(tmpdir,0600,buf)) >= 0) break ;

	    logfile_printf(&g.lh,
	        "couldn't make jobfile \"%s\" on pass %d (rs %d)\n",
	        buf,i,rs) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) debugprintf(
	        "main: couldn't make jobfile \"%s\" (rs %d)\n",
	        buf,rs) ;
#endif

	    sleep(waittime) ;

	} /* end if (creating job file) */

	if (rs < 0) goto badjobfile ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 1\n") ;
#endif

	jobfname = putheap(buf) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 2\n") ;
#endif

	jobid = putheap(strbasename(buf)) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: created job file > %s\n",jobfname) ;
#endif

	if (g.debuglevel > 0) {

	    bprintf(efp,"%s: jobid=%s\n",progname,jobid) ;

	}

	logfile_printf(&g.lh,"jobid=%s\n",jobid) ;

/* open files (if necessary) */

	if (! f_noinput) {

	    if (ifname != NULL) {

	        close(0) ;

	        if ((ifd = open(ifname,O_RDONLY,0666)) < 0)
	            goto badinfile ;

	    } else 
	        ifd = 0 ;

	    if ((rs = fstat(ifd,&sb)) < 0) goto badstat ;

	    if (S_ISREG(sb.st_mode)) f_clen = TRUE ;

	} else {

	    clen = 0 ;
	    f_clen = TRUE ;

	}






/* write out the job file header */

	bprintf(jfp,"x-mailer: AT&T RSLOW (version %s)\n",VERSION) ;

	if (address_errors != NULL)
	    bprintf(jfp,"errors-to: %s\n",address_errors) ;

	bprintf(jfp,"content-type: application/octet\n") ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 4\n") ;
#endif

	cp = getenv("FULLNAME") ;

	if (cp == NULL) cp = getenv("NAME") ;

	if (cp == NULL) cp = g.gecosname ;

	if (cp != NULL)
	    bprintf(jfp,"full-name: %s\n",cp) ;

	if (f_clen)
	    bprintf(jfp,"content-length: %ld\n",sb.st_size) ;

	bprintf(jfp,"x-orighost: %s%s%s\n",
	    g.nodename,(g.domainname != NULL) ? "." : ".att.com",
	    (g.domainname != NULL) ? g.domainname : "") ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 5\n") ;
#endif

	bprintf(jfp,"x-username: %s\n",user) ;

	if (address_error != NULL)
	    bprintf(jfp,"error-to: %s\n",address_error) ;

	if (address_sender != NULL)
	    bprintf(jfp,"sender: %s\n",address_sender) ;

	bprintf(jfp,"from: %s\n",address_from) ;

/* put out the destination queue specification */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 6\n") ;
#endif

	if (queuespec[0] == '!')
	    bprintf(jfp,"x-queuespec: %s%s\n",
	        g.nodename,queuespec) ;

	else
	    bprintf(jfp,"x-queuespec: %s\n",queuespec) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 6a\n") ;
#endif

/* put out the serivce name and any arguments */

	bprintf(jfp,"x-service: %s",servicename) ;

	logfile_printf(&g.lh, "srv=%s\n",servicename) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 6b\n") ;
#endif

	l = 0 ;
	for (i = ai ; i < argc ; i += 1) {

	    bprintf(jfp," %s",argv[i]) ;

	    l += sprintf(buf + l," %s",argv[i]) ;

	}

	if (l > 0)
	    logfile_printf(&g.lh,"args=\"%W\"\n",buf,l) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 6c\n") ;
#endif

	bprintf(jfp,"\n") ;

/* end of service name header */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf("main: 7\n") ;
#endif

	bprintf(jfp,"x-jobid: %s\n",jobid) ;

	if (f_mail)
	    bprintf(jfp,"x-options: /rc\n") ;

	if (address_reply != NULL)
	    bprintf(jfp,"reply-to: %s\n",address_reply) ;

	if (queue_machine == NULL) queue_machine = "" ;

	bprintf(jfp,"to: %s!rslow\n",
	    (queue_machine[0] != '\0') ? queue_machine : g.nodename) ;

/* print out the END-OF-HEADER indicator */

	bputc(jfp,'\n') ;

/* if we have an input file, go through the loops */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf("main: 8\n") ;
#endif

	if (! f_noinput) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: go through the loops\n") ;
#endif

	    jlen = 0 ;
	    while ((len = read(ifd,buf,BUFLEN)) > 0) {

	        i = 0 ;
	        l = len ;
	        while ((rs = bwrite(jfp,buf + i,l)) < l) {

	            if (rs < 0) goto badwrite ;

	            i += rs ;
	            l -= rs ;

	        }

	        jlen += len ;

	    } /* end while */

	    logfile_printf(&g.lh,"joblen=%d\n",jlen) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: about to close input file\n") ;
#endif

	    close(ifd) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to close job file\n") ;
#endif

	bclose(jfp) ;

/* send it along using the transport provider */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to use transport if real job\n") ;
#endif




	if (jobfname != NULL) {

	    if (f_jobfilepath)
	        bprintf(ofp,"%s\n",jobfname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: jobfname=%s\n",jobfname) ;
#endif

	    delete_jobfile(jobfname) ;

	}

	if ((tmpfname != NULL) && (tmpfname[0] != '\0')) {

	    unlink(tmpfname) ;

	    tmpfname[0] = '\0' ;
	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: finishing up\n") ;
#endif

/* finish up */
done:
	if (g.debuglevel > 0)
	    bprintf(efp,"%s: job \"%s\" sent\n",progname,jobid) ;

	logfile_printf(&g.lh,"job \"%s\" sent using %s\n",
	    jobid,trans_name[transport]) ;

	if (f_verbose)
	    bprintf(efp,"%s: jobid %s\n",progname,jobid) ;

	bclose(ofp) ;

	logfile_close(&g.lh) ;

/* take the early return here */
earlyret:
	bclose(efp) ;

	return transport ;

/* bad returns come here */
badret:

#if	CF_DEBUG
	bflush(efp) ;
#endif

	if (jobfname != NULL)
	    delete_jobfile(jobfname) ;

	if ((tmpfname != NULL) && (*tmpfname != '\0'))
	    unlink(tmpfname) ;

	bclose(ofp) ;

	logfile_close(&g.lh) ;

	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> rslow [-?VD] mach!path | - ",
	    progname) ;

	bprintf(efp,"[-UR] [-u user] [-d directory]") ;

	bprintf(efp,"\n\t[-mp] [-f from] [-r reply] ") ;

	bprintf(efp,"service [service_args ...]\n") ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (errno %d)\n",
	    progname,errno) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: cannot open the output file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badwrite:
	logfile_printf(&g.lh,"failed on write to temporary file (rs %d)\n",
	    rs) ;

	bprintf(efp,"%s: failed on write to temporary file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badjobread:
	logfile_printf(&g.lh,"failed on read from job file (rs %d)\n",
	    rs) ;

	bprintf(efp,"%s: failed on read from temporary file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badstat:
	bprintf(efp,"%s: could not get status on input file (errno %d)\n",
	    progname,errno) ;

	goto badret ;

badjobfile:
	logfile_printf(&g.lh,"could not make a job file (rs %d)\n",
	    rs) ;

	bprintf(efp,"%s: could not make a job file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badjobopen:
	logfile_printf(&g.lh,"could not open the job file (rs %d)\n",
	    rs) ;

	bprintf(efp,"%s: could not open the job file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badtransport:
	logfile_printf(&g.lh,"no suitable transport provider was found\n") ;

	bprintf(efp,"%s: no suitable transport provider was found\n",
	    progname) ;

	goto badret ;

badservice:
	bprintf(efp,"%s: bad or no service specification was given\n",
	    progname) ;

	goto badret ;

badqueuespec:
	bprintf(efp,"%s: bad queue specification was given\n",
	    progname) ;

	goto badret ;
}
/* end subroutine (main) */


/* make a temporary file with a single character in it for RSLOW interrupt */
static int make_intfile(tmpfname)
char	tmpfname[] ;
{
	bfile	tmpfile, *tfp = &tmpfile ;

	int	rs ;


	rs = mktmpfile( tmpfname, 0600, "/tmp/rslowXXXXXXXXX") ;
	if (rs < 0)
	    return rs ;

	if ((rs = bopen(tfp,tmpfname,"wct",0666)) < 0) goto badret ;

	if ((rs = bputc(tfp,'\n')) < 0) goto badret ;

	bclose(tfp) ;

	return OK ;

badret:
	bclose(tfp) ;

	unlink(tmpfname) ;

	return rs ;
}
/* end subroutine (make_intfile) */


static int delete_jobfile(jobfname)
char	jobfname[] ;
{
	int	rs, i, err ;


	if ((jobfname == NULL) || (*jobfname == '\0')) return OK ;

	if ((rs = fork()) == 0) {

	    setsid() ;

	    for (i = 0 ; i < 3 ; i += 1) close(i) ;

	    sleep(30) ;

	    unlink(jobfname) ;

	    exit(0) ;

	}

	if (rs < 0) {

	    err = errno ;
	    bprintf(g.efp,
	        "%s: non-fatal error - could not fork (errno %d)\n",
	        g.progname,err) ;

	    logfile_printf(&g.lh,
	        "delete_jobfile: non-fatal error - could not fork (errno %d)\n",
	        err) ;

	    sleep(10) ;		/* less clash protection than usual */

	    rs = unlink(jobfname) ;

	}

	return (rs == -1) ? -err : rs ;
}
/* end subroutine (delete_jobfile) */


/* check for UUCP availability */
static int avail_uucp(queue_machine)
char	queue_machine[] ;
{
	bfile		procfile, *pfp = &procfile ;
	bfile		*fpa[3] ;

	pid_t		pid ;

	int	child_stat ;
	int	rs, i, l ;

	char	buf[MAXPATHLEN + 1] ;
	char	*cp ;


#if	CF_DEBUGS
	debugprintf("avail_uucp: entered\n") ;
#endif

	if ((queue_machine == NULL) || (queue_machine[0] == '\0'))
	    return BAD ;

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

#if	CF_DEBUGS
	debugprintf("avail_uucp: got in\n") ;
#endif

	sprintf(buf, "uuname 2> /dev/null <&2") ;

	fpa[1] = pfp ;		/* capture the standard output ! */

#if	CF_DEBUGS
	debugprintf("avail_uucp: about to open command - FPA[1]=%08X\n",fpa[1]) ;
#endif

	if ((rs = bopencmd(fpa,buf)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("avail_uucp: opened command OK\n") ;
#endif

	    pid = rs ;
	    rs = BAD ;
	    while ((l = breadline(pfp,buf,BUFLEN)) > 0) {

#if	CF_DEBUGS
	        debugprintf("avail_uucp: got a line\n") ;
#endif

	        buf[l] = '\0' ;
	        cp = strshrink(buf) ;

	        if (strcasecmp(cp,queue_machine) == 0) {

	            rs = OK ;
	            break ;
	        }
	    }

	    bclose(pfp) ;

	    waitpid(pid,&child_stat,WUNTRACED) ;

	} /* end if (program spawned) */

#if	CF_DEBUGS
	debugprintf("avail_uucp: past the command OK\n") ;
#endif

	return rs ;
}
/* end subroutine (avail_uucp) */


/* check for RCP availability */
static int avail_rcp(queue_machine,queue_path,jobid)
char	queue_machine[] ;
char	queue_path[] ;
char	jobid[] ;
{
	bfile		procfile, *pfp = &procfile ;
	bfile		*fpa[3] ;

	pid_t		pid ;

	int	child_stat ;
	int	rs, i, l ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[MAXPATHLEN + 1] ;
	char	*cp ;


	if ((queue_machine == NULL) || (*queue_machine == '\0'))
	    return BAD ;

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

/* try to ping the machine */

/* check if we can actually create a file over there */

	sprintf(tmpfname,"%s/q/%s",
	    queue_path,jobid) ;

	sprintf(buf,
	    "rsh -n %s 'cp /dev/null %s ; ls %s' 2> /dev/null",
	    queue_machine,
	    tmpfname,
	    tmpfname) ;

	fpa[1] = pfp ;
	if ((rs = bopencmd(fpa,buf)) >= 0) {

	    pid = rs ;
	    rs = BAD ;
	    if ((l = breadline(pfp,buf,BUFLEN)) > 0) {

	        buf[l] = '\0' ;
	        cp = strshrink(buf) ;

	        if (strcasecmp(cp,tmpfname) == 0)
	            rs = OK ;

	    }

	    bclose(pfp) ;

	    waitpid(pid,&child_stat,WUNTRACED) ;

	} /* end if (program spawned) */

	return rs ;
}
/* end subroutine (avail_rcp) */


/* check for REXEC availability */
static int avail_rexec(queue_machine,queue_path,jobid)
char	queue_machine[] ;
char	queue_path[] ;
char	jobid[] ;
{
	int	rs, i, len, l ;
	int	rfd, efd ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	cmdbuf[(2 * MAXPATHLEN) + 1] ;
	char	hostname[2048 + 1] ;
	char	*cp, *ahost = hostname ;


	if ((queue_machine == NULL) || (*queue_machine == '\0'))
	    return BAD ;

	strcpy(hostname,queue_machine) ;

/* try to ping the machine */

/* check if we can actually create a file over there */

	sprintf(tmpfname,"%s/q/%s",
	    queue_path,jobid) ;

	sprintf(cmdbuf,"%s %s",
	    "/usr/add-on/pcs/etc/bin/mktestfile",
	    tmpfname) ;

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "avail_rexec: about to call 'rexec'\n") ;
#endif

	if ((rs = rexec(&ahost,(unsigned short) 512,"pcs","pcs31",
	    cmdbuf,&efd)) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0) debugprintf(
	        "avail_rexec: we are in\n") ;
#endif

	    rfd = rs ;
	    rs = BAD ;
	    i = 0 ;
	    while (((BUFLEN - i) > 0) && 
	        ((rs = read(rfd,buf + i,BUFLEN - i)) > 0))
	        i += rs ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) debugprintf(
	        "avail_rexec: goto some output i=%d\n",i) ;
#endif

	    if (i > 0) {

	        buf[i] = '\0' ;
	        cp = strshrink(buf) ;

	        if (strcasecmp(cp,tmpfname) == 0)
	            rs = OK ;

	    } else {

	        rs = BAD ;
	        if ((l = read(efd,buf,BUFLEN)) > 0) {

	            if (buf[l - 1] == '\n') buf[--l] = '\0' ;

	            logfile_printf(&g.lh,"rexec error to host=%s\n",
	                ahost) ;

	            logfile_printf(&g.lh,"rexec error \"%W\"\n",
	                buf,MIN(l,60)) ;

	        }

	    }

	    close(efd) ;

	    close(rfd) ;

	} /* end if (opened connection) */

	return rs ;
}
/* end subroutine (avail_rexec) */


/* dump data from pipes */

/* 
	This is a cheap way to try and read the junk coming out of two
	pipes without getting into a situation where the sending program
	blocks.
*/

#define	DUMPLEN	50

static void dump(f1p,f2p)
bfile	*f1p, *f2p ;
{
	int	f_done1, f_done2 ;

	char	buf[DUMPLEN + 1] ;


	f_done1 = f_done2 = FALSE ;
	while ((! f_done1) || (! f_done2)) {

	    if (! f_done1) {

	        if (bread(&f1p,buf,DUMPLEN) <= 0)
	            f_done1 = TRUE ;

	    }

	    if (! f_done2) {

	        if (bread(&f2p,buf,DUMPLEN) <= 0)
	            f_done2 = TRUE ;

	    }

	} /* end while */

}
/* end subroutine (dump) */


/* check if the UUCPPUBLIC spool area has the proper directories */
static int checkuucppublic(gp)
struct global	*gp ;
{
	bfile		file0, file1, file2, *fpa[3] ;

	struct passwd	*pp ;

	pid_t		cpid ;

	int		rs, srs ;
	int		len ;
	int		childstat ;

	char		linebuf[LINELEN + 1] ;
	char		*dirp ;


/* if we are not a real UUCP site (as evidenced by no UUCP hosts), punt out */

	fpa[0] = &file0 ;
	fpa[1] = &file1 ;
	fpa[2] = &file2 ;
	if ((cpid = bopencmd(fpa,"uuname")) < 0) return BAD ;

	bclose(fpa[0]) ;

	len = breadline(fpa[1],linebuf,LINELEN) ;

	dump(&file1,&file2) ;

	bclose(fpa[1]) ;

	bclose(fpa[2]) ;

	waitpid(cpid,&childstat,0) ;

	if (len <= 0) return BAD ;

/* OK, we go for it */

	srs = OK ;
	dirp = "/var/spool/uucppublic/receive" ;
	if (access(dirp,W_OK) < 0) {

	    if (mkdir(dirp,0777) >= 0) {

	        chmod(dirp,0777) ;

	        if (gp->debuglevel > 1) bprintf(gp->efp,
	            "%s: made the UUCP spool 'receive' directory\n",
	            gp->progname) ;

	        logfile_printf(&gp->lh,
	            "made the UUCP spool 'receive' directory\n") ;

	        if ((pp = getpwnam("uucp")) != NULL) {

	            chown("/var/spool/uucppublic/receive",
	                pp->pw_uid,-1) ;

	        }

	    }

	}

	dirp = "/var/spool/uucppublic/receive/pcs" ;
	if (access(dirp,W_OK) < 0) {

	    rs = mkdir(dirp,0777) ;

	    if (rs < 0) {

	        rs = -errno ;
	        if (srs >= 0) srs = rs ;

	    }

		if (rs >= 0)
	        	chmod(dirp,0777) ;

	    if (gp->debuglevel > 1) bprintf(gp->efp,
	        "%s: tried to make 'UUCPPUBLIC/receive/pcs' (rs %d)\n",
	        gp->progname,rs) ;

	    logfile_printf(&gp->lh,
	        "tried to make 'UUCPPUBLIC/receive/pcs' (rs %d)\n",
	        rs) ;

	}

	dirp = "/var/spool/uucppublic/receive/pcs/rslow" ;
	if (access(dirp,
	    W_OK) < 0) {

	    rs = mkdir(dirp,
	        0777) ;

	    if (rs < 0) {

	        rs = -errno ;
	        if (srs >= 0) srs = rs ;

	    }

		if (rs >= 0)
	        	chmod(dirp,0777) ;

	    if (gp->debuglevel > 1) bprintf(gp->efp,
	        "%s: tried to make 'UUCPPUBLIC/receive/pcs/rslow (rs %d)'\n",
	        gp->progname,rs) ;

	    logfile_printf(&gp->lh,
	        "tried to make 'UUCPPUBLIC/receive/pcs/rslow' (rs %d)\n",
	        rs) ;

	}

	return srs ;

baduucp:
	bclose(fpa[1]) ;

	bclose(fpa[2]) ;

	waitpid(cpid,&childstat,0) ;

	return BAD ;
}
/* end subroutine (checkuucppublic) */



