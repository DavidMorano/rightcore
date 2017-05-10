/* transfer */

/* transfer data */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_LOCALEOF	0		/* allow local EOF to exit */
#define	CF_SIGPIPE	1		/* ignore SIGPIPE */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/**********************************************************************

	This subroutine transfer data among several file descriptors.


***********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"transfer.h"



/* local defines */

#define	TO_READ		30
#define	TO_PING		20

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#undef	MSGBUFLEN
#define	MSGBUFLEN	2048

#define	DENOM		(1000 * 1000)
#define	NFDS		6

#ifndef	SHUT_RD
#define	SHUT_RD		0
#define	SHUT_WR		1
#define	SHUT_RDWR	2
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif



/* external subroutines */

extern int	isasocket(int) ;
extern int	inetping(const char *,int) ;


/* external variables */


/* local structures */


/* local variables */







int transfer(pip,hostname,rfd,r2fd,ifd,ofd,efd,mxu)
struct proginfo	*pip ;
const char	hostname[] ;
int	rfd, r2fd ;
int	ifd, ofd, efd ;
int	mxu ;
{
	struct sigaction	sigs, oldsigs ;

	struct pollfd	fds[NFDS] ;

	struct fpstat	fp[NFDS] ;

	struct ustat	sb ;

	sigset_t	signalmask ;

	time_t	t_pollsanity ;
	time_t	t_sanity ;

	int	rs, rs1, i, nfds, len, sanityfailures = 0 ;
	int	fdi = 0 ;
	int	loopcount = 0 ;
	int	stype, optlen ;
	int	pollint = (10 * POLLINTMULT) ;
	int	pollinput = (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI) ;
	int	polloutput = (POLLWRNORM | POLLWRBAND) ;
	int	c_already = 0 ;
	int	f_exit ;
	int	f_daytime = FALSE ;
	int	f_issock = FALSE ;
	int	f_dgram = FALSE ;
	int	f ;

	char	buf[BUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: rfd=%d r2fd=%d\n",rfd,r2fd) ;
#endif

	for (i = 0 ; i < 6 ; i += 1) {

	    fds[i].fd = -1 ;
	    fds[i].events = 0 ;
	    fds[i].revents = 0 ;
	    memset(fp + i,0,sizeof(struct fpstat)) ;

	}

/* ignore the SIGPIPE signal */

#if	CF_SIGPIPE

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = SIG_IGN ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGPIPE,&sigs,&oldsigs) ;

#endif /* CF_SIGPIPE */


/* continue */

	f_issock = isasocket(rfd) ;

	if (f_issock) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: isasocket() rs=%d\n",f_issock) ;
#endif

		optlen = sizeof(int) ;
		rs1 = u_getsockopt(rfd,SOL_SOCKET,SO_TYPE,&stype,&optlen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: rs=%d socket type=%d\n",rs1, stype) ;
#endif

		f_dgram = (stype == SOCK_DGRAM) ? 1 : 0 ;

	}


/* standard input */

	fds[0].fd = -1 ;
	if (! pip->f.ni) {

	    fds[0].fd = ifd ;
	    fds[0].events = (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI) ;

	} else
	    fp[0].eof = TRUE ;

/* standard output */

	fds[1].fd = -1 ;
	if ((rs = u_fstat(ofd,&sb)) >= 0) {

	    fds[1].fd = ofd ;
	    fds[1].events = (POLLWRNORM | POLLWRBAND) ;

	}

/* standard error */

	fds[2].fd = -1 ;
	if ((r2fd >= 0) && (u_fstat(efd,&sb) >= 0)) {

	    fds[2].fd = efd ;
	    fds[2].events = (POLLWRNORM | POLLWRBAND) ;

#ifdef	COMMENT /* no input on STDERR ! */
	    fds[2].events |= (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI) ;
#endif /* COMMENT */

	}


/* remote socket */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: remote socket ?\n") ;
#endif

	fds[3].fd = rfd ;
	fds[3].events = (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI) ;

	if (pip->f.ni) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: no-input mode\n") ;
#endif

	    if (f_issock) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: have socket so doing shutdown\n") ;
#endif

	        rs = u_shutdown(rfd,SHUT_WR) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: u_shutdown() rs=%d\n",rs) ;
#endif

	    } else
	        u_write(rfd,buf,0) ;

	} else
	    fds[3].events |= (POLLWRNORM | POLLWRBAND) ;

/* secondary connection */

	fdi = 4 ;
	if (r2fd >= 0) {

	    fds[fdi].fd = r2fd ;
	    fds[fdi].events = (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI) ;
	    fds[fdi].events |= (POLLWRNORM | POLLWRBAND) ;

	    fdi += 1 ;
	}


/* what about sanity checking */

	if (pip->f.sanity) {

	    t_pollsanity = 0 ;
	    t_sanity = 1 ;
	    sanityfailures = 0 ;

	}


/* do the copy data function */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: about to loop\n") ;
#endif

	f_exit = FALSE ;
	while (! f_exit) {

	    f_daytime = FALSE ;

	    rs = u_poll(fds,fdi,pollint) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
		struct timeval	tv ;
	        debugprintf("transfer: back from POLL w/ rs=%d\n",
	            rs) ;
		uc_gettimeofday(&tv,NULL) ;
	        debugprintf("transfer: %s.%ld\n",
			timestr_log(((time_t) tv.tv_sec),timebuf),
			(tv.tv_usec/1000)) ;
	}
#endif

	    if (rs < 0) {

	        if (rs == SR_AGAIN) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ EAGAIN\n") ;
#endif

	            sleep(1) ;

	            continue ;

	        } else if (rs == SR_INTR) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ EINTR\n") ;
#endif

	            continue ;

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: back from POLL w/ BADPOLL\n") ;
#endif

	            break ;
	        }

	    } /* end if (poll got an error) */

	    nfds = rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2) {

	        for (i = 0 ; i < NFDS ; i += 1) {

	            debugprintf("transfer: fds%d %s\n",i,
	                d_reventstr(fds[i].revents,buf,BUFLEN)) ;

	        }
	    }
#endif /* CF_DEBUG */

/* check the actual low level events */

	    if (fds[0].revents != 0) {

	        if (fds[0].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN0\n") ;
#endif

	            fp[0].in = TRUE ;
	            fds[0].events &= (~ pollinput) ;

	        }

	        if (fds[0].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: HUP0\n") ;
#endif

	            fp[0].hup = TRUE ;
	            fp[0].out = FALSE ;
	            fds[0].events = 0 ;
		    fds[0].fd = -1 ;

	        }

	    } /* end if */

	    if ((! fp[0].in) && fp[0].hup) {

			fp[0].in = TRUE ;
			fp[0].hup = FALSE ;
			fp[0].final = TRUE ;
	    }

	    if (fds[1].revents != 0) {

	        if (fds[1].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: OUT1\n") ;
#endif

	            fp[1].out = TRUE ;
	            fds[1].events &= (~ polloutput) ;

	        }

	        if (fds[1].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: HUP1\n") ;
#endif

	            fp[1].hup = TRUE ;
	            fp[1].out = FALSE ;
	            fds[1].events &= (~ polloutput) ;

	        }

	    } /* end if */

	    if ((! fp[1].in) && fp[1].hup) {

			fp[1].in = TRUE ;
			fp[1].hup = FALSE ;
			fp[1].final = TRUE ;
	    }

	    if (fds[2].revents != 0) {

	        if (fds[2].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN2\n") ;
#endif

	            fp[2].in = TRUE ;
	            fds[2].events &= (~ pollinput) ;

	        }

	        if (fds[2].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: OUT2\n") ;
#endif

	            fp[2].out = TRUE ;
	            fds[2].events &= (~ polloutput) ;

	        }

	        if (fds[2].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: HUP2\n") ;
#endif

	            fp[2].hup = TRUE ;
	            fp[2].out = FALSE ;
	            fds[2].events &= (~ polloutput) ;

	        }

	    } /* end if */

	    if ((! fp[2].in) && fp[2].hup) {

			fp[2].in = TRUE ;
			fp[2].hup = FALSE ;
			fp[2].final = TRUE ;
	    }

/* the remote promary connection */

	    if (fds[3].revents != 0) {

	        if (fds[3].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN3\n") ;
#endif

	            fp[3].in = TRUE ;
	            fds[3].events &= (~ pollinput) ;

	        }

	        if (fds[3].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: OUT3\n") ;
#endif

	            fp[3].out = TRUE ;
	            fds[3].events &= (~ polloutput) ;

	        }

	        if (fds[3].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: HUP3\n") ;
#endif

	            fp[3].hup = TRUE ;
	            fp[3].out = FALSE ;
	            fds[3].events &= (~ polloutput) ;

	        }

	        if ((fds[3].revents & POLLERR) ||
	            (fds[3].revents & POLLNVAL)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {

	                if (fds[3].revents & POLLERR)
	                    debugprintf("transfer: ERR3\n") ;

	                if (fds[3].revents & POLLNVAL)
	                    debugprintf("transfer: NVAL3\n") ;

	            }
#endif /* CF_DEBUG */

	            fp[3].in = FALSE ;
	            fp[3].out = FALSE ;
	            fp[3].final = TRUE ;
	            fds[3].fd = -1 ;
	            fds[3].events = 0 ;

	        }

	    } /* end if */

	    if ((! fp[3].in) && fp[3].hup) {

			fp[3].in = TRUE ;
			fp[3].hup = FALSE ;
			fp[3].final = TRUE ;
	    }

/* the secondard connection */

	    if (fdi > 4) {

	        if (fds[4].revents != 0) {

	        if (fds[4].revents & pollinput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN4\n") ;
#endif

	            fp[4].in = TRUE ;
	            fds[4].events &= (~ pollinput) ;

	        }

	        if (fds[4].revents & polloutput) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: OUT4\n") ;
#endif

	            fp[4].out = TRUE ;
	            fds[4].events &= (~ polloutput) ;

	        }

	        if (fds[4].revents & POLLHUP) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: HUP4\n") ;
#endif

	            fp[4].hup = TRUE ;
	            fp[4].out = FALSE ;
	            fds[4].events &= (~ polloutput) ;

	        }

	        if ((fds[4].revents & POLLERR) ||
	            (fds[4].revents & POLLNVAL)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {

	                if (fds[4].revents & POLLERR)
	                    debugprintf("transfer: ERR4\n") ;

	                if (fds[4].revents & POLLNVAL)
	                    debugprintf("transfer: NVAL4\n") ;

	            }
#endif /* CF_DEBUG */

	            fp[4].in = FALSE ;
	            fp[4].out = FALSE ;
	            fp[4].final = TRUE ;
	            fds[4].fd = -1 ;
	            fds[4].events = 0 ;

	        }

		}

	    if ((! fp[4].in) && fp[4].hup) {

			fp[4].in = TRUE ;
			fp[4].hup = FALSE ;
			fp[4].final = TRUE ;
	    }

	    } /* end if (secondary connection) */


/* now we are ready to check for the events that we really want */

/* output from remote connection to our standard output */

	    if (fp[3].in && fp[1].out) {

	        len = u_read(rfd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("transfer: IN3 -> OUT1 len=%d\n",
	                len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN3 EOF\n") ;
#endif

	            fp[3].eof = TRUE ;
	            fp[3].in = FALSE ;
	            fds[3].events &= (~ pollinput) ;

	        } else
	            uc_writen(fds[1].fd,buf,len) ;

	        if ((! fp[3].eof) && (! (fp[3].hup || fp[3].final))) {

	            fp[3].in = FALSE ;
	            fds[3].events |= pollinput ;

	        } else if (fds[3].events == 0)
	            fds[3].fd = -1 ;

	        fp[1].out = FALSE ;
	        fds[1].events |= polloutput ;

	    } /* end if (remote connection to standard output) */

/* input from our standard input out to the remote connection */

	    if (fp[0].in && fp[3].out) {

	        len = u_read(ifd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("transfer: IN0 -> OUT3 len=%d\n",len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN0 EOF\n") ;
#endif

	            fp[0].eof = TRUE ;
	            fp[0].in = FALSE ;
	            fds[0].fd = -1 ;

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {
	                rs = u_fstat(rfd,&sb) ;

	                debugprintf("transfer: RFD stat rs=%d mode=%08X\n",
	                    rs,sb.st_mode) ;
	            }
#endif /* CF_DEBUG */

	            if (f_issock) {

			if (f_dgram) {

	                rs1 = u_send(rfd,buf,0,0) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {
			debugprintf("transfer: u_send() rs=%d\n",rs1) ;
		}
#endif

			}

	                u_shutdown(rfd,SHUT_WR) ;

	            } else
	                u_write(rfd,buf,0) ;

	        } else
	            uc_writen(fds[3].fd,buf,len) ;

	        if ((! fp[0].eof) && (! (fp[0].hup || fp[0].final))) {

	            fp[0].in = FALSE ;
	            fds[0].events |= pollinput ;

	        }

	        fp[3].out = FALSE ;
	        fds[3].events |= polloutput ;

	    } /* end if (standard input to primary socket) */

/* standard error to secondary socket */

	    if ((r2fd >= 0) && (fp[2].in && fp[4].out)) {

	        len = u_read(efd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("transfer: IN2 -> OUT4 len=%d\n",len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN2 EOF\n") ;
#endif

	            fp[2].eof = TRUE ;
	            fp[2].in = FALSE ;
	            fds[2].events &= (~ pollinput) ;

	            if (f_issock)
	                u_shutdown(r2fd,SHUT_WR) ;

	            else
	                u_write(r2fd,buf,0) ;

	        } else
	            uc_writen(fds[4].fd,buf,len) ;

	        if ((! fp[2].eof) && (! (fp[2].hup || fp[2].final))) {

	            fp[2].in = FALSE ;
	            fds[2].events |= pollinput ;

	        } else if (fds[2].events == 0)
	            fds[2].fd = -1 ;

	        fp[4].out = FALSE ;
	        fds[4].events |= polloutput ;

	    } /* end if (standard error to secondary connection) */

/* secondary socket to standard error */

	    if ((r2fd >= 0) && (fp[4].in && fp[2].out)) {

	        len = u_read(r2fd,buf,mxu) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("transfer: IN4 -> OUT2 len=%d\n",
	                len) ;
#endif

	        if (len <= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2)
	                debugprintf("transfer: IN4 EOF\n") ;
#endif

	            fp[4].eof = TRUE ;
	            fp[4].in = FALSE ;
	            fds[4].events &= (~ pollinput) ;

	        } else
	            uc_writen(fds[2].fd,buf,len) ;

	        if ((! fp[4].eof) && (! (fp[4].hup || fp[4].final))) {

	            fp[4].in = FALSE ;
	            fds[4].events |= pollinput ;

	        } else if (fds[4].events == 0)
	            fds[4].fd = -1 ;

	        fp[2].out = FALSE ;
	        fds[2].events |= polloutput ;

	    } /* end if (secondary connection to standard error) */

/* should we break out ? */

	    if ((c_already > 0) && (nfds == 0)) {

	        if (c_already > 1)
	            break ;

	        c_already += 1 ;
	    }

#if	CF_LOCALEOF
	    f = fp[0].eof || fp[3].eof ;
#else
	    f = fp[3].eof ;
#endif

	    if (f && (c_already == 0)) {

	        pollint = POLLINTMULT / 4 ;
	        c_already = 1 ;
	    }

/* miscellaneous functions */

	    if (pip->f.sanity) {

		if (! f_daytime) {
		f_daytime = TRUE ;
	        pip->daytime = time(NULL) ;
		}

	        if ((pip->daytime - t_pollsanity) > 
	            (pip->keeptime / SANITYFAILURES)) {

	            t_pollsanity = pip->daytime ;
	            if (inetping(hostname,TO_PING) >= 0) {

	                sanityfailures = 0 ;
	                t_sanity = pip->daytime ;

	            } else
	                sanityfailures += 1 ;

	            if (((pip->daytime - t_sanity) > pip->keeptime) &&
	                (sanityfailures >= SANITYFAILURES) &&
	                ((rs = inetping(hostname,TO_PING)) < 0))
	                break ;

	        } /* end if (sanity poll) */

	    } /* end if (sanity check) */

	    if (pip->open.log && ((loopcount % 3) == 0)) {

		if (! f_daytime) {
		f_daytime = TRUE ;
	        pip->daytime = time(NULL) ;
		}

		logfile_check(&pip->lh,pip->daytime) ;

	    }

	    loopcount += 1 ;

	} /* end while (transferring data) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("transfer: ret rs=%d\n",rs) ;
#endif

ret0:
	return rs ;
}
/* end subroutine (transfer) */



