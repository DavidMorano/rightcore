/* u_connect */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ISCONN	0		/* change the meaning of ISCONN? */
#define	CF_REVENT	0		/* use 'revent(3debug)' */


/* revision history:

	= 1998-02-14, David A­D­ Morano
	This subroutine was originally written.

	= 1998-03-26, David A­D­ Morano
        This subroutine was updated to handle the case where a file descriptor
        was in non-blocking mode during an initial 'connect()' operation, was
        then put into blocking mode, and then an additional 'connect()' was
        issued. Before, we returned something like SR_ALREADY rather than SR_OK.
        Now, we detect if we are now in blocking mode and we will wait for the
        connect to complete.

	= 2017-05-08, David A.D. Morano
	I do not think it is a good idea to ever turn ON the compile-time
	CF_ISCONN switch. That would return SR_OK instead of SR_ISCONN
	when the connection is complete and we were in a non-blocking mode.
	This would mean that clinets would need to poll for either SR_OK
	or SR_ISCONN if they wanted to know if the connection was already
	completed.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the friendly hack version of the more "standard"
	UNIX 'connect()' subroutine.  I use the term "standard" advisedly since
	the hot-shot hot-heads working on BSD UNIX keep chaning the semantics
	slightly on this call and then the System V people change their version
	(usually a while afterwards) to catch up to the new "standard."

	Somewhere along the line, someone suggested to change the error return
	SR_ISCONN to SR_OK when we get it and if we are currently in blocking
	mode.  I put the code in there but it is not turned on (see compile
	flag above 'CF_ISCONN').


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<time.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	CONNECTINFO	struct connectinfo
#define	CONNECTINFO_FL	struct connectinfo_flags

#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#define	TO_NOBUFS	(5 * 60)		/* seconds */
#define	TO_NOSR		(5 * 60)		/* seconds */
#define	TO_CONNECT	(5 * 60)		/* seconds */
#define	POLLTIMEOUT	(1 * 1000)		/* milliseconds */

#if	CF_DEBUGS
#define	BUFLEN		100
#endif


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct connectinfo_flags {
	unsigned int	checkblock:1 ;
	unsigned int	nonblock:1 ;
	unsigned int	ndelay:1 ;
	unsigned int	blocking:1 ;
} ;

struct connectinfo {
	struct sockaddr	*sap ;
	CONNECTINFO_FL	f ;
	int		sal ;
	int		fd ;
} ;


/* forward references */

static int	connectinfo_start(CONNECTINFO *,int,struct sockaddr *,int) ;
static int	connectinfo_finish(CONNECTINFO *) ;
static int	connectinfo_checkblock(CONNECTINFO *) ;
static int	connectinfo_proc(CONNECTINFO *) ;
static int	connectinfo_wait(CONNECTINFO *,int) ;


/* exported subroutines */


int u_connect(int s,const void *vsap,int sal)
{
	struct sockaddr *sap = (struct sockaddr *) vsap ;
	CONNECTINFO	ci ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("u_connect: ent FD=%d\n",s) ;
#endif

	if ((rs = connectinfo_start(&ci,s,sap,sal)) >= 0) {
	    {
	        rs = connectinfo_proc(&ci) ;
	    }
	    rs1 = connectinfo_finish(&ci) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (connectinfo) */

#if	CF_DEBUGS
	debugprintf("u_connect: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_connect) */


/* local subroutines */


static int connectinfo_start(CONNECTINFO *cip,int fd,SOCKADDR *sap,int sal)
{

	memset(cip,0,sizeof(CONNECTINFO)) ;
	cip->fd = fd ;
	cip->sap = sap ;
	cip->sal = sal ;
	return SR_OK ;
}
/* end subroutine (connectinfo_start) */


static int connectinfo_finish(CONNECTINFO *cip)
{
	if (cip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (connectinfo_finish) */


static int connectinfo_checkblock(CONNECTINFO *cip)
{
	int		rs = SR_OK ;

	if (! cip->f.checkblock) {
	    cip->f.checkblock = TRUE ;
	    if ((rs = u_fcntl(cip->fd,F_GETFL,0)) >= 0) {
	        if (rs & O_NONBLOCK) cip->f.nonblock = TRUE ;
	        if (rs & O_NDELAY) cip->f.ndelay = TRUE ;
		cip->f.blocking = ((! cip->f.nonblock) && (! cip->f.ndelay)) ;
#if	CF_DEBUGS
		debugprintf("u_connect/connectinfo_checkblock: nonblock=%u\n",
		cip->f.nonblock) ;
		debugprintf("u_connect/connectinfo_checkblock: ndelay=%u\n",
		cip->f.ndelay) ;
#endif
	    }
	} /* end if (block check) */

	return rs ;
}
/* end subroutine (connectinfo_checkblock) */


static int connectinfo_proc(CONNECTINFO *cip)
{
	const int	sal = cip->sal ;
	const int	s = cip->fd ;
	int		rs ;
	const void	*sap = cip->sap ;
	int		to_nobufs = TO_NOBUFS ;
	int		to_nosr = TO_NOSR ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = connect(s,sap,sal)) < 0) rs = (- errno) ;
	    if (rs < 0) {
#if	CF_DEBUGS
	        debugprintf("u_connect: err rs=%d\n",rs) ;
#endif
	        switch (rs) {
	        case SR_NOBUFS:
	            if (to_nobufs-- > 0) {
	                msleep(1000) ;
	            } else {
	                f_exit = TRUE ;
	            }
	            break ;
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
	                msleep(1000) ;
	            } else {
	                f_exit = TRUE ;
	            }
	            break ;
	        case SR_INTR:
#if	CF_DEBUGS
	            debugprintf("u_connect: INTR\n") ;
#endif
	            if ((rs = connectinfo_checkblock(cip)) >= 0) {
	                if (cip->f.blocking) {
	                    rs = connectinfo_wait(cip,TO_CONNECT) ;
	                } else {
			    rs = SR_INTR ; /* continue looping */
			}
	            } else {
			f_exit = TRUE ;
		    }
	            break ;
/* a previous connect attempt is still in progress */
	        case SR_ALREADY:
#if	CF_DEBUGS
	            debugprintf("u_connect: ALREADY\n") ;
#endif
	            if ((rs = connectinfo_checkblock(cip)) >= 0) {
	                if (cip->f.blocking) {
	                    rs = connectinfo_wait(cip,TO_CONNECT) ;
	                } else {
			    rs = SR_ALREADY ; /* propagate */
	            	    f_exit = TRUE ; /* end exit */
			}
		    } else {
			f_exit = TRUE ;
	            }
	            break ;
/* the socket is already connected */
	        case SR_ISCONN:
#if	CF_DEBUGS
	            debugprintf("u_connect: ISCONN\n") ;
#endif
#if	CF_ISCONN
	            if ((rs = connectinfo_checkblock(cip)) >= 0) {
			if (cip->f.blocking) {
			    rs = SR_OK ;
			} else {
			    rs = SR_ISCONN ; /* propagate */
			    f_exit = TRUE ; /* and exit */
			}
		    } else {
			f_exit = TRUE ;
		    }
#else
	            f_exit = TRUE ;
	            ;
#endif /* CF_ISCONN */
	            break ;
	        default:
	            f_exit = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if (error) */
	} 
	until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (connectinfo_proc) */


static int connectinfo_wait(CONNECTINFO *cip,int to)
{
	struct pollfd	fds[2] ;
	time_t		ti_now = 0 ;
	time_t		ti_start = time(NULL) ;
	time_t		ti_end ;
	int		rs = SR_OK ;
	int		f_done = FALSE ;
	int		nfds ;

#if	CF_DEBUGS && CF_REVENT
	char	hexbuf[HEXBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("u_connecte/connecinfo_wait: ent to=%d\n",to) ;
#endif

	ti_end = (ti_start + to) ;

	nfds = 0 ;
	fds[nfds].fd = cip->fd ;
	fds[nfds].events = POLLOUT ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;
	fds[nfds].fd = -1 ;

	while ((rs >= 0) && (ti_now < ti_end)) {

#if	CF_DEBUGS
	    debugprintf("u_connecte: poll\n") ;
#endif

	    if ((rs = u_poll(fds,nfds,POLLTIMEOUT)) > 0) {
	        const int	re = fds[0].revents ;

#if	CF_DEBUGS && CF_REVENT
	        debugprintf("u_connecte: back poll re=%s\n", 
	            d_reventstr(re,hexbuf,HEXBUFLEN)) ;
#endif

	        if (re & POLLOUT) {
		    f_done = TRUE ;
	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        }
	    } else if (rs == SR_INTR) {
	        rs = SR_OK ;
	    } /* end if */

	    if (f_done) break ;
	    ti_now = time(NULL) ;
	    if (rs < 0) break ;
	} /* end while (loop timeout) */

#if	CF_DEBUGS
	debugprintf("u_connect/connwait: out rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (ti_now >= ti_end)) {
	    rs = SR_TIMEDOUT ;
	}

#if	CF_DEBUGS
	debugprintf("u_connect/connwait: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (connectinfo_wait) */


