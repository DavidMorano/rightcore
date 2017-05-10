/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGSLEEP	0
#define	CF_DEBUGFROM	1
#define	CF_WHOOPEN	0		/* who is open */
#define	CF_SIGCHILD	1		/* catch SIGCHLD? */
#define	CF_SNDDD	1


/* revision history:

	= 91/09/01, David A­D­ Morano

	This subroutine was adopted from the DWD program.  I may not
	have changed all of the comments correctly though !


	= 03/06/23, David A­D­ Morano

	I updated this subroutine to just poll for machine status and
	write the Machine Status (MS) file.  This was a cheap excuse
	for not writing a whole new daemon program just to poll for
	machine status.  I hope this works out ! :-)


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
	lfp	lockfile manager pointer (might be NULL)

	Returns:

	OK	doesn't really matter in the current implementation
	<0	error


*****************************************************************************/


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
#include	<time.h>
#include	<dirent.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sfstr.h>
#include	<baops.h>
#include	<bfile.h>
#include	<lfm.h>
#include	<field.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<acctab.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"imsg.h"


/* local defines */

#define	IPCDIRMODE	0777
#define	W_OPTIONS	(WNOHANG)
#define	TO_MAINT	(3 * 60)	/* miscellaneous maintenance */
#define	TO_RECVMSG	3
#define	IPCBUFLEN	MSGBUFLEN
#define	MAXOUTLEN	62
#define	NIOVECS		1
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpdir(struct proginfo *,char *) ;
extern int	checkdirs(struct proginfo *,char *,int,int) ;
extern int	acceptpass(int,struct strrecvfd *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* external variables */

extern int	if_int ;
extern int	if_child ;


/* local structures */


/* forward references */

static int	openipc(struct proginfo *,char *) ;
static int	closeipc(struct proginfo *,char *) ;


/* local variables */


/* exported subroutines */


int watch(pip,lfp)
struct proginfo	*pip ;
LFM		*lfp ;
{
	struct pollfd		fds[4] ;

	struct ustat		sb ;

	struct msghdr	ipcmsg ;

	struct iovec	vecs[NIOVECS] ;

	SOCKADDRESS	ipcfrom ;

	sigset_t	signalmask ;

	time_t		ti_start ;
	time_t		ti_lockcheck = 1 ;
	time_t		ti_pidcheck = 1 ;
	time_t		ti_lastmark = 0 ;

	int	rs = SR_BAD, rs1, i, len, blen ;
	int	re, ns ;
	int	size, cl ;
	int	to_pollidle = pip->pollint ;
	int	to_pollcheck = pip->pollint ;
	int	to_poll ;
	int	loopcount = 0 ;
	int	nclients = 0 ;
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


	rs = SR_OK ;
	reqfname[0] = '\0' ;

	size = NIOVECS * sizeof(struct iovec) ;
	(void) memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;


	(void) memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = &ipcfrom ;
	ipcmsg.msg_namelen = sizeof(SOCKADDRESS) ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;

	    pip->daytime = time(NULL) ;

	    ti_lastmark = pip->daytime ;

	pip->fd_req = -1 ;

#ifdef	COMMENT
	if (pip->f.daemon) {

	    for (i = 0 ; i < 3 ; i += 1) {

	        if (u_fstat(i,&sb) < 0)
	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	    }

/* setup the socket for IPC */

	rs = openipc(pip,reqfname) ;

	pip->fd_req = rs ;

	} /* end if (daemon mode) */

	if (rs < 0) {

	    bprintf(pip->efp,"%s: could not create IPC portal (%d)\n",
	        pip->progname,rs) ;

	    goto ret0 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("watch: MSG FD=%d\n",pip->fd_req) ;
#endif

#endif /* COMMENT */

/* let's go ! */

	nfds = 0 ;
	if (pip->f.daemon) {

	fds[nfds].fd = pip->fd_req ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

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

	fds[nfds].fd = -1 ;

	ti_start = pip->daytime ;

/* top of loop */

	loopcount = 0 ;
	while (rs >= 0) {

	f_logchange = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(10))
	printsubs(pip,"loop") ;
#endif

/* do the poll */

	if (nclients <= 0)
		to_poll = to_pollidle * POLLINTMULT ;

	else
		to_poll = to_pollidle * POLLINTMULT / 2 ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: about to poll to=%d\n",to_poll) ;
#endif

	rs = u_poll(fds,nfds,to_poll) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: back from poll w/ rs=%d f_child=%d\n",
	        rs,f_child) ;
#endif

	pip->daytime = time(NULL) ;

#ifdef	COMMENT
	if (pip->f.daemon && (rs > 0)) {

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

	            } else if (re & POLLHUP) {


	            } else if (re & POLLERR) {


		    }

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

/* what do we do with this code? */

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
	                                nclients += 1 ;

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

	            } /* end if (poll came back on input) */

	        } /* end if (our intra-program message portal) */

	    } /* end for (looping through FDs) */

	} /* end if (something from 'poll') */
#endif /* COMMENT */

	if (pip->f.daemon) {

/* maintenance the LOCK mutex file */

	    if ((rs >= 0) && pip->open.lockfile &&
	        ((pip->daytime - ti_lockcheck) >= pip->lockint)) {

	        LFM_CHECK	ci ;


	        rs = lfm_check(&pip->lockfile,&ci,pip->daytime) ;

	        if (rs <= 0) {

	            logfile_printf(&pip->lh,
	                "%s lock file conflict, other PID=%u\n",
	                timestr_logz(pip->daytime,timebuf),
	                ci.pid) ;

	        }

	        ti_lockcheck = pip->daytime ;

	    } /* end if (maintaining the lock file) */

/* maintenance the PID mutex file */

	    if ((rs >= 0) && pip->open.pidfile &&
	        ((pip->daytime - ti_pidcheck) >= pip->pidint)) {

	        LFM_CHECK	ci ;


	        rs = lfm_check(&pip->pidfile,&ci,pip->daytime) ;

	        if (rs <= 0) {

	            logfile_printf(&pip->lh,
	                "%s PID file conflict, other PID=%u\n",
	                timestr_logz(pip->daytime,timebuf),
	                ci.pid) ;

	        }

	        ti_pidcheck = pip->daytime ;

	    } /* end if (maintaining the PID mutex file) */

		if ((rs >= 0) && pip->have.config) {

	    rs = progconfig_check(pip) ;

	    if (rs > 1) {

		    f_logchange = TRUE ;
	            logfile_printf(&pip->lh,"%s configuration file change\n",
	                timestr_logz(pip->daytime,timebuf)) ;

	    } /* end if */

		} /* end if (configuration file check) */

	    if (pip->open.logfile)
		logfile_check(&pip->lh,pip->daytime) ;

	} /* end if (daemon mode) */

/* log marking */

	    if ((rs >= 0) && pip->open.logfile &&
	        ((pip->daytime - ti_lastmark) >= pip->markint)) {

		    f_logchange = TRUE ;
	            logfile_printf(&pip->lh,"%s mark> %s\n",
	                timestr_logz(pip->daytime,timebuf),
	                pip->nodename) ;

	            ti_lastmark = pip->daytime ;

	    } /* end if */

	if (f_logchange && pip->open.logfile)
		logfile_flush(&pip->lh) ;

/* go back to the top of the loop */

	loopcount += 1 ;

	} /* end while */

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("watch: out-of-loop\n") ;
#endif

#ifdef	COMMENT
	if (pip->f.daemon) {

	closeipc(pip,reqfname) ;

	    logfile_printf(&pip->lh,"%s daemon exiting\n",
	        timestr_logz(pip->daytime,timebuf)) ;

	}
#endif /* COMMENT */

ret0:
	return (rs >= 0) ? loopcount : rs ;
}
/* end subroutine (watch) */



/* LOCAL SUBROUTINES */



#ifdef	COMMENT

/* open up the MSG portal */
static int openipc(pip,fname)
struct proginfo	*pip ;
char		fname[] ;
{
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

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: daemon mode\n") ;
#endif

	    rs = u_socket(PCF_UNIX,SOCK_DGRAM,0) ;
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

	    rs = sockaddress_start(&pip->sa,ACF_UNIX,fname,0,0) ;
	    if (rs >= 0) {

	        alen = sockaddress_getlen(&pip->sa) ;

	        rs = u_bind(fd,(struct sockaddr *) &pip->sa,alen) ;
		if (rs < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: could not bind\n") ;
#endif

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

	    rs = opentmpfile(template,
	        O_SRVFLAGS,(S_IFSOCK | 0600),fname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("watch/openipc: opentmpfile() rs=%d fname=%s\n",
		rs,fname) ;
#endif

	    fd = rs ;
	    if (rs < 0)
	        return rs ;

	    rs = sockaddress_start(&pip->sa,ACF_UNIX,fname,0,0) ;
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

#endif /* COMMENT */



