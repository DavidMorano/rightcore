/* uc_recve */

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

        Get some amount of data, and time it also so that we can abort if it
        times out.

	Synopsis:

	int uc_recve(fd,rbuf,rlen,mflags,timeout,opts)
	int		fd ;
	char		rbuf[] ;
	int		rlen ;
	int		mflags ;
	int		timeout ;
	int		opts ;

	Arguments:

	fd		file descriptor
	rbuf		user buffer to receive daa
	rlen		maximum amount of data the user wants
	mflags		option flags for MSG reception
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

#ifndef	POLLMULT
#define	POLLMULT	1000		/* poll() takes milliseconds ! */
#endif

#ifndef	POLLTIMEINT
#define	POLLTIMEINT	10		/* seconds */
#endif

#define	EBUFLEN		100

#define	POLLEVENTS	(POLLIN | POLLPRI) ;


/* external subroutines */


/* forward references */

#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* exported subroutines */


int uc_recve(fd,rbuf,rlen,mflags,timeout,opts)
int		fd ;
void		*rbuf ;
int		rlen ;
int		mflags ;
int		timeout ;
int		opts ;
{
	struct pollfd	fds[2] ;
	time_t		previous = time(NULL) ;
	time_t		current ;
	int		rs = SR_OK ;
	int		events = POLLEVENTS ;
	int		to ;
	int		pollint ;
	int		tlen = 0 ;
	int		f_eof = FALSE ;

#if	CF_DEBUGS
	char	ebuf[EBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_recve: ent rlen=%u to=%d opts=%08x\n",
		rlen,timeout,opts) ;
#endif

	if (rlen <= 0)
	    return SR_OK ;

	if (timeout < 0)
	    timeout = INT_MAX ;

	to = timeout ;

#ifdef	POLLRDNORM
	events |= POLLRDNORM ;
#endif
#ifdef	POLLRDBAND
	events |= POLLRDBAND ;
#endif

	pollint = MIN(timeout,POLLTIMEINT) ;

	memset(fds,0,sizeof(fds)) ;
	fds[0].fd = fd ;
	fds[0].events = events ;
	fds[1].fd = -1 ;
	fds[1].events = 0 ;

	while ((rs >= 0) && (to >= 0)) {

#if	CF_DEBUGS
	    debugprintf("uc_recve: u_poll() pollint=%d to=%d\n",
	        pollint,timeout) ;
#endif

	    if ((rs = u_poll(fds,1,(pollint * POLLMULT))) > 0) {

#if	CF_DEBUGS
	        debugprintf("uc_recve: events %s\n",
	            d_reventstr(fds[0].revents,ebuf,EBUFLEN)) ;

	        debugprintf("uc_recve: about to 'read'\n") ;
#endif

	        rs = u_recv(fd,rbuf,rlen,mflags) ;
	        tlen = rs ;

#if	CF_DEBUGS
	        debugprintf("uc_recve: u_read() rs=%d\n",
	            rs) ;
#endif

		f_eof = (tlen == 0) ;
	        break ;

	    } else if (rs == 0) {

		if (to > 0) {

	            current = time(NULL) ;

	            to -= (current - previous) ;
	            previous = current ;
	            if (to < POLLTIMEINT) {
	                pollint = to ;
		    }

		} else
	            break ;

	    } else {
		if (rs == SR_INTR) rs = SR_OK ;
	    }

	} /* end while */

	if ((rs >= 0) && (tlen == 0) && (to <= 0) && 
	    (! f_eof) && (rlen > 0)) {

	    if (opts & FM_AGAIN) {
		rs = SR_AGAIN ;
	    } else if (opts & FM_TIMED) {
		rs = SR_TIMEDOUT ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_recve: ret rs=%d tlen=%u\n",
	    rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uc_recve) */


#if	CF_DEBUGS
static char *d_reventstr(int revents,char *rbuf,int rlen)
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


