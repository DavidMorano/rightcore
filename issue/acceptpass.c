/* acceptpass */

/* subroutine to accept an FD that was passed through a FIFO */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little subroutine accepts a file descriptor from some type of FIFO,
        pipe, or other (weirdo) STREAMS-like thing. This subroutine will not
        compile on a non-STREAMS system; we depend on passing FDs through pipes
        and not eveyone (every OS) has that ability.

	Synopsis:

	int acceptpass(fd_pass,sp,to)
	int		fd_pass ;
	STRRECVFD	*sp ;
	int		to ;

	Arguments:

	fd_pass		FIFO file descriptor
	sp		pointer to structure to receive the passed FD
	to		time-out in seconds

	Returns:

	>=0		reeived file descriptot
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef STRRECVFD
#define	STRRECVFD	struct strrecvfd
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif


/* external subroutines */


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int acceptpass_stall(int,struct strrecvfd *) ;
static int acceptpass_poll(int,struct strrecvfd *,int) ;


/* local variables */


/* exported subroutines */


int acceptpass(int fd_pass,STRRECVFD *sp,int to)
{
	STRRECVFD	extra ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("acceptpass: ent pass-fd=%d\n",fd_pass) ;
#endif

	if (fd_pass < 0)
	    return SR_BADF ;

	if (sp == NULL)
	    sp = &extra ;

	if (to >= 0) {
	    rs = acceptpass_poll(fd_pass,sp,to) ;
	} else {
	    rs = acceptpass_stall(fd_pass,sp) ;
	}

	return rs ;
}
/* end subroutine (acceptpass) */


/* local subroutines */


static int acceptpass_poll(int fd_pass,STRRECVFD *sp,int to)
{
	struct pollfd	pfds[2] ;
	time_t		daytime = time(NULL) ;
	time_t		ti_start ;
	int		rs ;
	int		nfds = 0 ;
	int		fd ;
	int		re ;
	int		i ;
	int		pfd = -1 ;
	int		f_timed = (to > 0) ;

	pfds[nfds].fd = fd_pass ;
	pfds[nfds].events = (POLLIN | POLLPRI) ;
	nfds += 1 ;

	ti_start = daytime ;
	while ((rs = u_poll(pfds,nfds,POLLMULT)) >= 0) {

	    daytime = time(NULL) ;
	    if (rs > 0) {
		for (i = 0 ; (rs >= 0) && (i < nfds) ; i += 1) {
		    fd = pfds[i].fd ;
		    re = pfds[i].revents ;
		    if (fd == fd_pass) {
		   	if ((re & POLLIN) || (re & POLLPRI)) {
			    rs = u_ioctl(fd_pass,I_RECVFD,sp) ;
			    pfd = sp->fd ;
			    if (rs > 0) break ;
			    if ((rs == SR_BADMSG) || (rs == SR_INTR)) {
				rs = SR_OK ;
			    }
			} /* end if (event) */
		    } /* end if (ours) */
		} /* end for */
	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }

	    if ((rs >= 0) && (pfd >= 0)) break ;
	    if ((rs >= 0) && f_timed && ((daytime - ti_start) >= to)) {
		rs = SR_TIMEDOUT ;
	    }

	    if (rs < 0) break ;
	} /* end while (polling) */

	return (rs >= 0) ? pfd : rs ;
}
/* end subroutine (acceptpass_poll) */


static int acceptpass_stall(int fd_pass,STRRECVFD *sp)
{
	int		rs = SR_OK ;
	int		pfd = -1 ;

	while (rs >= 0) {

	    rs = u_ioctl(fd_pass,I_RECVFD,sp) ;
	    pfd = sp->fd ;
	    if (rs >= 0) break ;

	    if ((rs == SR_BADMSG) || (rs == SR_INTR))
		rs = SR_OK ;

	} /* end while */

	return (rs >= 0) ? pfd : rs ;
}
/* end subroutine (acceptpass_stall) */


