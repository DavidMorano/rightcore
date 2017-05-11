/* uc_recvfrome */

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

	int uc_recvfrome(fd,rbuf,rlen,flags,fromp,fromlenp,timeout,opts)
	int		fd ;
	void		*rbuf ;
	int		rlen ;
	int		flags ;
	struct sockaddr	*fromp ;
	int		*fromlenp ;
	int		timeout ;
	int		opts ;

	Arguments:

	fd		file descriptor
	rbuf		user buffer to receive daa
	rlen		maximum amount of data the user wants
	flags		option flags for MSG reception
	fromp		pointer to socket address structure
	fromlenp	pointer to length of socket address
	timeout		time in seconds to wait
	opts		time in seconds to wait

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

#define	POLLMULT	1000		/* poll() takes milliseconds ! */
#define	TI_POLL		10
#define	EBUFLEN		100

#define	POLLEVENTS	(POLLIN | POLLPRI)


/* external subroutines */


/* forward references */

#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* exported subroutines */


int uc_recvfrome(fd,rbuf,rlen,flags,fromvp,fromlenp,timeout,opts)
int		fd ;
void		*rbuf ;
int		rlen ;
void		*fromvp ;
int		*fromlenp ;
int		timeout ;
int		opts ;
{
	struct pollfd	fds[2] ;

	struct sockaddr	*fromp = (struct sockaddr *) fromvp ;

	time_t	previous = time(NULL) ;
	time_t	current ;

	int	rs = SR_OK ;
	int	events = POLLEVENTS ;
	int	pollint ;
	int	len = 0 ;
	int	f_first ;

#if	CF_DEBUGS
	char	ebuf[EBUFLEN + 1] ;
#endif


#if	CF_DEBUGS
	    debugprintf("uc_recvfrome: rlen=%d\n",rlen) ;
	    debugprintf("uc_recvfrome: flags=%04x\n",flags) ;
	    debugprintf("uc_recvfrome: timeout=%d\n",timeout) ;
#endif

	if (rlen <= 0) return SR_OK ;

	if (rbuf == NULL) return SR_FAULT ;

	if (timeout < 0) timeout = INT_MAX ;

	pollint = (timeout > TI_POLL) ? TI_POLL : timeout ;

#ifdef	POLLRDNORM
	events |= POLLRDNORM ;
#endif
#ifdef	POLLRDBAND
	events |= POLLRDBAND ;
#endif

	memset(fds,0,sizeof(fds)) ;
	fds[0].fd = fd ;
	fds[0].events = events ;
	fds[1].fd = -1 ;

	f_first = TRUE ;
	while (f_first || (timeout > 0)) {

	    f_first = FALSE ;
	    rs = u_poll(fds,1,(pollint * POLLMULT)) ;
	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("uc_recvfrome: back from POLL w/ rs=%d\n",
	        rs) ;
#endif

	    if (rs > 0) {

#if	CF_DEBUGS
	        debugprintf("uc_recvfrome: events %s\n",
	            d_reventstr(fds[0].revents,ebuf,EBUFLEN)) ;
	        debugprintf("uc_recvfrome: about to 'read'\n") ;
#endif

	        rs = u_recvfrom(fd,rbuf,rlen,flags,fromp,fromlenp) ;
	        len = rs ;

#if	CF_DEBUGS
	        debugprintf("uc_recvfrome: u_recvfrom() rs=%d\n",
	            rs) ;
#endif

	        break ;

	    } else {

	        current = time(NULL) ;

	        timeout -= (current - previous) ;
	        previous = current ;
	        if (timeout < TI_POLL)
	            pollint = timeout ;

	        if (timeout <= 0)
	            rs = SR_TIMEDOUT ;

	    } /* end if */

	} /* end while */

#if	CF_DEBUGS
	debugprintf("uc_recvfrome: ret rs=%d len=%d\n",
	    rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_recvfrome) */


/* local subroutines */


#if	CF_DEBUGS
static char *d_reventstr(revents,rbuf,rlen)
int	revents ;
char	rbuf[] ;
int	rlen ;
{
	rbuf[0] = '\0' ;
	bufprintf(rbuf,rlen,"%s %s %s %s %s %s %s %s %s",
	    (revents & POLLIN) ? "I " : "  ",
	    (revents & POLLRDNORM) ? "IN" : "  ",
	    (revents & POLLRDBAND) ? "IB" : "  ",
	    (revents & POLLPRI) ? "PR" : "  ",
	    (revents & POLLWRNORM) ? "WN" : "  ",
	    (revents & POLLWRBAND) ? "WB" : "  ",
	    (revents & POLLERR) ? "ER" : "  ",
	    (revents & POLLHUP) ? "HU" : "  ",
	    (revents & POLLNVAL) ? "NV" : "  ") ;
	return rbuf ;
}
/* end subroutine (d_reventstr) */
#endif /* CF_DEBUGS */


