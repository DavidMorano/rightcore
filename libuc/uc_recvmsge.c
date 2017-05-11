/* uc_recvmsge */

/* interface component for UNIX® library-3c */
/* extended read */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */


/* revision history:

	= 1998-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get some amount of data and time it also so that we can abort if it
        times out.

	Synopsis:

	int uc_recvmsge(fd,msgp,flags,timeout,opts)
	int		fd ;
	struct msghdr	*msgp ;
	int		flags ;
	int		timeout ;
	int		opts ;

	Arguments:

	fd		file descriptor
	msgp		pointer to MSG structure
	flags		option flags for the reception of MSG
	timeout		time in seconds to wait
	opts		options

	Returns:

	>=0		amount of data returned
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<poll.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	POLLMULT	1000		/* poll() takes milliseconds! */
#define	TO_POLL		10
#define	EBUFLEN		100

#define	POLLEVENTS	(POLLIN | POLLPRI)


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* exported subroutines */


int uc_recvmsge(fd,msgp,flags,timeout,opts)
int		fd ;
struct msghdr	*msgp ;
int		flags ;
int		timeout ;
int		opts ;
{
	struct pollfd	fds[2] ;
	time_t		previous = time(NULL) ;
	time_t		current ;
	int		rs = SR_OK ;
	int		events = POLLEVENTS ;
	int		nfds ;
	int		to = timeout ;
	int		pollint ;
	int		len = 0 ;
	int		f_first = TRUE ;

#if	CF_DEBUGS
	char		ebuf[EBUFLEN + 1] ;
#endif

	if (msgp == NULL) return SR_FAULT ;

	if (timeout < 0)
	    timeout = INT_MAX ;

	pollint = TO_POLL ;
	if (timeout >= 0)
	    pollint = MIN(timeout,TO_POLL) ;

#if	defined(POLLRDNORM)
	events |= POLLRDNORM ;
#endif
#if	defined(POLLRDBAND)
	events |= POLLRDBAND ;
#endif

	nfds = 0 ;
	memset(fds,0,sizeof(fds)) ;
	fds[nfds].fd = fd ;
	fds[nfds].events = events ;
	nfds += 1 ;
	fds[nfds].fd = -1 ;

	while ((rs >= 0) && (f_first || (to > 0))) {
	    int	f_break = FALSE ;
	    f_first = FALSE ;

	    if ((rs = u_poll(fds,nfds,(pollint * POLLMULT))) > 0) {
		const int	re = fds[0].revents ;

		if ((re & POLLIN) || (re & POLLPRI)) {
	            rs = u_recvmsg(fd,msgp,flags) ;
	            len = rs ;
		    f_break = TRUE ;
		} else if (re & POLLNVAL) {
		    rs = SR_NOTOPEN ;
		} else if (re & POLLERR) {
		    rs = SR_POLLERR ;
		} else if (re & POLLHUP) {
		    msleep(1) ;
		} /* end if */

	    } else if (rs == 0) {

	        current = time(NULL) ;

	        to -= (current - previous) ;
	        previous = current ;
	        if (to < TO_POLL)
	            pollint = to ;

	        if (to <= 0)
	            rs = SR_TIMEDOUT ;

	    } else if (rs == SR_INTR)
		rs = SR_OK ;

	    if (f_break) break ;
	} /* end while */

	if ((rs >= 0) && (timeout >= 0) && (to <= 0))
	    rs = SR_TIMEDOUT ;

#if	CF_DEBUGS
	debugprintf("uc_recvmsge: ret rs=%d len=%d\n",
	    rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_recvmsge) */


#if	CF_DEBUGS
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


