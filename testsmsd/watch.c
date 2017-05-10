/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGSLEEP	0
#define	CF_DEBUGFROM	1
#define	CF_WHOOPEN	0		/* who is open */
#define	CF_SIGCHILD	1		/* catch SIGCHLD ? */
#define	CF_SNDDD	1


/* revision history:

	= 1991-09-01, David A­D­ Morano

	This subroutine was adopted from the DWD program.  I may not
	have changed all of the comments correctly though!


	= 2003-06-23, David A­D­ Morano

	I updated this subroutine to just poll for machine status and
	write the Machine Status (MS) file.  This was a cheap excuse
	for not writing a whole new daemon program just to poll for
	machine status.  I hope this works out! :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for listening on the given
	socket and spawning off a program to handle any incoming
	connection.  Some of the "internal" messages are handled here
	(the easy ones -- or the ones that fit here best).  The rest
	(that look like client-sort-of requests) are handled in the
	'standing' object module.

	Synopsis:

	int watch(pip,bip)
	struct proginfo	*pip ;
	BUILTIN		*bip ;

	Arguments:

	pip	program information pointer

	Returns:

	OK	doesn't really matter in the current implementation
	<0	error


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/msg.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<poll.h>
#include	<signal.h>
#include	<dirent.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"jobdb.h"
#include	"builtin.h"
#include	"config.h"
#include	"defs.h"
#include	"standing.h"
#include	"sysmisc.h"
#include	"imsg.h"


/* local defines */

#define	IPCDIRMODE	0777
#define	W_OPTIONS	(WNOHANG)
#define	TI_POLL		10		/* u_poll(2) interval seconds */
#define	TI_MAINT	(3 * 60)	/* miscellaneous maintenance */
#define	TI_RECVMSG	3
#define	IPCBUFLEN	MSGBUFLEN
#define	MAXOUTLEN	62
#define	NIOVECS		1
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpdir(struct proginfo *,char *) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;

extern int	checklockfile(struct proginfo *,bfile *,char *,char *,
			time_t,pid_t) ;
extern int	handle(struct proginfo *,
			BUILTIN *,STANDING *, struct clientinfo *) ;
extern int	checkdirs(struct proginfo *,char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	openipc(struct proginfo *,char *) ;
static int	closeipc(struct proginfo *,char *) ;
static int	writeout() ;
static int	watch_newjob(struct proginfo *,
			BUILTIN *, STANDING *, struct clientinfo *,int,int) ;

#if	CF_DEBUG
static int	printsubs(struct proginfo *,const char *) ;
#endif

#ifdef	COMMENT
static int	write_mqid() ;
#endif

static void	int_exit(int) ;
static void	int_child(int) ;


/* local variables */

/* writable */

static int	f_exit, f_child ;


/* exported subroutines */


int watch(pip,bip)
struct proginfo	*pip ;
BUILTIN		*bip ;
{
	struct pollfd		fds[4] ;

	struct sigaction	sigs ;

	struct ustat		sb ;

	struct clientinfo	ci, *cip = &ci ;

	struct msghdr	ipcmsg ;

	struct iovec	vecs[NIOVECS] ;

	STANDING	ourstand ;

	JOBDB_ENT	*jep ;

	SOCKADDRESS	ipcfrom ;

	pid_t		pid ;

	sigset_t	signalmask ;

	time_t		ti_start ;
	time_t		ti_lockcheck = 1 ;
	time_t		ti_pidcheck = 1 ;
	time_t		ti_lastmark = 0 ;

	int	rs = SR_BAD, rs1, i, len, blen ;
	int	re, ns ;
	int	size, cl ;
	int	to_pollidle = TI_POLL ;
	int	to_pollcheck = TI_POLL ;
	int	to_poll ;
	int	loopcount = 0 ;
	int	njobs = 0 ;
	int	child_stat ;
	int	nfds, nfd ;
	int	ifd, ofd, efd ;
	int	fromlen ;
	int	f_logchange = FALSE ;

	char	reqfname[MAXPATHLEN + 2] ;
	char	jobdname[MAXPATHLEN + 1] ;
	char	ipcbuf[IPCBUFLEN + 2] ;
	char	resbuf[MSGBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;

#if	CF_DEBUG
	char	tmpbuf[BUFLEN + 1] ;
#endif


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("watch: entered \n") ;
	    debugprintf("watch: fd_listentcp=%d\n",pip->fd_listentcp) ;
	    debugprintf("watch: fd_listenpass=%d\n",pip->fd_listenpass) ;
	}
#endif


	reqfname[0] = '\0' ;
	jobdname[0] = '\0' ;

	f_exit = f_child = FALSE ;

	pip->fd_req = -1 ;

	pip->subserial = 0 ;
	(void) memset(cip,0,sizeof(struct clientinfo)) ;


	size = NIOVECS * sizeof(struct iovec) ;
	(void) memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;


	(void) memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = &ipcfrom ;
	ipcmsg.msg_namelen = sizeof(SOCKADDRESS) ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("watch: initialization srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */


/* more initialization */

	rs = mktmpdir(pip,jobdname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("watch: mktmpdir() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = jobdb_init(&pip->jobs,4,jobdname) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: jobdb_init rs=%d\n",rs) ;
#endif


	if (rs < 0)
	    goto bad1 ;

	if (pip->f.daemon) {

	    pip->daytime = time(NULL) ;

	    ti_lastmark = pip->daytime ;

	}


/* setup the socket for IPC */

	rs = openipc(pip,reqfname) ;

	pip->fd_req = rs ;
	if (rs < 0) {

	    bprintf(pip->efp,"%s: could not create IPC portal (%d)\n",
	        pip->progname,rs) ;

	    goto bad1a ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: MSG FD=%d\n",pip->fd_req) ;
#endif


/* initialize the standing server part */

	rs = standing_init(&ourstand,pip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: standing_init() rs=%d\n",rs) ;
#endif


/* we want to receive the new socket (from 'accept') above these guys */

	if (pip->f.daemon) {

	    for (i = 0 ; i < 3 ; i += 1) {

	        if (u_fstat(i,&sb) < 0)
	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	    }

	} /* end if (daemon mode) */


	memset(&sigs,0,sizeof(struct sigaction)) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;

#if	CF_SIGCHILD

	(void) uc_sigsetempty(&signalmask) ;

	sigs.sa_handler = int_child ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = SA_NOCLDSTOP ;
	u_sigaction(SIGCHLD,&sigs,NULL) ;

#endif /* CF_SIGCHILD */


/* let's go! */

	nfds = 0 ;

	if (pip->f.daemon) {

	    if (pip->fd_listentcp >= 0) {

	        fds[nfds].fd = pip->fd_listentcp ;
	        fds[nfds].events = POLLIN | POLLPRI ;
	        fds[nfds].revents = 0 ;
	        nfds += 1 ;

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("watch: listening on TCP FD=%d\n",
	                pip->fd_listentcp) ;
#endif

	    }

	    if (pip->fd_listenpass >= 0) {

	        fds[nfds].fd = pip->fd_listenpass ;
	        fds[nfds].events = POLLIN | POLLPRI ;
	        fds[nfds].revents = 0 ;
	        nfds += 1 ;

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("watch: listening on PASS FD=%d\n",
	                pip->fd_listenpass) ;
#endif

	    }

	} /* end if (daemon mode) */

	fds[nfds].fd = pip->fd_req ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	fds[nfds].fd = -1 ;


/* if we are not in daemon mode, then we have a job waiting on FD_STDIN */

	if (! pip->f.daemon) {

#if	CF_DEBUG && CF_WHOOPEN
	    if (pip->debuglevel >= 4)
	        d_whoopen("not daemon initial") ;
#endif

	    (void) memset(cip,0,sizeof(struct clientinfo)) ;

	    ifd = FD_STDIN ;
	    fromlen = sizeof(SOCKADDRESS) ;
	    rs1 = u_getpeername(ifd,&cip->sa,&fromlen) ;

	    if (rs1 >= 0)
	        cip->salen = fromlen ;

	    ofd = (isasocket(ifd)) ? ifd : FD_STDOUT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("watch: watch_newjob() srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

	    rs = watch_newjob(pip,bip,&ourstand,cip,ifd,ofd) ;

	    if (rs < 0) {

	        cp = "could not allocate job resource\n" ;
	        uc_writen(FD_STDOUT,cp,strlen(cp)) ;

	        logfile_printf(&pip->lh,"jobdb new failed, rs=%d\n",rs) ;

	    }

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	} /* end if (not running daemon mode) */


	ti_start = pip->daytime ;
	loopcount = 0 ;

/* top of loop */
top:
	f_logchange = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"loop") ;
#endif

/* do the poll */

	if (njobs <= 0)
		to_poll = to_pollidle * POLLINTMULT ;

	else
		to_poll = to_pollidle * POLLINTMULT / 2 ;

	if (f_child)
	    to_poll = 100 ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: about to poll to=%d\n",to_poll) ;
#endif

	f_child = FALSE ;
	rs = u_poll(fds,nfds,to_poll) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: back from poll w/ rs=%d f_child=%d\n",
	        rs,f_child) ;
#endif

	pip->daytime = time(NULL) ;

	if (rs > 0) {

	    for (nfd = 0 ; nfd < nfds ; nfd += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel >= 4) {

	            re = fds[nfd].revents ;
	            debugprintf("watch: nfd=%d FD=%2d re=>%s<\n",
	                nfd,fds[nfd].fd,
	                d_reventstr(re,tmpbuf,BUFLEN)) ;

	            rs1 = u_fstat(fds[nfd].fd,&sb) ;

	            debugprintf("watch: nfd=%d FD=%2d open=%d\n",
	                nfd,fds[nfd].fd,
	                (rs1 >= 0)) ;

	        }
#endif /* CF_DEBUG */

/* handle any activity on our listen socket or FIFO */

	        if (((fds[nfd].fd == pip->fd_listentcp) ||
	            (fds[nfd].fd == pip->fd_listenpass)) &&
	            ((re = fds[nfd].revents) != 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel >= 4)
	                debugprintf("watch: back from poll, FD=%d re=>%s<\n",
	                    fds[nfd].fd,
	                    d_reventstr(re,tmpbuf,BUFLEN)) ;
#endif

	            if ((re & POLLIN) || (re & POLLPRI)) {

#if	CF_DEBUG
	                if (pip->debuglevel >= 4)
	                    debugprintf("watch: got a poll in, n=%d p=%d\n",
	                        (re & POLLIN) ? 1 : 0,(re & POLLPRI) ? 1 : 0) ;
#endif

			(void) memset(cip,0,sizeof(struct clientinfo)) ;

	                if (fds[nfd].fd == pip->fd_listentcp) {

	                    fromlen = sizeof(SOCKADDRESS) ;
	                    rs = u_accept(pip->fd_listentcp,&cip->sa,&fromlen) ;

	                    ns = rs ;
	                    cip->salen = fromlen ;

#if	CF_DEBUG
	                    if (pip->debuglevel >= 4)
	                        debugprintf("watch: u_accept() rs=%d\n",rs) ;
#endif

	                } else if (fds[nfd].fd == pip->fd_listenpass) {

	                    struct strrecvfd	passer ;


	                    cip->salen = -1 ;
	                    rs = acceptpass(pip->fd_listenpass,&passer,-1) ;

	                    ns = passer.fd ;

#if	CF_DEBUG
	                    if (pip->debuglevel >= 4)
	                        debugprintf("watch: acceptpass() rs=%d ns=%d\n",
	                            rs,ns) ;
#endif

	                    fromlen = sizeof(SOCKADDRESS) ;
	                    rs1 = u_getpeername(ns,&cip->sa,&fromlen) ;

	                    if (rs1 >= 0)
	                        cip->salen = fromlen ;

	                } /* end if (getting FD of new connection) */

/* pop it if we have it */

	                if (rs >= 0) {

	                    rs = watch_newjob(pip,bip,&ourstand,cip,ns,ns) ;

	                    if (rs >= 0)
	                        njobs += 1 ;

	                    u_close(ns) ;

	                    if (rs < 0) {

	                        cp = "could not allocate job resource\n" ;
	                        uc_writen(ns,cp,strlen(cp)) ;

	                        logfile_printf(&pip->lh,
	                            "jobdb new failed, rs=%d\n",rs) ;

	                    } else
	                        pip->subserial += 1 ;

	                } else {

	                    logfile_printf(&pip->lh,
	                        "no FD was given to us, rs=%d\n",rs) ;

	                } /* end if (nothing was there) */

	            } else if (re & POLLHUP)
	                goto badhup ;

	            else if (re & POLLERR)
	                goto baderr ;

	        } /* end if (our listen socket) */


/* handle activity on the intra-program service message portal */

	        if ((fds[nfd].fd == pip->fd_req) &&
	            ((re = fds[nfd].revents) != 0)) {

	            if ((re & POLLIN) || (re & POLLPRI)) {

			struct imsg_response	m0 ;

			struct imsg_noop	m1 ;

			struct imsg_exit	m2 ;

			struct imsg_passfd	m3 ;

	                uint	rcode ;


	                vecs[0].iov_base = ipcbuf ;
	                vecs[0].iov_len = IPCBUFLEN ;

	                ipcmsg.msg_name = &ipcfrom ;
	                ipcmsg.msg_namelen = sizeof(SOCKADDRESS) ;
	                ipcmsg.msg_iov = vecs ;
	                ipcmsg.msg_iovlen = NIOVECS ;
			ipcmsg.msg_accrights = (caddr_t) &ns ;
			ipcmsg.msg_accrightslen = sizeof(int) ;

	                rs = u_recvmsg(pip->fd_req,&ipcmsg,0) ;

	                len = rs ;
			if (ipcmsg.msg_accrightslen <= 0)
				ns = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	                if (pip->debuglevel >= 4) {
			    char	fromfname[MAXPATHLEN + 1] ;

	                    debugprintf("watch: MSG read, rs=%d\n",len) ;
	                    debugprintf("watch: namelen=%d\n",
	                		ipcmsg.msg_namelen) ;

#if	CF_DEBUGFROM
				cl = sockaddress_getaddr(&ipcfrom,
					fromfname,MAXPATHLEN) ;

			    debugprintf("watch: fromlen=%d from=%t\n",
				cl,fromfname,strnlen(fromfname,cl)) ;

#endif /* CF_DEBUGFROM */

	                    debugprintf("watch: ") ;
	                    for (i = 0 ; i < 10 ; i += 1)
	                        debugprintf(" %02x",ipcbuf[i]) ;
	                    debugprintf("\n") ;
	                }

	}
#endif /* CF_DEBUG */

	                if (len > 0) {

	                    rcode = (uint) ipcbuf[0] ;

#if	CF_DEBUG
	                    if (pip->debuglevel >= 4)
	                        debugprintf("watch: MSG rcode=%d\n",
	                            rcode) ;
#endif

/* what do we do with this code ? */

	                    switch (rcode) {

	                    case imsgtype_noop:
				imsg_noop(ipcbuf,IPCBUFLEN,0,&m1) ;

				m0.tag = m1.tag ;
				m0.rc = 0 ;
				blen = imsg_response(ipcbuf,IPCBUFLEN,0,&m0) ;

				ipcmsg.msg_accrightslen = 0 ;
	                        vecs[0].iov_len = blen ;
	                        u_sendmsg(pip->fd_req,&ipcmsg,0) ;

	                        break ;

			    case imsgtype_passfd:
				imsg_passfd(ipcbuf,IPCBUFLEN,0,&m3) ;

				if (ipcmsg.msg_accrightslen > 0) {

				    memset(cip,0,sizeof(struct clientinfo)) ;

#if	CF_DEBUG
	                            if (pip->debuglevel >= 4)
	                                debugprintf("watch: PASSFD len=%d ns=%d\n",
	                                    ipcmsg.msg_accrightslen,ns) ;
#endif

	                            fromlen = sizeof(SOCKADDRESS) ;
	                            rs1 = u_getpeername(ns,&cip->sa,&fromlen) ;

	                            if (rs1 >= 0)
	                                cip->salen = fromlen ;

	                            rs = watch_newjob(pip,bip,&ourstand,cip,
					ns,ns) ;

	                            if (rs >= 0)
	                                njobs += 1 ;

	                            if (rs < 0) {

#ifdef	COMMENT
	                                cp = "could not allocate resource\n" ;
	                            	uc_writen(ns,cp,strlen(cp)) ;
#endif /* COMMENT */

	                                logfile_printf(&pip->lh,
	                                    "jobdb new failed, rs=%d\n",rs) ;

	                            } else
	                                pip->subserial += 1 ;

	                        } else {

				    rs = SR_NOENT ;
	                            logfile_printf(&pip->lh,
	                                "no FD was given to us, rs=%d\n",rs) ;

	                        } /* end if (nothing was there) */

				m0.tag = m3.tag ;
				m0.rc = (rs >= 0) ? 0 : 1 ;
				blen = imsg_response(ipcbuf,IPCBUFLEN,0,&m0) ;

				ipcmsg.msg_accrightslen = 0 ;
	                        vecs[0].iov_len = blen ;
	                        u_sendmsg(pip->fd_req,&ipcmsg,0) ;

				break ;

	                    case imsgtype_exit:
				imsg_exit(ipcbuf,IPCBUFLEN,0,&m2) ;

				m0.tag = m2.tag ;
				m0.rc = 1 ;
				blen = imsg_response(ipcbuf,IPCBUFLEN,0,&m0) ;

				ipcmsg.msg_accrightslen = 0 ;
	                        vecs[0].iov_len = blen ;
	                        u_sendmsg(pip->fd_req,&ipcmsg,0) ;

	                        f_exit = TRUE ;
	                        break ;

	                    default:

#if	CF_DEBUG
	                        if (pip->debuglevel >= 4)
	                            debugprintf("watch: MSG standing\n") ;
#endif

	                        rs = standing_request(&ourstand,pip->daytime,
					rcode, ipcbuf,resbuf) ;

	                        len = rs ;
	                        if (len > 0) {

#if	CF_DEBUGSLEEP
	                            sleep(3) ;
#endif

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
					debugprintf("watch: type=%d\n",
						(int) resbuf[0]) ;
#endif

				    ipcmsg.msg_accrightslen = 0 ;
	                            vecs[0].iov_base = resbuf ;
	                            vecs[0].iov_len = len ;

	                            rs = u_sendmsg(pip->fd_req,&ipcmsg,0) ;

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("watch: u_sendmsg() rs=%d\n",
	                                    rs) ;
#endif

	                        } /* end if */

	                    } /* end switch */

	                } /* end if (non-zero length) */

			if (ns >= 0)
				u_close(ns) ;

	            } /* end if (poll came back on input) */

	        } /* end if (our intra-program message portal) */


	    } /* end for */

	} /* end if (something from 'poll') */


/* are there any completed jobs yet ? */

	if ((njobs > 0) &&
	    ((rs = u_waitpid(-1,&child_stat,W_OPTIONS)) > 0)) {

	    int	ji ;


#if	CF_DEBUG
	    if (pip->debuglevel >= 4)
	        debugprintf("watch: child exit, pid=%d stat=%d\n",
	            rs,(child_stat & 0xFF)) ;
#endif

	    pid = rs ;
	    if ((ji = jobdb_findpid(&pip->jobs,pid,&jep)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("watch: found child, ji=%d\n",ji) ;
#endif

	        logfile_setid(&pip->lh,jep->jobid) ;

/* process this guy's termination */

	        if ((efd = u_open(jep->efname,O_RDONLY,0666)) >= 0) {

	            rs = SR_OK ;
	            logfile_printf(&pip->lh, "%s server exit, ex=%d\n",
	                timestr_logz(pip->daytime,timebuf),
	                child_stat & 255) ;

	            writeout(pip,efd,"standard error") ;

	            logfile_printf(&pip->lh,"elapsed time %s\n",
	                timestr_elapsed((pip->daytime - jep->atime),timebuf)) ;

	            u_close(efd) ;

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel >= 4)
	                debugprintf("watch: child did not 'exec'\n") ;
#endif

	            logfile_printf(&pip->lh,"server did not 'exec'\n") ;

	        }

	        jobdb_del(&pip->jobs,ji) ;

	        logfile_setid(&pip->lh,pip->logid) ;


	        if (! pip->f.daemon)
	            f_exit = TRUE ;

	    } else
	        logfile_printf(&pip->lh,"unknown PID=%d\n",rs) ;

	} /* end if (a child process exited) */


/* maintenance the lock file */

	if (pip->f.daemon) {

/* maintenance the LOCK mutex file */

	    if (pip->f.lockfile &&
	        ((pip->daytime - ti_lockcheck) >= (TI_MAINT - 1))) {

	        LFM_CHECK	ci ;


	        rs = lfm_check(&pip->lfile,&ci,pip->daytime) ;

	        if (rs <= 0) {

	            logfile_printf(&pip->lh,
	                "%s lock file conflict, other PID=%d\n",
	                timestr_logz(pip->daytime,timebuf),
	                ci.pid) ;

	            goto badlockfile ;
	        }

	        ti_lockcheck = pip->daytime ;

	    } /* end if (maintaining the lock file) */

/* maintenance the PID mutex file */

	    if ((pip->pidfp != NULL) && 
	        ((pip->daytime - ti_pidcheck) >= (TI_MAINT - 1))) {

	        rs = checklockfile(pip,pip->pidfp,pip->pidfname,
	            BANNER,pip->daytime,pip->pid) ;

	        if (rs != 0) {

	            logfile_printf(&pip->lh,
	                "%s PID file conflict, other PID=%d\n",
	                timestr_logz(pip->daytime,timebuf),
	                rs) ;

	            goto badpidfile ;
	        }

	        ti_pidcheck = pip->daytime ;

	    } /* end if (maintaining the PID mutex file) */

/* maintenance the log file */

	    if (pip->f.log)
		logfile_check(&pip->lh,pip->daytime) ;


	} /* end if (daemon mode) */


/* check up on the standing server object */

	rs = standing_check(&ourstand,pip->daytime) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: standing_check() rs=%d\n",rs) ;
#endif

	if (rs > 0) {

		to_pollcheck = rs / 5 ;
		if (to_pollcheck <= 0)
			to_pollcheck = 1 ;

		if (rs >= to_pollidle) {

			int	min ;


			min = MIN(rs,TI_POLL) ;
			if ((to_pollidle < min) && ((loopcount % 60) == 0))
				to_pollidle += 1 ;

		} else
			to_pollidle = to_pollcheck ;

	} /* end if (standing part minimum interval check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: to_pollidle=%d\n",to_pollidle) ;
#endif


/* check if the server table file (srvtab) has changed */

	if (pip->f.daemon) {

	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;

	    if (rs > 0) {

		f_logchange = TRUE ;
	        logfile_printf(&pip->lh,"%s server table file changed\n",
	            timestr_logz(pip->daytime,timebuf)) ;

	    }

	    if (pip->markint > 0) {

	        if ((pip->daytime - ti_lastmark) >= pip->markint) {

		    f_logchange = TRUE ;
	            logfile_printf(&pip->lh,"%s mark> %s\n",
	                timestr_logz(pip->daytime,timebuf),
	                pip->nodename) ;

		    logfile_flush(&pip->lh) ;

	            ti_lastmark = pip->daytime ;
	        }

	    } /* end if */

	} /* end if (daemon mode) */


/* check if the access table has changed, if we have one */

	if (pip->f.daemon && pip->f.acctab) {

	    rs = acctab_check(&pip->atab,NULL) ;

	    if (rs > 0) {

		f_logchange = TRUE ;
	        logfile_printf(&pip->lh,"%s access table file changed\n",
	            timestr_logz(pip->daytime,timebuf)) ;

	    }

	}

	if (f_logchange)
		logfile_flush(&pip->lh) ;


	njobs = jobdb_count(&pip->jobs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: jobs=%d\n",njobs) ;
#endif

/* should we exit due to idle timeout ? */

	if (pip->runint > 0) {

		if (((pip->daytime - ti_start) > pip->runint) && (njobs <= 0))
			f_exit = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

		if (f_exit)
		debugprintf("watch: exiting due to RUNINT=%d expiration\n",
			pip->runint);

	}
#endif /* CF_DEBUG */

	} /* end if (running on a run-interval) */


/* go back to the top of the loop */

	loopcount += 1 ;

	if (! f_exit)
	    goto top ;


#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: returning!\n") ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"0") ;
#endif

	if (pip->f.daemon) {

	    logfile_printf(&pip->lh,"%s daemon exiting\n",
	        timestr_logz(pip->daytime,timebuf)) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"2") ;
#endif

	rs = SR_OK ;


/* early and regular exits */
badlockfile:
badpidfile:

bad4:
	(void) standing_free(&ourstand) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"3") ;
#endif

bad3:

bad2:

bad1b:
	closeipc(pip,reqfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"4") ;
#endif

bad1a:
	jobdb_free(&pip->jobs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"5") ;
#endif

bad1:

/* do something ?? with the 'jobdname' ; like maybe remove it ? */

#ifdef	OPTIONAL
	u_rmdir(jobdname) ;
#endif

bad0:

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"6") ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad things */
badhup:

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: poll() hangup FD=%d\n",fds[nfd].fd) ;
#endif

	goto bad4 ;

baderr:

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: poll() error FD=%d\n",fds[nfd].fd) ;
#endif

	goto bad4 ;

}
/* end subroutine (watch) */



/* LOCAL SUBROUTINES */



static void int_exit(sn)
int	sn ;
{


	f_exit = TRUE ;
}
/* end subroutine (int_exit) */


static void int_child(sn)
int	sn ;
{


	f_child = TRUE ;
}


/* write out the output files from the executed program */
static int writeout(pip,fd,s)
struct proginfo	*pip ;
int	fd ;
char	s[] ;
{
	bfile		file, *fp = &file ;

	struct ustat	sb ;

	int		tlen, len ;

	char		linebuf[LINEBUFLEN + 1] ;


	tlen = 0 ;
	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    logfile_printf(&pip->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,MAXOUTLEN)) > 0) {

	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %W\n",
	                linebuf,MIN(len,MAXOUTLEN)) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */


#ifdef	COMMENT

/* write our MSGQ ID to the MSGQ ID file */
static int write_mqid(pip,mqid)
struct proginfo	*pip ;
int		mqid ;
{
	bfile	mqfile ;

	int	rs ;

	char	tmpfname[MAXPATHLEN + 2] ;


	mkpath2(tmpfname, pip->spooldname,MSGQFNAME) ;

	rs = bopen(&mqfile,tmpfname,"wct",0664) ;

	if (rs >= 0) {

	bprintf(&mqfile,"%d\n",mqid) ;

	bclose(&mqfile) ;

	}

	return rs ;
}
/* end subroutine (write_mqid) */

#endif /* COMMENT */


/* spawn a job */
static int watch_newjob(pip,bip,ourp,cip,nsi,nso)
struct proginfo		*pip ;
BUILTIN			*bip ;
STANDING		*ourp ;
struct clientinfo	*cip ;
int	nsi, nso ;
{
	JOBDB_ENT	*jep ;

	pid_t	pid ;

	int	rs, ji, i ;

	char	logid[JOBDB_JOBIDLEN + 1] ;


/* enter this job into the database */

#if	CF_SNDDD
	snddd(logid,JOBDB_JOBIDLEN,
	    pip->serial,pip->subserial) ;
#else
	bufprintf(logid,JOBDB_JOBIDLEN,"%d.%d",
	    pip->serial,pip->subserial) ;
#endif

	rs = jobdb_newjob(&pip->jobs,logid,0) ;

	if (rs < 0)
	    goto bad0 ;

	logfile_setid(&pip->lh,logid) ;

	ji = rs ;
	cip->mtype = (long) (ji + 2) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch_newjob: JID=%d mtype=%ld\n",
	        ji,cip->mtype) ;
#endif

	jobdb_get(&pip->jobs,ji,&jep) ;


/* let's fork the processing subroutine and get on with our lives! */

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch_newjob: about to fork, passing ns=%d\n",nsi) ;
#endif

	if (pip->f.log)
	    logfile_flush(&pip->lh) ;

	bflush(pip->efp) ;

/* do it */

	rs = uc_fork() ;
	pid = rs ;

	if (rs < 0) {
	    logfile_printf(&pip->lh,
	        "cannot fork (%d)\n",rs) ;
	    goto bad1 ;
	}

	if (pid == 0) {
	    int		fd ;
	    int		f_nsiok, f_nsook ;
	    char	timebuf[TIMEBUFLEN + 1] ;

/* we are now the CHILD!! */

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("child 0") ;
#endif

	    pip->logid = jep->jobid ;
	    cip->pid = getpid() ;

/* close stuff we don't need */

	    if (pip->f.daemon) {

		if (pip->fd_listenpass >= 0)
	        	u_close(pip->fd_listenpass) ;

		if (pip->fd_listentcp >= 0)
	        	u_close(pip->fd_listentcp) ;

	    }


/* move some FDs that we do need, if necessary */

	    f_nsiok = (nsi == FD_STDIN) ;

	    f_nsook = (nso == FD_STDOUT) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	    debugprintf("watch_newjob: 0 nsi=%d nso=%d\n",nsi,nso) ;
#endif

/* we must use 'dupup()' and NOT 'uc_moveup()'! */

	    if ((nsi < 3) && (! f_nsiok))
	        nsi = dupup(nsi,3) ;

	    if ((nso < 3) && (! f_nsook))
	        nso = dupup(nso,3) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	    debugprintf("watch_newjob: 1 nsi=%d nso=%d\n",nsi,nso) ;
#endif

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("after possible move ups") ;
#endif

/* setup the input and output for the program */

	    for (i = 0 ; i < 3 ; i += 1) {

	        int	f_keep ;


	        f_keep = ((i == nsi) && f_nsiok) ;
	        if (! f_keep)
	            f_keep = ((i == nso) && f_nsook) ;

	        if (! f_keep)
	            (void) u_close(i) ;

	    } /* end for */

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("after possible closes") ;
#endif

	    if (! f_nsiok) {

	        fd = u_dup(nsi) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("watch_newjob: nsi=%d duped to FD=%d \n",nsi,fd) ;
#endif

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("after first dup") ;
#endif

	    }

	    if (! f_nsook) {

	        fd = u_dup(nso) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("watch_newjob: nso=%d duped to FD=%d \n",nso,fd) ;
#endif

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("after second dup") ;
#endif

	    }

	    fd = u_open(jep->efname,O_WRONLY,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("watch_newjob: STDERR on FD=%d \n",fd) ;
#endif

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4))
	        d_whoopen("after first dup") ;
#endif

/* close extras that we do not need (there are some that we keep) */

	    if (! f_nsiok)
	        u_close(nsi) ;

	    if (! f_nsook)
	        u_close(nso) ;

	    cip->fd_input = 0 ;
	    cip->fd_output = 1 ;

#if	CF_DEBUG && CF_WHOOPEN
	    if (DEBUGLEVEL(4)) {
	        debugprintf("watch_newjob: calling handle nsi=%d nso=%d\n",
	            nsi,nso) ;
	        d_whoopen("about to call handle()") ;
	    }
#endif

/* do it */

	    cip->ctime = pip->daytime ;
	    logfile_printf(&pip->lh,"%s request pid=%d\n",
	        timestr_logz(cip->ctime,timebuf),(int) cip->pid) ;

	    rs = handle(pip,bip,ourp,cip) ;


	    if (rs < 0) {

	        (void) u_unlink(jep->efname) ;

	    }

	    logfile_close(&pip->lh) ;

	    uc_exit(EX_NOEXEC) ;

	} /* end if */

	jep->pid = pid ;


	return rs ;

/* bad things */
bad1:
	logfile_setid(&pip->lh,pip->logid) ;

	jobdb_del(&pip->jobs,ji) ;

bad0:
	return rs ;
}
/* end subroutine (watch_newjob) */


/* open up the MSG portal */
static int openipc(pip,fname)
struct proginfo	*pip ;
char		fname[] ;
{
	mode_t	operms ;

	int	rs ;
	int	bnlen, alen ;
	int	fd ;
	int	sumask ;
	int	sl, cl ;

	char	buf[MAXPATHLEN + 1] ;
	char	template[MAXPATHLEN + 1] ;
	char	*bn ;
	char	*sp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: reqfname=%s\n",pip->reqfname) ;
#endif

	if (pip->f.daemon && 
		(pip->reqfname[0] != '-') && (pip->reqfname[0] != '+')) {

	    rs = u_socket(PF_UNIX,SOCK_DGRAM,0) ;
	    fd = rs ;
	    if (rs < 0)
	        return rs ;

#ifdef	COMMENT
	if ((pip->reqfname[0] == '/') || (u_stat(pip->reqfname,&sb) >= 0))
		strwcpy(fname,pip->reqfname,(MAXPATHLEN - 1)) ;

	else
	    mkpath2(fname,pip->pr,pip->reqfname) ;
#endif /* COMMENT */

		cl = strwcpy(fname,pip->reqfname,(MAXPATHLEN - 1)) - fname ;

		if ((sl = sfdirname(fname,cl,&sp)) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: checkdirs() dname=%t\n",sp,sl) ;
#endif

			checkdirs(pip,sp,sl,0600) ;

		}

	    u_unlink(fname) ;

	    rs = sockaddress_start(&pip->sa,AF_UNIX,fname,0,0) ;
	    if (rs >= 0) {

	        alen = sockaddress_getlen(&pip->sa) ;

	        rs = u_bind(fd,(struct sockaddr *) &pip->sa,alen) ;
		if (rs < 0) {

	    		sockaddress_finish(&pip->sa) ;

	    		if (fname[0] != '\0')
	        		u_unlink(fname) ;

		}

	    }

	    if (rs < 0)
	        u_close(fd) ;

	} else {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: non-daemon mode\n") ;
#endif

		rs = mktmpdir(pip,buf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("watch: mktmpdir() rs=%d\n",rs) ;
#endif

		if (rs < 0)
			return rs ;

/* create our socket template */

	    mkpath2(template,buf,"ipcXXXXXXXXXXX") ;

/* create our socket there */

	    operms = (S_IFSOCK | 0600) ;
	    rs = opentmpfile(template, O_SRVFLAGS,operms,fname) ;
	    fd = rs ;
	    if (rs < 0)
	        return rs ;

	    rs = sockaddress_start(&pip->sa,AF_UNIX,fname,0,0) ;
	    if (rs >= 0)
	        alen = sockaddress_getlen(&pip->sa) ;

/* handle any necessary cleanup */

	    if (rs < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: could not init\n") ;
#endif

	        sockaddress_finish(&pip->sa) ;

	        if (fname[0] != '\0')
	            u_unlink(fname) ;

	        u_close(fd) ;

	    } /* end if (error) */

	} /* end if */

	if (rs >= 0)
		u_chmod(fname,0600) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

		struct ustat	sb1 ;

		u_stat(buf,&sb1) ;

		debugprintf("watch/openipc: mode=%09o\n",sb1.st_mode) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: rs=%d FD=%d fname=%s\n",
		rs,fd,fname) ;
#endif

/* we're out of here */

	pip->salen = alen ;
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openipc) */


static int closeipc(pip,fname)
struct proginfo	*pip ;
char		fname[] ;
{


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

		struct ustat	sb1 ;

		int	cl ;

		char	buf[MAXPATHLEN + 1] ;
		char	*cp ;

		cl = sfdirname(fname,-1,&cp) ;

		strwcpy(buf,cp,MIN(cl,(MAXPATHLEN - 1))) ;

		u_stat(buf,&sb1) ;

		debugprintf("watch/closeipc: 1 dir=%s mode=%06o\n",
			buf,sb1.st_mode) ;
	}
#endif

	u_close(pip->fd_req) ;

	if (! pip->f.daemon)
	    u_unlink(fname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

		struct ustat	sb1 ;

		int	cl ;

		char	buf[MAXPATHLEN + 1] ;
		char	*cp ;

		cl = sfdirname(fname,-1,&cp) ;

		strwcpy(buf,cp,MIN(cl,(MAXPATHLEN - 1))) ;

		u_stat(buf,&sb1) ;

		debugprintf("watch/closeipc: 2 dir=%s mode=%06o\n",
			buf,sb1.st_mode) ;
	}
#endif

	return SR_OK ;
}
/* end subroutine (closeipc) */


#if	CF_DEBUG

static int printsubs(pip,s)
struct proginfo	*pip ;
const char	s[] ;
{
	int	rs1, i ;


	if (DEBUGLEVEL(5)) {

		int	klen, vlen ;

		char	*kp, *vp ;


		debugprintf("watch: printsubs=%s\n",s) ;

		i = 0 ;
		while (TRUE) {

		rs1 = varsub_get(&pip->subs,i,&kp,&klen,&vp,&vlen) ;
		if (rs1 < 0)
			break ;

		debugprintf("watch: key=%t value=>%t<\n",
				kp,klen,vp,MIN(30,vlen)) ;

			i += 1 ;

		} /* end while */

	}

	return 0 ;
}

#endif /* CF_DEBUG */



