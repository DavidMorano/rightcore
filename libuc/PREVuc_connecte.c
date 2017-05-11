/* uc_connecte */

/* UNIX® XNET subroutine */
/* connection a socket */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */
#define	CF_BADSOLARIS	1		/* Solaris is bad? */
#define	CF_REVENT	1		/* show receive events */


/* revision history:

	= 1998-03-26, David A­D­ Morano

	This was first written to give a little bit to UNIX® what we have in
	our own circuit pack OSes!  The UNIX® system is such crap for real-time
	oriented applications!  Just for completeness: Oryx Pecos isn't the
	answer either!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we connect a socket but we do it in a way that we can time it
	out.  It should have been like this from the beginning!

	Synopsis:

	int uc_connecte(fd,sap,sal,to)
	int		fd ;
	struct sockaddr	*sap ;
	int		sal ;
	int		to ;

	Arguments:

	fd		file descriptor
	sap		socket address pointer
	sal		socket address length
	to		time in seconds to wait

	Returns:

	>=0		amount of data returned
	<0		error


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	POLLINTMULT	1000		/* poll-time multiplier */
#define	POLLTIMEOUT	200		/* milliseconds */

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	100
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local structures */


/* forward references */

static int	connwait(int,struct sockaddr *,int,int) ;

#if	CF_DEBUGS
static int	debugprintsa(const char *,void *) ;
#endif
#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* exported subroutines */


int uc_connecte(int fd,struct sockaddr *sap,int sal,int to)
{
	int		rs = SR_OK ;
	int		f_nonblock = FALSE ;

#if	CF_DEBUGS && CF_REVENT
	char		hexbuf[HEXBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_connecte: fd=%d sal=%d to=%d\n",fd,sal,to) ;
#endif

/* set non-blocking mode on this socket */

	if (to >= 0) {

	    rs = uc_nonblock(fd,TRUE) ;
	    f_nonblock = (rs > 0) ;
	    if (rs < 0)
	        goto bad0 ;

#if	CF_DEBUGS
	    debugprintf("uc_connecte: previous f_nonblock=%u\n",f_nonblock) ;
#endif

	} /* end if (a timeout value was specified) */

/* attempt to connect to the host */

#if	CF_DEBUGS
	debugprintf("uc_connecte: about to attempt a connect\n") ;
	{
	    mkhexstr(hexbuf,HEXBUFLEN,sap,8) ;
	    debugprintf("uc_connecte: sa=%s\n",hexbuf) ;
	    debugprintsa("uc_connecte",sap) ;
	}
#endif /* CF_DEBUGS */

	rs = u_connect(fd,sap,sal) ;

#if	CF_DEBUGS
	debugprintf("uc_connecte: u_connect() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (rs != SR_INPROGRESS)) {

#if	CF_DEBUGS
	    debugprintf("uc_connecte: u_connect() not_inprogress\n") ;
#endif

	    goto bad1 ;
	}

/* if in timeout-mode: wait for the socket to connect */

	if (to >= 0) {

	    if (rs == SR_INPROGRESS)
	        rs = connwait(fd,sap,sal,to) ;

	    if (! f_nonblock)
	        uc_nonblock(fd,FALSE) ;

	} /* end if (timeout-mode) */

ret0:
	return rs ;

/* bad stuff */
bad1:
	if ((to >= 0) && (! f_nonblock)) {
	    uc_nonblock(fd,FALSE) ;
	}

bad0:
	goto ret0 ;

}
/* end subroutine (uc_connecte) */


/* local subroutines */


static int connwait(fd,sap,sal,to)
int		fd ;
struct sockaddr	*sap ;
int		sal ;
int		to ;
{
	struct pollfd	fds[2] ;
	time_t		ti_now = 0 ;
	time_t		ti_start = time(NULL) ;
	time_t		ti_end ;
	int		rs = SR_OK ;
	int		nfds ;

#if	CF_DEBUGS && CF_REVENT
	char		hexbuf[HEXBUFLEN + 1] ;
#endif

	ti_end = ti_start + to ;

	nfds = 0 ;
	fds[nfds].fd = fd ;
	fds[nfds].events = POLLOUT ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	while ((rs >= 0) && (ti_now < ti_end)) {

#if	CF_DEBUGS
	    debugprintf("uc_connecte: about to poll\n") ;
#endif

	    rs = u_poll(fds,nfds,POLLTIMEOUT) ;

#if	CF_DEBUGS
	    debugprintf("uc_connecte: u_poll() rs=%d\n",rs) ;
#endif

	    if (rs > 0) {
	        int	re = fds[0].revents ;

#if	CF_DEBUGS && CF_REVENT
	        debugprintf("uc_connecte: back poll re=%s\n", 
	            d_reventstr(re,hexbuf,HEXBUFLEN)) ;
#endif

	        if (re & POLLOUT) {

#if	CF_DEBUGS
	            debugprintf("uc_connecte: writable?\n") ;
#endif

#if	CF_BADSOLARIS && LOCAL_SOLARIS
	            rs = u_connect(fd,sap,sal) ;

#if	CF_DEBUGS
	            debugprintf("uc_connecte: connect2 rs=%d\n",rs) ;
#endif

/* is it connected successfully yet? */

	            if (rs == SR_ISCONN)
	                rs = SR_OK ;

	            if (rs >= 0)
	                break ;

/* is it still in progress? */

	            if (rs == SR_ALREADY)
	                rs = SR_OK ;

#else /* CF_BADSOLARIS */
	            break ;
#endif /* CF_BADSOLARIS */

	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL)
	            rs = SR_NOTOPEN ;

	    } /* end if */

	    if (rs == SR_INTR) rs = SR_OK ;

	    ti_now = time(NULL) ;
	    if (rs < 0) break ;
	} /* end while (loop timeout) */

#if	CF_DEBUGS
	debugprintf("uc_connect/connwait: out rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (ti_now >= ti_end))
	    rs = SR_TIMEDOUT ;

#if	CF_DEBUGS
	debugprintf("uc_connect/connwait: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (connwait) */


#if	CF_DEBUGS && CF_REVENT
static char *d_reventstr(revents,buf,buflen)
int	revents ;
char	buf[] ;
int	buflen ;
{
	buf[0] = '\0' ;
	bufprintf(buf,buflen,"%s %s %s %s %s %s %s %s %s",
	    (revents & POLLIN) ? "I " : "  ",
	    (revents & POLLRDNORM) ? "IN" : "  ",
	    (revents & POLLRDBAND) ? "IB" : "  ",
	    (revents & POLLPRI) ? "PR" : "  ",
	    (revents & POLLWRNORM) ? "WN" : "  ",
	    (revents & POLLWRBAND) ? "WB" : "  ",
	    (revents & POLLERR) ? "ER" : "  ",
	    (revents & POLLHUP) ? "HU" : "  ",
	    (revents & POLLNVAL) ? "NV" : "  ") ;
	return buf ;
}
/* end subroutine (d_reventstr) */
#endif /* CF_DEBUGS */


#if	CF_DEBUGS
static int debugprintsa(fn,vp)
const char	fn[] ;
void		*vp ;
{
	int	i = 0 ;
	int	af ;
	short	*swp ;
	char	hexbuf[HEXBUFLEN + 1] ;
	char	*sp = (char *) vp ;
	char	*cp ;
	mkhexstr(hexbuf,HEXBUFLEN,sp,4) ;
	debugprintf("%s: sa0= %s\n",fn,hexbuf) ;
	swp = (short *) vp ;
	af = ntohs(*swp) ;
	switch (af) {
	case AF_UNSPEC:
	    break ;
	case AF_UNIX:
	    cp = (sp + 2) ;
	    debugprintf("%s: sa1= %s\n",fn,cp) ;
	    break ;
	case AF_INET:
	    break ;
	case AF_INET6:
	    for (i = 1 ; i < 8 ; i += 1) {
	        cp = (sp + (i * 4)) ;
	        mkhexstr(hexbuf,HEXBUFLEN,cp,4) ;
	        debugprintf("%s: sa%u= %s\n",fn,i,hexbuf) ;
	    }
	    break ;
	} /* end switch */
	return 0 ;
}
/* end subroutine (debugprintsa) */
#endif /* CF_DEBUGS */


