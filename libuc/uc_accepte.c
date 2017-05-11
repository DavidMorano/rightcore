/* uc_accepte */

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

        Accept a connection on a socket and time it also so that we can abort if
        it times out.

	Synopsis:

	int uc_accepte(fd,sap,sal,to)
	int		fd ;
	struct sockaddr	*sap ;
	int		*sal ;
	int		to ;

	Arguments:

	fd		file descriptor
	sap		address to buffer to receive the "from" address
	sal		pointer to the length of the "from" buffer
	to		time in seconds to wait

	Returns:

	<0		error
	>=0		socket of new connection


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	POLLINTMULT	1000		/* poll-time multiplier */
#define	EBUFLEN		100


/* external subroutines */


/* forward references */

#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* exported subroutines */


int uc_accepte(int fd,const void *sap,int *salp,int to)
{
	int		rs = SR_OK ;
	int		s = -1 ;

#if	CF_DEBUGS
	char	ebuf[EBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_accepte: fd=%d to=%d\n",
	    fd,to) ;
#endif

	if (to < 0)
	    to = INT_MAX ;

	if (to >= 0) {
	    POLLFD	fds[1] ;

	    fds[0].fd = fd ;
	    fds[0].events = POLLIN ;

	    while (rs >= 0) {
	        if ((rs = u_poll(fds,1,POLLINTMULT)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("uc_accepte: back from POLL w/ rs=%d\n",
	                rs) ;
#endif

	            if (rs > 0) {
	                const int	re = fds[0].revents ;

#if	CF_DEBUGS
	                debugprintf("uc_accepte: events %s\n",
	                    d_reventstr(fds[0].revents,ebuf,EBUFLEN)) ;

	                debugprintf("uc_accepte: u_accept()\n") ;
#endif

	                if (re & POLLIN) {
	                    rs = u_accept(fd,sap,salp) ;
	                    s = rs ;
	                } else if (re & POLLHUP) {
	                    rs = SR_HANGUP ;
	                } else if (re & POLLERR) {
	                    rs = SR_POLLERR ;
	                } else if (re & POLLNVAL) {
	                    rs = SR_NOTOPEN ;
	                }

	            } else
	                to -= 1 ;

	            if (to <= 0) break ;
	            if (s >= 0) break ;
	        } else if (rs == SR_INTR) rs = SR_OK ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("uc_accepte: rs=%d \n", rs) ;
#endif

	    if ((rs == 0) && (s < 0) && (to <= 0))
	        rs = SR_TIMEDOUT ;

	} else {
	    rs = u_accept(fd,sap,salp) ;
	    s = rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_accepte: ret rs=%d \n", rs) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (uc_accepte) */


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


