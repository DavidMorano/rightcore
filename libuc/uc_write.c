/* uc_write */

/* interface component for UNIX® library-3c */
/* UNIX® write system call subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1986-03-26, David A­D­ Morano
        This subroutine was originally written to get around some stupid UNIX®
        sematics of their stupid system calls!

	= 1993-02-21, David A­D­ Morano
        The interrupt code below was changed so that stupid UNIX® would not ____
        up when the file descriptor got a HANUP on the other end. This problem
        surfaced in connection with networking stuff.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an enhanced |write(2)|-like subroutine that waits until a write
        will not hang before making the actual write.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<limits.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	TO_NOSR		(10 * 60)
#define	TO_NOSPC	(10 * 60)
#define	TO_NOLCK	10
#define	TO_AGAIN	2


/* external subroutines */

extern int	iaddsat(int,int) ;
extern long	laddsat(long,long) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_write(int fd,const void *ubuf,int ulen,int to)
{
	struct pollfd	fds[1] ;

	time_t		ti_now ;
	time_t		ti_start ;

	int		rs = SR_OK ;
	int		nfds = -1 ;
	int		len = 0 ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_write: ent FD=%d ulen=%d\n",fd,ulen) ;
#endif

	if (to >= 0) {
	    ti_now = time(NULL) ;
	    ti_start = ti_now ;
	}

	nfds = 0 ;
	fds[nfds].fd = fd ;
	fds[nfds].events = POLLOUT ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	while (rs >= 0) {
	    if ((rs = u_poll(fds,nfds,500)) >= 0) {
	        if (rs > 0) {
	            const int re = fds[0].revents ;

	            if (re & POLLHUP) {
	                rs = SR_HANGUP ;
	            } else if (re & POLLERR) {
	                rs = SR_POLLERR ;
	            } else if (re & POLLNVAL) {
	                rs = SR_NOTOPEN ;
	            } else if (re & POLLOUT) {
			f_exit = TRUE ;
	                rs = u_write(fd,ubuf,ulen) ;
	                len = rs ;
	            }

	        } /* end if (got something) */
	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }
	    if (rs >= 0) {
		if (f_exit) break ;
		if (to >= 0) {
		    ti_now = time(NULL) ;
		    if ((ti_now-ti_start) >= to) {
		        rs = SR_TIMEDOUT ;
		        break ;
		    }
		}
	    }
	} /* end while */

#if	CF_DEBUGS
	debugprintf("uc_write: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_write) */


