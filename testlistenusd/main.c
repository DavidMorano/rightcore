/* main */

/* testing */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	F_STDOUT	0		/* write to STDOUT also ? */


/* revision history:

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 88/02/10, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/


/************************************************************************

	Call as :

	$ testlistenusd path



*************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<libgen.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
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

#define	TO		10
#define	NIOVECS		10
#define	POLLINT		10
#define	POLLINTMULT	1000



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(char *const *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	listenusd(const char *,int,int) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	testlistenusd_makedate[] ;


/* local structures */


/* forward references */

static void	helpfile(const char *,bfile *) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* global variables */


/* local variables */

static char *const argopts[] = {
	    "ROOT",
	    "DEBUG",
	    "VERSION",
	    "VERBOSE",
	    "HELP",
	    "LOG",
	    "MAKEDATE",
	    NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4
#define	ARGOPT_LOG		5
#define	ARGOPT_MAKEDATE		6






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct msghdr	mh ;

	struct pollfd	fds[3] ;

	struct iovec	vecs[NIOVECS] ;

	struct ustat	sb ;

	struct userinfo	u ;

	struct proginfo	pi, *pip = &pi ;

	LFM		lk ;

	SOCKADDRESS	from ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;
	bfile		pidfile ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	rs, rs1, len, argnum ;
	int	ex = EX_USAGE ;
	int	nfds, nfd, to_poll ;
	int	size, blen, sl, re ;
	int	fd_debug ;
	int	fd_stdout, fd_ipc ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help ;
	int	f ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	tmpbuf[MAXPATHLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	fromname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*logfname = NULL ;
	char	*hostname = NULL ;
	char	*portspec = NULL ;
	char	*cp ;


	if (((cp = getenv(VARDEBUGFD)) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: fd_debug=%d\n",fd_debug) ;
#endif

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {

	    u_close(2) ;

	    pip->efp = efp ;
	    bcontrol(efp,BC_LINEBUF,0) ;

	}


	pip->debuglevel = 0 ;
	pip->verboselevel = 0 ;

	pip->f_exit = FALSE ;

	pip->f.verbose = FALSE ;
	pip->f.quiet = FALSE ;

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

	            if (isdigit(argp[1])) {

	                if (cfdeci(argp + 1,argl - 1,&argnum))
	                    goto badargvalue ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

/* do we have a keyword match or should we assume only key letters ?
					    */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->pr = argp ;

	                        }

	                        break ;

/* debug level */
	                    case ARGOPT_DEBUG:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf("main: debug flag, avp=\"%t\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                                if (rs < 0)
	                                    goto badargvalue ;

	                            }

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        pip->f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->helpfname = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

/* log file */
	                    case ARGOPT_LOG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                logfname = avp ;

	                        }

	                        break ;

/* display the time this program was last "made" */
	                    case ARGOPT_MAKEDATE:
	                        f_makedate = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                                if (rs < 0)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                pip->progname,*aop) ;

/* fall through from above */
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


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    bprintf(efp,"%s: built %s\n",
	        pip->progname,testlistenusd_makedate) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto exit ;


	if (f_help) {

	    if (pip->helpfname == NULL) {

	        blen = mkpath2(buf, pip->pr,HELPFNAME) ;

	        pip->helpfname = (char *) mallocstrw(buf,blen) ;

	    }

	    helpfile(pip->helpfname,pip->efp) ;

	    goto exit ;

	} /* end if */


/* get our program root (if we have one) */

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT1) ;

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT2) ;

	if (pip->pr == NULL)
	    pip->pr = PROGRAMROOT ;


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->pr) ;



/* who are we ? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->pid = u.pid ;


	u_time(&daytime) ;


/* do we have a log file ? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {

	    len = mkpath2(buf, pip->pr,logfname) ;

	    logfname = mallocstrw(buf,len) ;

	}

/* make a log entry */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: logfile=%s\n",
	        logfname) ;
#endif

	if ((rs = logfile_open(&pip->lh,logfname,0,0666,u.logid)) >= 0) {

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    else if (u.mailname != NULL)
	        sprintf(buf,"(%s)",u.mailname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	        timestr_log(daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	    logfile_printf(&pip->lh,"os=%s %s!%s %s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),u.nodename,u.username,buf) ;

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


/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf(
	                    "main: got positional arg i=%d pan=%d arg=%s\n",
	                    i,pan,argv[i]) ;
#endif

	            switch (pan) {

	            case 0:
	                portspec = argv[i] ;
	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} /* end if (getting arguments) */




	if ((hostname == NULL) || (hostname[0] == '\0'))
	    hostname = "rca" ;

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = "echo" ;


	fd_stdout = FD_STDOUT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: portspec=%s\n", portspec) ;
#endif

	if (u_stat(portspec,&sb) >= 0)
	    u_unlink(portspec) ;

	rs = listenusd(portspec,0666,0) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: listenusd() rs=%d\n",rs) ;
#endif

	fd_ipc = rs ;
	if (rs < 0) {

	    bprintf(pip->efp,
	        "%s: listen address already in use (%d)\n",
	        pip->progname,rs) ;

	    goto badlisten ;
	}


	rs1 = uc_fchmodsuid(fd_ipc,TRUE) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: uc_fchmodsuid() rs=%d\n",rs1) ;
		u_fstat(fd_ipc,&sb) ;
		f = (sb.st_mode & S_ISUID) ? 1 : 0 ;
	    debugprintf("main: FD perm S_ISUID=%u\n",f) ;
		u_stat(portspec,&sb) ;
		f = (sb.st_mode & S_ISUID) ? 1 : 0 ;
	    debugprintf("main: file perm S_ISUID=%u\n",f) ;
	}
#endif

	rs1 = uc_chmodsuid(portspec,TRUE) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: uc_chmodsuid() rs=%d\n",rs1) ;
		u_fstat(fd_ipc,&sb) ;
		f = (sb.st_mode & S_ISUID) ? 1 : 0 ;
	    debugprintf("main: FD perm S_ISUID=%u\n",f) ;
		u_stat(portspec,&sb) ;
		f = (sb.st_mode & S_ISUID) ? 1 : 0 ;
	    debugprintf("main: file perm S_ISUID=%u\n",f) ;
	}
#endif

	nfds = 0 ;

	fds[nfds].fd = fd_ipc ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	fds[nfds].fd = -1 ;


	size = NIOVECS * sizeof(struct iovec) ;
	(void) memset(&vecs,0,size) ;

	vecs[0].iov_base = buf ;
	vecs[0].iov_len = BUFLEN ;


	(void) memset(&mh,0,sizeof(struct msghdr)) ;

	mh.msg_name = &from ;
	mh.msg_namelen = sizeof(SOCKADDRESS) ;
	mh.msg_iov = vecs ;
	mh.msg_iovlen = NIOVECS ;



	to_poll = POLLINTMULT * POLLINT ;

	while (TRUE) {

	    rs = u_poll(fds,nfds,to_poll) ;

	    if (rs < 0)
	        break ;

	    if (rs > 0) {

	        for (nfd = 0 ; nfd < nfds ; nfd += 1) {


/* handle any activity on our request FIFO */

	            if ((fds[nfd].fd == fd_ipc) &&
	                ((re = fds[nfd].revents) != 0)) {

	                if ((re & POLLIN) || (re & POLLPRI)) {


#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("main: back from poll, FD=%d re=%s\n",
	                            fds[nfd].fd,
	                            d_reventstr(re,tmpbuf,BUFLEN)) ;
#endif

	                    mh.msg_namelen = sizeof(SOCKADDRESS) ;

	                    vecs[0].iov_len = BUFLEN ;

	                    rs = u_recvmsg(fd_ipc,&mh,0) ;

	                    len = rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2)) {
	                        char	hexbuf[100 + 1] ;

	                        debugprintf("main: IPC recvmsg() rs=%d\n",rs) ;
	                        debugprintf("main: IPC msg_namelen=%d\n",
					mh.msg_namelen) ;

	                        sl = mkhexstr(hexbuf,100,&from,32) ;

	                        debugprintf("main: from=%t\n",hexbuf,sl) ;

	                        sl = sockaddress_getaddr(&from,
	                            fromname,MAXPATHLEN) ;

	                        debugprintf("main: flen=%d from=%t\n",
	                            sl,fromname,sl) ;
	                    }
#endif /* CF_DEBUG */

	                    if (len >= 0) {

	                        vecs[0].iov_len = len ;

#if	F_STDOUT
	                        rs = u_writev(fd_stdout,vecs,NIOVECS) ;
#endif

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: u_writev() rs=%d\n",rs) ;
#endif /* CF_DEBUG */


	                        if (mh.msg_namelen > 0) {

					mh.msg_accrights = NULL ;
	                            rs = u_sendmsg(fd_ipc,&mh,0) ;

				}

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: IPC sendmsg() rs=%d\n",rs) ;
#endif /* CF_DEBUG */


	                    } /* end if (non-zero length) */

	                } /* end if (readable) */


	            } /* end if (our request FIFO) */


	        } /* end for (looping through possible FDs) */

	    } /* end if (something from 'poll') */


/* more stuff to poll */

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("main: random other poll\n") ;
#endif


	} /* end while */


done:
	u_close(fd_stdout) ;


/* close off and get out ! */
exit:
earlyret:
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

	goto earlyret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto earlyret ;

baduser:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	goto badret ;

badret:
	ex = EX_USAGE ;
	goto earlyret ;

badlisten:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;


	if ((f == NULL) || (f[0] == '\0'))
	    return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



