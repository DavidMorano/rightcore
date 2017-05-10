/* main (rslow) */

/* program to submit a job for the RSLOW daemon */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0
#define	CF_RFILE	1
#define	CF_UUCP		1


/* revision history:

	= 94-12-01, David A­D­ Morano

	This program completely (I think) can replace any existing
	'rslow' programs.  Some elements of some previous 'rslow' 
	programs may have been used but it is all mixed in now.


	- 96-06-06, David A­D­ Morano

	Added code to create directories in the UUCPPUBLIC
	area so that any soft links pointing into here (for user 'pcs')
	will point to something that exists !


	- 97-01-29, David A­D­ Morano

	I added code to the command invocation for an optional
	authorization file for use when 'rexec' is the transport
	to PCS.  A default in PROGRAMROOT is used otherwise.

	I also added some crap code to check for certain machine
	aliases for those local machines that do not have uniform
	INET and UUCP access.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ rslow [-f from_address] [-u user] [-ln] 
		[-i input] mach:path service arguments < input


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<vecelem.h>
#include	<netfile.h>
#include	<getax.h>
#include	<mallocstuff.h>
#include	<rex.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dialtab.h"


/* local defines */

#define	NPARG		2
#define	USERBUFLEN	(NODENAMELEN + (2 * 1024))
#define	CMDBUFLEN	(MAXPATHLEN * 2)
#define	O_WFLAGS	(O_WRONLY | O_CREAT | O_TRUNC)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	unlinkd(char *,int) ;
extern int	quoteshellarg() ;
extern int	hostequiv() ;
extern int	dialtab_start(), dialtab_search(), dialtab_finish() ;

extern char	*strshrink(char *) ;
extern char	*strbasename(char *) ;
extern char	*ns_mailname(char *) ;

void		bdump() ;


/* external variables */


/* local structures */


/* forward references */

static int	make_intfile() ;
static int	delete_jobfile() ;
static int	checkuucppublic() ;


/* local variables */

struct global	g ;

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


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		jobfile, *jfp = &jobfile ;
	bfile		*fpa[3], file0, file1, file2  ;

	vecelem		ne ;

	struct ustat	sb ;

	struct tm	*timep ;

	struct passwd	pe, *pp ;

	struct userinfo	u ;

	struct rex_auth	auth ;

	struct netrc	*mp ;

	offset_t	clen ;

	pid_t	cpid ;

	int	argr, argl, aol, akl, avl ;
	int	kwi, npa, ai, i, j ;
	int	rs, len, l, uulen ;
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
	int	f_euid ;
	int	f_initlist = FALSE ;
	int	f_authforce ;
	int	jfd, tfd, rfd, efd ;
	int	childstat ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*progname ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*jobfname = NULL ;
	const char	*authfname = NULL ;
	const char	*jobid ;
	const char	*cmd_uucico ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERBUFLEN + 1] ;
	char	u_buf[100], p_buf[100] ;
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
	const char	*queue_uumachine, *queue_inmachine ;
	const char	*local_path = NULL ;
	const char	*copy_path = NULL ;
	const char	*cp, *cp1 ;
	char	cmdbuf[CMDBUFLEN + 1] ;
	const char	*ssp ;


#if	CF_DEBUGS
	debugprintf("main: program entered\n") ;

#ifdef	COMMENT
	nprintf("/home/dam/src/rbbpost/rslow.here",
		"main: here we are\n") ;
#endif
#endif

	progname = strbasename(argv[0]) ;

	g.f.errout = TRUE ;
	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0) g.f.errout = FALSE ;

	if (g.f.errout)
	    bcontrol(efp,BC_LINEBUF,0) ;

/* miscellaneous early stuff */

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	g.progname = progname ;
	g.logfname = NULL ;
	g.netfname = NULL ;
	g.efp = efp ;
	g.debuglevel = 0 ;
	g.authusername = NULL ;
	g.authpassword = NULL ;

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
	                        if (g.f.errout)
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

/* alternate user's NETRC file */
	                        case 'N':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    g.netfname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    g.netfname = argp ;

	                            }

	                            break ;

/* authorization file */
	                        case 'a':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    authfname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    authfname = argp ;

	                            }

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
	                            if (g.f.errout)
	                                bprintf(efp,"%s: unknown option - %c\n",
	                                    progname,*aop) ;

/* fall through to the next case statement */

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
	            if (g.f.errout)
	                bprintf(efp,
	                    "%s: extra positional arguments ignored\n",
	                    progname) ;

	        } /* end switch */

	        npa += 1 ;

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if ((g.debuglevel > 0) && g.f.errout)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        progname) ;

	if (f_usage) goto usage ;

	if ((g.debuglevel > 0) && g.f.errout)
	    bprintf(efp,"%s: debug level %d\n",
	        progname,g.debuglevel) ;

	rs = OK ;
	if (f_version) {

	    if (g.f.errout)
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

/* perform miscellaneous functions */

	cmd_uucico = UUCICO_CMD ;

/* get user information */

	rs = userinfo(&u,userbuf,USERBUFLEN,NULL) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: rs=%d node=%s domain=%s\n",
	        rs,u.nodename,u.domainname) ;
#endif

	g.nodename = u.nodename ;
	g.domainname = u.domainname ;

/* initialize some other common stuff only needed for mail operations */

	g.pid = u.pid ;
	g.uid = u.uid ;
	g.gid = u.gid ;
	g.username = u.username ;
	g.homedir = u.homedname ;
	g.gecosname = u.gecosname ;
	g.mailname = u.mailname ;

	g.euid = geteuid() ;
	if (g.euid != g.uid) seteuid(g.uid) ;

/* get our program root directory */

	if ((g.programroot = getenv("PROGRAMROOT")) == NULL)
	    g.programroot = PROGRAMROOT ;

/* get "mailhost" name */

	if ((g.mailhost = getenv("MAILHOST")) == NULL)
	    g.mailhost = MAILHOST ;

	if ((cp = strchr(g.mailhost,'.')) != NULL)
	    g.fromnode = mallocsbuf(g.mailhost,cp - g.mailhost) ;

	else
	    g.fromnode = g.mailhost ;

/* try to get an authorization file if we need one */

	f_authforce = TRUE ;
	if (authfname == NULL) {

	    f_authforce = FALSE ;
	    bufprintf(buf,BUFLEN,"%s/%s",g.programroot,AUTHFILE) ;

	    authfname = buf ;

	}

	u_buf[0] = '\0' ;
	p_buf[0] = '\0' ;

/* access the authorization file with SETUID permissions if we have any */

	f_euid = FALSE ;
	if ((g.uid != g.euid) && (u_access(authfname,R_OK) < 0)) {

	    f_euid = TRUE ;
	    seteuid(g.euid) ;

	}

	rs = authfile(authfname,u_buf,p_buf) ;

	if (f_euid) seteuid(g.uid) ;

	if (rs == 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: authfile u=%s p=%s\n",
	            u_buf,p_buf) ;
#endif
	    if ((u_buf[0] != '\0') && f_authforce)
	        g.authusername = u_buf ;

	    if ((p_buf[0] != '\0') && f_authforce)
	        g.authpassword = p_buf ;

	} else if (rs > 0) {

	    if (g.f.errout)
	        bprintf(g.efp,"%s: error in authorization file (rs %d)\n",
	            g.progname,rs) ;

	} /* end if (authorization file stuff) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: u=%s p=%s\n",g.authusername,g.authpassword) ;
#endif


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
	if (u_access("/usr/sbin",R_OK) >= 0) g.f.sysvrt = TRUE ;


/* create a log ID */

	bufprintf(buf,BUFLEN,"%s%d",g.nodename,(int) g.pid) ;

	g.logid = mallocstr(buf) ;

/* find the log file */

	if (g.logfname == NULL) {

	    bufprintf(buf,BUFLEN,"%s/%s",g.programroot,LOGFNAME) ;

	    g.logfname = mallocstr(buf) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: trying to open log file \"%s\"\n",
	        g.logfname) ;
#endif

/* open the log file (if we can) */

	if ((g.logfname != NULL) &&
	    (logfile_open(&g.lh,g.logfname,0,0666,g.logid) >= 0)) {

	    g.f.log = TRUE ;

	    buf[0] = '\0' ;
	    if (u.name != NULL)
	        bufprintf(buf,BUFLEN,"(%s)",u.name) ;

	    else if (u.fullname != NULL)
	        bufprintf(buf,BUFLEN,"(%s)",u.fullname) ;

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
	    debugprintf("main: about to check arguments\n") ;
#endif

	if (waittime < 1) waittime = 1 ;

	if ((servicename == NULL) || (*servicename == '\0'))
	    goto badservice ;

	if ((kickhost != NULL) && (kickhost[0] != '\0'))
	    f_kick = TRUE ;


/* parse out the machine part from the queue path part of the 'queuespec' */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to parse the queuespec, q=%s\n",
	        queuespec) ;
#endif

	if (queuespec == NULL)
	    queuespec = DEFQUEUESPEC ;

	else if (*queuespec == '-')
	    queuespec = DEFQUEUESPEC ;

	else {

	    cp = mallocstr(queuespec) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: about to shrink the queuespec, q=%s\n",
	            cp) ;
#endif

	    queuespec = strshrink(cp) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to shrink the queuespec, q=%s\n",
	        queuespec) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: q=%s\n",queuespec) ;
#endif

	logfile_printf(&g.lh,"q=%s\n",queuespec) ;

	cp1 = mallocstr(queuespec) ;

	if ((cp = strpbrk(cp1,"!:")) != NULL) {

	    *cp++ = '\0' ;
	    queue_path = cp ;
	    queue_machine = cp1 ;

	} else {

	    queue_path = DEFQUEUEPATH ;
	    queue_machine = cp1 ;

	}

	if ((g.debuglevel > 0) && g.f.errout) {

	    if (queuespec != NULL)
	        bprintf(efp,"%s: queuespec > %s\n",progname,queuespec) ;

	    if (queue_machine != NULL)
	        bprintf(efp,"%s: queue_machine > %s\n",
	            progname,queue_machine) ;

	    if (queue_path != NULL)
	        bprintf(efp,"%s: queue_path > %s\n",
	            progname,queue_path) ;

	    bflush(efp) ;

	}

/* check out the various addresses that we may be using */

	if (address_reply != NULL) {

	    address_reply = strshrink(address_reply) ;

	    if (((cp = strpbrk(address_reply,"!\\@%/=")) == NULL) &&
	        ((pp = getpwnam(address_reply)) != NULL)) {

	        name_to = ns_mailname(pp->pw_gecos) ;

	        if ((l = strlen(address_reply)) > (BUFLEN - SAVESIZE))
	            address_reply[BUFLEN - SAVESIZE] = '\0' ;

	        bufprintf(buf,BUFLEN,"%s <%s!%s>",name_to,g.nodename,
	            address_reply) ;

	        address_reply = mallocstr(buf) ;

	    }

	    logfile_printf(&g.lh,"reply=%s\n",address_reply) ;

	} /* end if (reply address) */

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

	        bufprintf(buf,BUFLEN,"%s <%s!%s>",name_from,g.nodename,
	            address_from) ;

	        address_from = mallocstr(buf) ;

	    }

	} else {

	    bufprintf(buf,BUFLEN,"%s <%s!%s>",
	        g.mailname,g.fromnode,g.username) ;

	    address_from = mallocstr(buf) ;

	} /* end if (from address processing) */

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

	    bufprintf(buf,BUFLEN,"%s!%s",g.nodename,user) ;

	    address_sender = address_from ;
	    address_from = mallocstr(buf) ;

	} else
	    user = g.username ;

/* make some log entries */

#ifdef	COMMENT
	logfile_printf(&g.lh,"host=%s.%s user=%s(%d)\n",
	    (g.nodename != NULL) ? g.nodename : "*unknown*",
	    (g.domainname != NULL) ? g.domainname : "*unknown*",
	    (user != NULL) ? user : "*unknown*",g.uid) ;
#endif

/* continue address preparation */

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    debugprintf("main: user > %s\n",(user == NULL) ? "" : user) ;

	}
#endif

	if (address_from == NULL) {

	    bufprintf(buf,BUFLEN,"%s!%s",g.nodename,user) ;

	    address_from = mallocstr(buf) ;

	}

	if ((g.debuglevel > 0) && g.f.errout)
	    bprintf(efp,"%s: done with address from \"%s\"\n",
	        progname,address_from) ;

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

	jobfname = mallocstr(buf) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 2\n") ;
#endif

	jobid = mallocstr(strbasename(buf)) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: created job file > %s\n",jobfname) ;
#endif

	if ((g.debuglevel > 0) && g.f.errout)
	    bprintf(efp,"%s: jobid=%s\n",progname,jobid) ;

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

	    if (S_ISREG(sb.st_mode)) {

	        clen = sb.st_size ;
	        f_clen = TRUE ;
	    }

	} else {

	    clen = 0 ;
	    f_clen = TRUE ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: find a suitable transport\n") ;
#endif

/* handle some machine aliases */

	queue_inmachine = queue_machine ;
	queue_uumachine = queue_machine ;
	if (queue_machine != NULL) {

	    dialtab		t ;

	    struct dialentry	*dep ;

	    char	dialfname[MAXPATHLEN + 1] ;


	    bufprintf(dialfname,MAXPATHLEN,"%s/%s",g.programroot,DTFNAME) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: dialtab filename=%s\n",dialfname) ;
#endif

	    f_euid = FALSE ;
	    if ((g.uid != g.euid) && (u_access(dialfname,R_OK) < 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: using SETEUID\n") ;
#endif

	        f_euid = TRUE ;
	        seteuid(g.euid) ;

	    }

	    rs = dialtab_start(&t,dialfname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: dialtab rs %d\n",rs) ;
#endif

	    if (f_euid) seteuid(g.uid) ;

	    if (rs > 0) {

	        if (dialtab_search(&t,queue_machine,&dep) >= 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: got a dialtab match \n") ;
#endif

	            if (dep->uucp != NULL)
	                queue_uumachine = mallocstr(dep->uucp) ;

	            if (dep->inet != NULL)
	                queue_inmachine = mallocstr(dep->inet) ;

	            if ((dep->username != NULL) && (g.authusername == NULL))
	                g.authusername = mallocstr(dep->username) ;

	            if ((dep->password != NULL) && (g.authpassword == NULL))
	                g.authpassword = mallocstr(dep->password) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) {

	                debugprintf("main: dialtab uucp=%s inet=%s\n",
	                    queue_uumachine,queue_inmachine) ;

	                debugprintf("main: dialtab u=%s p=%s\n",
	                    g.authusername,g.authpassword) ;

	            }
#endif

	            logfile_printf(&g.lh,"dialinfo uucp=%s inet=%s\n",
	                (dep->uucp != NULL) ? dep->uucp : "",
	                (dep->inet != NULL) ? dep->inet : "") ;

	            logfile_printf(&g.lh,"dialinfo username=%s password=%s\n",
	                (dep->username != NULL) ? dep->username : "",
	                (dep->password != NULL) ? "********" : "") ;

	        } /* end if (we had it in the dial table) */

	        dialtab_finish(&t) ;

	    } /* end if (we had a dialtab file) */

#ifdef	COMMENT
	    if (strncmp(queue_machine,"mthost2",7) == 0) {

	        queue_inmachine = queue_machine ;
	        queue_uumachine = "mtgbcs" ;

	    } else if (strncmp(queue_machine,"mtgbcs",6) == 0) {

	        queue_inmachine = "mthost2.mt.lucent.com" ;
	        queue_uumachine = queue_machine ;

	    } else if (strncmp(queue_machine,"mtgzfs3",7) == 0) {

	        queue_inmachine = "mthost2.mt.lucent.com" ;
	        queue_uumachine = queue_machine ;

	    } else if (strncmp(queue_machine,"hocpa",5) == 0) {

	        queue_inmachine = "hocpb.ho.lucent.com" ;
	        queue_uumachine = queue_machine ;

	    }
#endif /* COMMENT */

	    uulen = strlen(queue_uumachine) ;

	    if ((cp = strchr(queue_uumachine,'.')) != NULL)
	        uulen = cp - queue_uumachine ;

/* try to use any default authorization if we have any ! */

	    if ((g.authusername == NULL) || (g.authpassword == NULL)) {

	        if ((u_buf[0] != '\0') && (g.authusername == NULL))
	            g.authusername = u_buf ;

	        if ((p_buf[0] != '\0') && (g.authpassword == NULL))
	            g.authpassword = p_buf ;

/* the default authorization information */

	        if (g.authusername == NULL)
	            g.authusername = DEFAUTHUSER ;

	        if (g.authpassword == NULL)
	            g.authpassword = DEFAUTHPASS ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: default u=%s p=%s\n",g.authusername,g.authpassword) ;
#endif

	    } /* end if (the dregs) */

	} /* end if (handle some aliases) */


/* it is never appropriate to expand the queue_path */

#ifdef	COMMENT		

/* expand the queue path if appropriate */

	if ((queue_path != NULL) && (queue_path[0] == '~')) {

	    char	buf2[64] ;


/* length of username in 'l', rest of queue path in 'cp' */

	    if ((cp = strchr(queue_path + 1,'/')) != NULL) {

	        l = cp - (queue_path + 1) ;

	    } else {

	        cp = "" ;
	        l = strlen(queue_path + 1) ;

	    }

	    strwcpy(buf2,queue_path + 1,MIN(l,64)) ;

#if	SYSV
	    pp = getpwnam_r(buf2,
	        &pe,buf,BUFLEN) ;
#else
	    pp = getpwnam(buf2) ;
#endif

	    if (pp == NULL) goto badqueuespec ;

	    bufprintf(cmdbuf,CMDBUFLEN,"%s%s",pp->pw_dir,cp) ;

	    queue_path = mallocstr(cmdbuf) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: expanded queue path> %s\n",queue_path) ;
#endif

	} /* end if (expanding the queue path) */

#endif /* COMMENT */


/* find the user's NETRC file */

	if (g.netfname == NULL) {

	    bufprintf(buf,BUFLEN,"%s/.netrc",u.homedname) ;

	    g.netfname = mallocstr(buf) ;

	}


/* find a suitable transport */

	transport = TRANS_NONE ;
	if (! f_output) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: real transport needed\n") ;
#endif

/* should we use CP as the transport ? */

	    if ((queue_machine != NULL) &&
	        hostequiv(queue_machine,u.nodename,u.domainname))
	        f_localfile = TRUE ;

	    if ((transport == TRANS_NONE) &&
	        (f_localfile || (queue_machine == NULL) ||
	        (queue_machine[0] == '\0'))) {

	        checkuucppublic(&g) ;

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to test CP ? \n") ;
#endif

	        bufprintf(buf,BUFLEN,"%s/q/%s",queue_path,jobid) ;

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: CP file=%s\n",buf) ;
#endif

	        if ((tfd = u_open(buf,O_WFLAGS,0664)) >= 0) {

	            transport = TRANS_CP ;

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("main: we chose CP as transport\n") ;
#endif

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("main: access returned errno=%d\n",errno) ;
#endif
	        }

	    } /* end if (TRANS_CP) */

/* should we use UUCP as the transport ? (assumming that we even have it) */

#if	CF_UUCP

	    if (f_uucp && (transport == TRANS_NONE) && 
	        (queue_machine != NULL) && (queue_machine[0] != '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to test for UUCP ? \n") ;
#endif

	        bufprintf(buf,BUFLEN,"%s/q/%s",queue_path,jobid) ;

	        if ((tfd = uucp(queue_uumachine,buf,NULL)) >= 0)
	            transport = TRANS_UUCP ;

	        else
	            logfile_printf(&g.lh,
	                "UUCP transport was not available (rs %d)\n",
	                tfd) ;

	    } /* end if (TRANS_UUCP) */

#endif /* CF_UUCP */


/* should we use RFILE as the transport ? */

#if	CF_RFILE

	    if ((transport == TRANS_NONE) && (queue_machine != NULL) && 
		(queue_machine[0] != '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to test for RFILE\n") ;
#endif


/* process any NETRC files that we can find */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: about to process the NETRC files\n") ;
#endif

	        if ((rs = veceleminit(&ne,20,0)) < 0)
	            goto badinit ;

	        f_initlist = TRUE ;

/* process the local user's NETRC file */

	        if (g.netfname != NULL)
	            copymachines(&ne,g.netfname,queue_machine,u.domainname) ;

/* process the system-wide NETRC files */

	        bufprintf(buf,BUFLEN,"%s/%s",g.programroot,NETFNAME1) ;

	        copymachines(&ne,buf,queue_machine,u.domainname) ;

	        bufprintf(buf,BUFLEN,"%s/%s",g.programroot,NETFNAME2) ;

	        copymachines(&ne,buf,queue_machine,u.domainname) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processed NETRC files\n") ;
#endif


	        bufprintf(buf,BUFLEN,"%s/q/%s",queue_path,jobid) ;

	        auth.restrict = "any" ;
	        auth.username = g.authusername ;
	        auth.password = g.authpassword ;
	        auth.machinev = (struct netrc **) ne.va ;
	        if ((tfd = rfile(queue_inmachine,&auth,
	            buf, O_WRONLY | O_CREAT,0664)) >= 0)
	            transport = TRANS_RFILE ;

	        else
	            logfile_printf(&g.lh,
	                "RFILE transport was not available (rs %d)\n",
	                tfd) ;

	    } /* end if (TRANS_RFILE) */

#endif /* CF_RFILE */

/* should we use UUCP as the transport ? (assumming that we even have it) */

#if	CF_UUCP

	    if ((! f_uucp) && (transport == TRANS_NONE) && 
	        (queue_machine != NULL) && (queue_machine[0] != '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            debugprintf("main: about to test for UUCP\n") ;
#endif

	        bufprintf(buf,BUFLEN,"%s/q/%s",queue_path,jobid) ;

	        if ((tfd = uucp(queue_uumachine,buf,NULL)) >= 0)
	            transport = TRANS_UUCP ;

	        else
	            logfile_printf(&g.lh,
	                "UUCP transport was not available (rs %d)\n",
	                tfd) ;

	    } /* end if (TRANS_UUCP) */

#endif /* CF_UUCP */

#ifdef	COMMENT
/* use the default local route ?? */

	    if (transport == TRANS_NONE) {

	        bufprintf(buf,BUFLEN,"%s/q",local_path) ;

	        if (u_access(buf,W_OK) == 0) {

	            transport = TRANS_CP ;
	            copy_path = mallocstr(buf) ;

	        }
	    }
#endif /* COMMENT */


	    if (transport == TRANS_NONE) goto badtransport ;

	    if ((g.debuglevel > 0) && (transport >= 0) && g.f.errout)
	        bprintf(efp,
	            "%s: transport %s\n",
	            progname,trans_name[transport]) ;

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: dummy output \n") ;
#endif

	    if ((ofname == NULL) || (ofname[0] == '\0'))
	        ofname = (char *) 1 ;

	    if ((tfd = u_open(ofname,O_WFLAGS,0666)) < 0)
	        goto badoutopen ;

	} /* end if (opening the target FD) */


/* prepare a possible temporary file for interrupts */

	tmpfname[0] = '\0' ;

/* "bopen" the transport */

	if ((rs = bopen(jfp,(char *) tfd,"wct",0664)) < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: write out job headers\n") ;
#endif

/* write out the job file header */

	rs = bprintf(jfp,"x-mailer: AT&T RSLOW (version %s)\n",VERSION) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 'bprintf' rs=%d\n",rs) ;
#endif

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
	    bprintf(jfp,"content-length: %ld\n",clen) ;

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

	cp = strbasename(servicename) ;

	logfile_printf(&g.lh, "srv=%s\n",cp) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: 6b\n") ;
#endif

	j = 1 ;
	for (i = ai ; i < argc ; i += 1) {

	    l = quoteshellarg(argv[i],-1,buf,BUFLEN,&bp) ;

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("main: quoteshellarg rs=%d\n",l) ;
#endif

	    if (l < 0) goto badtoomuch ;

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("main: sharg%d> %W\n",i,bp,l) ;
#endif

	    bprintf(jfp," %W",bp,l) ;

	    logfile_printf(&g.lh,"a%d= %W\n",j,bp,l) ;

	    j += 1 ;

	} /* end for */

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

	bflush(jfp) ;

/* if we have an input file, go through the loops */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf("main: 8\n") ;
#endif

	if (! f_noinput) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: going through the loops\n") ;
#endif

	    jlen = 0 ;
	    while ((len = read(ifd,buf,BUFLEN)) > 0) {

	        i = 0 ;
	        l = len ;
	        while ((rs = bwrite(jfp,buf + i,l)) < l) {

	            if (rs < 0) 
			goto badwrite ;

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

	} /* end if (we had input) */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to close job file\n") ;
#endif

	bflush(jfp) ;

	if ((fstat(tfd,&sb) >= 0) && S_ISSOCK(sb.st_mode))
	    shutdown(tfd,1) ;

	bclose(jfp) ;


/* delete any temporary file for interrupts */

	if (! f_output) {

	    if ((tmpfname != NULL) && (tmpfname[0] != '\0')) {

	        u_unlink(tmpfname) ;

	        tmpfname[0] = '\0' ;
	    }

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: presumably sent job using \"%s\"\n",
	            trans_name[transport]) ;
#endif

	} /* end if (we are not "outputting" the job file) */

	if (jobfname != NULL) {

	    if (f_jobfilepath &&
	        (bopen(ofp,BFILE_STDOUT,"wct",0666) >= 0)) {

	        bprintf(ofp,"%s\n",jobfname) ;

	        bclose(ofp) ;

	    }

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: deleting jobfname=%s\n",jobfname) ;
#endif

	    delete_jobfile(jobfname) ;

	}

	if ((tmpfname != NULL) && (tmpfname[0] != '\0')) {

	    u_unlink(tmpfname) ;

	    tmpfname[0] = '\0' ;
	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: finishing up\n") ;
#endif

/* finish up */
done:
	if (f_initlist) vecelemfree(&ne) ;

	if ((g.debuglevel > 0) && g.f.errout)
	    bprintf(efp,"%s: job \"%s\" sent\n",progname,jobid) ;

	logfile_printf(&g.lh,"job \"%s\" sent using %s\n",
	    jobid,trans_name[transport]) ;

	if (transport == TRANS_UUCP)
	    logfile_printf(&g.lh,"qp=%s\n",queue_path) ;

	if (f_verbose && g.f.errout)
	    bprintf(efp,"%s: jobid %s\n",progname,jobid) ;

	logfile_close(&g.lh) ;

/* take the early return here */
earlyret:
	if (g.f.errout)
	    bclose(efp) ;

	return transport ;

/* bad returns come here */
badret:

#if	CF_DEBUG
	if (g.f.errout)
	    bflush(efp) ;
#endif

	if (jobfname != NULL)
	    delete_jobfile(jobfname) ;

	if ((tmpfname != NULL) && (*tmpfname != '\0'))
	    u_unlink(tmpfname) ;

	logfile_close(&g.lh) ;

	if (g.f.errout)
	    bclose(efp) ;

	return BAD ;

/* display a useage report */
usage:
	if (g.f.errout) {

	    bprintf(efp,
	        "%s: USAGE> rslow [-?VD] mach!path | - ",
	        progname) ;

	    bprintf(efp,"[-UR] [-u user] [-d directory]") ;

	    bprintf(efp,"\n\t[-mp] [-f from] [-r reply] ") ;

	    bprintf(efp,"service [service_args ...]\n") ;

	}

	goto badret ;

badargextra:
	if (g.f.errout)
	    bprintf(efp,"%s: no value associated with this option key\n",
	        progname) ;

	goto badret ;

badargvalue:
	if (g.f.errout)
	    bprintf(efp,"%s: bad argument value was specified\n",
	        progname) ;

	goto badret ;

badargnum:
	if (g.f.errout)
	    bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badarg:
	if (g.f.errout)
	    bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badinit:
	if (g.f.errout)
	    bprintf(efp,"%s: failed internal memory allocation\n",progname) ;

	goto badret ;

badinfile:
	if (g.f.errout)
	    bprintf(efp,"%s: cannot open the input file (errno %d)\n",
	        progname,errno) ;

	goto badret ;

badoutopen:
	if (g.f.errout)
	    bprintf(efp,"%s: cannot open the output file (rs %d)\n",
	        progname,rs) ;

	goto badret ;

badwrite:
	logfile_printf(&g.lh,"failed on write to temporary file (rs %d)\n",
	    rs) ;

	if (g.f.errout)
	    bprintf(efp,"%s: failed on write to temporary file (rs %d)\n",
	        progname,rs) ;

	goto badret ;

badjobread:
	logfile_printf(&g.lh,"failed on read from job file (rs %d)\n",
	    rs) ;

	if (g.f.errout)
	    bprintf(efp,"%s: failed on read from temporary file (rs %d)\n",
	        progname,rs) ;

	goto badret ;

badstat:
	if (g.f.errout)
	    bprintf(efp,"%s: could not get status on input file (errno %d)\n",
	        progname,errno) ;

	goto badret ;

badjobfile:
	logfile_printf(&g.lh,"could not make a job file (rs %d)\n",
	    rs) ;

	if (g.f.errout)
	    bprintf(efp,"%s: could not make a job file (rs %d)\n",
	        progname,rs) ;

	goto badret ;

badjobopen:
	logfile_printf(&g.lh,"could not open the job file (rs %d)\n",
	    rs) ;

	if (g.f.errout)
	    bprintf(efp,"%s: could not open the job file (rs %d)\n",
	        progname,rs) ;

	goto badret ;

badtoomuch:
	logfile_printf(&g.lh,"arguments are too long\n") ;

	if (g.f.errout)
	    bprintf(efp,"%s: arguments are too long\n",
	        progname) ;

	goto badret ;

badtransport:
	logfile_printf(&g.lh,"no suitable transport provider was found\n") ;

	if (g.f.errout)
	    bprintf(efp,"%s: no suitable transport provider was found\n",
	        progname) ;

	goto badret ;

badservice:
	if (g.f.errout)
	    bprintf(efp,"%s: bad or no service specification was given\n",
	        progname) ;

	goto badret ;

badqueuespec:
	if (g.f.errout)
	    bprintf(efp,"%s: bad queue specification was given\n",
	        progname) ;

	goto badret ;

badrexec:
	logfile_printf(&g.lh,"internal program error in REXEC transport\n") ;

	if (g.f.errout)
	    bprintf(g.efp,"%s: internal program error\n",
	        g.progname) ;

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

	u_unlink(tmpfname) ;

	return rs ;
}
/* end subroutine (make_intfile) */


static int delete_jobfile(jobfname)
char	jobfname[] ;
{
	int	rs, i, err ;


	if ((jobfname == NULL) || (*jobfname == '\0'))
		return OK ;

	return unlinkd(jobfname,-1) ;
}
/* end subroutine (delete_jobfile) */


/* check if the UUCPPUBLIC spool area has the proper directories */
static int checkuucppublic(gp)
struct global	*gp ;
{
	bfile		file0, file1, file2, *fpa[3] ;

	struct passwd	pe, *pp = &pe ;

	pid_t		cpid ;

	int		rs, srs ;
	int		len ;
	int		childstat ;

	char		buf[BUFLEN + 1] ;
	char		linebuf[LINELEN + 1] ;
	char		*dirp ;


/* if we are not a real UUCP site (as evidenced by no UUCP hosts), punt out */

	fpa[0] = &file0 ;
	fpa[1] = &file1 ;
	fpa[2] = &file2 ;
	if ((cpid = bopencmd(fpa,"uuname")) < 0)
		return BAD ;

	bclose(fpa[0]) ;

	len = breadline(fpa[1],linebuf,LINELEN) ;

	bdump(&file1,&file2) ;

	bclose(fpa[1]) ;

	bclose(fpa[2]) ;

	waitpid(cpid,&childstat,WUNTRACED) ;

	if (len <= 0) return BAD ;

/* OK, we go for it */

	srs = OK ;
	dirp = DEFQUEUEPATH ;
	if (u_access(dirp,W_OK) < 0) {

	    if (u_mkdir(dirp,0777) >= 0) {

	        u_chmod(dirp,0777) ;

	        if ((g.debuglevel > 1) && g.f.errout)
	            bprintf(gp->efp,
	                "%s: made the UUCPPUBLIC 'rslow' directory\n",
	                gp->progname) ;

	        logfile_printf(&gp->lh,
	            "made the UUCPPUBLIC 'rslow' directory\n") ;

		if (getpw_name(&pe,buf,BUFLEN,USERNAME_UUCP) >= 0)
	            chown(dirp, pp->pw_uid,-1) ;

	    } /* end if (we made the directory) */

	} /* end if */

	dirp = buf ;
	bufprintf(buf,BUFLEN,"%s/%s",DEFQUEUEPATH,"q") ;

	if (u_access(dirp,W_OK) < 0) {

	    rs = u_mkdir(dirp,0777) ;

	    if (rs < 0) {

	        if (srs >= 0) srs = rs ;

	    }

	    if (rs >= 0)
	        chmod(dirp,0777) ;

	    if ((gp->debuglevel > 1) && g.f.errout)
	        bprintf(gp->efp,
	            "%s: tried to make \"%s\" (rs %d)\n",
	            gp->progname,dirp,rs) ;

	    logfile_printf(&gp->lh,
	        "tried to make \"%s\" (rs %d)\n",
	        dirp,rs) ;

	} /* end if */

	return srs ;
}
/* end subroutine (checkuucppublic) */



