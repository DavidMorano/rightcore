/* uc_waitwritable */

/* interface component for UNIX® library-3c */
/* wait for an FD to become writable */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1999-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Wait for an FD to become writable.

	Synopsis:

	int uc_waitwritable(fd,to)
	int		fd ;
	int		to ;

	Arguments:

	fd		FD to wait on
	to 		time-out in seconds to wait for

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/poll.h>
#include	<sys/socket.h>
#include	<limits.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(8 * 1024)
#endif

#define	INTPOLL		10		/* seconds */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_waitwritable(int fd,int timeout)
{
	struct pollfd	polls[2] ;
	time_t		dt = 0 ;
	time_t		ti_timeout = 0 ;
	int		rs = SR_OK ;
	int		pollto = (INTPOLL*POLLMULT) ;
	int		nfds = 0 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("dialpass/waitready: timeout=%d\n",timeout) ;
	debugprintf("dialpass/waitready: fd=%d\n",fd) ;
#endif

	if (timeout >= 0) {
	    dt = time(NULL) ;
	    ti_timeout = (dt + timeout) ;
	    pollto = MIN(timeout,5) * POLLMULT ;
	}

	polls[nfds].fd = fd ;
	polls[nfds].events = (POLLOUT | POLLWRBAND) ;
	polls[nfds].revents = 0 ;
	nfds += 1 ;

/* CONSTCOND */

	while (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("dialpass/waitready: pollto=%d\n",pollto) ;
#endif

	    if ((rs = u_poll(polls,nfds,pollto)) > 0) {
		const int	re = polls[0].revents ;

#if	CF_DEBUGS
	        {
	            char	buf[BUFLEN + 1] ;
	            debugprintf("dialpass/waitready: revents=%s\n",
	                d_reventstr(re,buf,BUFLEN)) ;
	        }
#endif /* CF_DEBUGS */

	        if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_BADF ;
	        } else if ((re & POLLOUT) || (re & POLLWRBAND)) {
		    f = TRUE ;
		}

	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }

	    if ((rs >= 0) && (! f) && (timeout >= 0)) {
	            dt = time(NULL) ;
		    if (dt >= ti_timeout) rs = SR_TIMEDOUT ;
	    }

	    if ((rs >= 0) && f) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("dialpass/waitready: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uc_waitwritable) */


