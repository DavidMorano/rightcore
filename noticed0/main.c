/* main */

/* generic test front-end */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Synopsis:

	$ noticed


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<sockaddress.h>
#include	<netorder.h>
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



/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	listenusd(const char *,int) ;

extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static const char *argopts[] = {
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

static const char	*progmodes[] = {
	"filesize",
	"filefind",
	NULL
} ;

enum progmodes {
	progmode_filesize,
	progmode_filefind,
	progmode_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct pollfd	fds[3] ;

	struct msghdr	mh ;

	struct iovec	vecs[NIOVECS] ;

	struct ustat	sb ;

	USERINFO	u ;

	LFM		lk ;

	SOCKADDRESS	from ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;
	bfile		pidfile ;

	int	argr, argl, aol, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int	srs, len, i ;
	int	nfds, nfd, to_poll ;
	int	size, blen, sl, re ;
	int	fd_stdout, fd_ipc ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	tmpdname[MAXPATHLEN + 1] ;
	char	tmpbuf[MAXPATHLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	fromname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*pr = NULL ;
	char	*searchname = NULL ;
	char	*logfname = NULL ;
	char	*hostname = NULL ;
	char	*portspec = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < NARGPRESENT ; ai += 1) 
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

		    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

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

/* do we have a keyword or only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

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

/* debug level */
	                    case argopt_debug:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                            }

	                        }

	                        break ;

	                    case argopt_version:
	                        f_version = TRUE ;
	                        break ;

	                    case argopt_verbose:
	                        pip->f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case argopt_help:
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

	                    while (akl--) {

	                        switch ((uint) *akp) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;


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

	                        case '?':
	                            f_usage = TRUE ;
					break ;

	                        default:
				rs = SR_INVALID ;
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

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

/* program search name */

	if (searchname == NULL)
	    searchname = getenv(VARSEARCHNAME) ;

#ifdef	COMMENT
	if ((searchname == NULL) && (pmspec != NULL)) {

	    searchname = pmspec ;

	}
#endif /* COMMENT */

	if (searchname == NULL) {

	    searchname = pip->progname ;
	    if ((cp = strchr(pip->progname,'.')) != NULL) {

	        searchname = tmpfname ;
	        strwcpy(tmpfname,pip->progname,(cp - pip->progname)) ;

	    }
	}

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->searchname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filesize ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

/* help file */

	if (f_help)
	    goto help ;


/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* who are we? */

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;

	if (rs < 0)
	    goto baduser ;

	pip->pid = u.pid ;


	daytime = time(NULL) ;


/* do we have a log file? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {
	    len = mkpath2(buf,pip->pr,logfname) ;
	    logfname = buf ;
	}

	    proginfo_setentry(pip,&pip->logfname,logfname,-1) ;

/* make a log entry */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: logfile=%s\n",
	        pip->logfname) ;
#endif

	rs = logfile_open(&pip->lh,pip->logfname,0,0666,u.logid) ;

	if (rs >= 0) {

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

	rs = listenusd(portspec,0) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: listenusd() rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    bprintf(pip->efp,
	        "%s: listen address already in use (%d)\n",
	        pip->progname,rs) ;

	    goto badlisten ;
	}


	fd_ipc = rs ;

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



	while (TRUE) {

	    rs = u_poll(fds,nfds,to_poll) ;

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
	                    if (pip->debuglevel > 1) {
	                        char	hexbuf[100 + 1] ;

	                        debugprintf("main: IPC recvmsg() rs=%d\n",rs) ;

	                        sl = mkhexstr(hexbuf,100,&from,16) ;

	                        debugprintf("main: from=%W\n",hexbuf,sl) ;

	                        sl = sockaddress_getaddr(&from,
	                            fromname,MAXPATHLEN) ;

	                        debugprintf("main: flen=%d from=%W\n",
	                            sl,fromname,sl) ;
	                    }
#endif /* CF_DEBUG */

	                    if (len >= 6) {

	                        uint	tag ;

	                        ushort	svc ;


	                        i = netorder_rushort(buf,&svc) ;

	                        i += netorder_ruint(buf + i,&tag) ;


	                        rs = SR_NOTFOUND ;
	                        if (svc == 1) {

/* try to send a notice to a terminal of the user */

	                            rs = srv_userterm(pip,&mh,len,fd_ipc) ;


	                        }

	                        if (rs < 0) {

	                            sl = sockaddress_getaddr(&from,
	                                fromname,MAXPATHLEN) ;

	                            if (sl > 0) {

	                                rs = SR_NOTFOUND ;
	                                netorder_wint(buf + 6,rs) ;

	                                vecs[0].iov_len = (6 + 4) ;
	                                rs = u_sendmsg(fd_ipc,&mh,0) ;

	                            }
	                        }

	                    } /* end if (non-zero length message) */

	                } /* end if (incoming message) */

	            } /* end if (our request FIFO) */

	        } /* end for (looping through possible FDs) */

	    } /* end if (something from 'poll') */


	} /* end while */


done:
	u_close(fd_stdout) ;


/* close off and get out ! */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* what are we about? */
usage:
	usage(pip) ;

	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

baduser:
	EX_NOUSER ;
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	goto badret ;

badlisten:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [mailfile [offset]] [-sV] [-t timeout] [-offset]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "\t[-d terminal_device]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



