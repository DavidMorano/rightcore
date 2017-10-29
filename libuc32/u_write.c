/* u_write */

/* UNIX write system call subroutine */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_UPOLL	0		/* use 'u_poll(2u)' ? */


/* revision history:

	= 1986-03-26, David A­D­ Morano

	This subroutine was originally written to get around some
	stupid UNIX sematics of their stupid system calls!


	= 1993-02-21, David A­D­ Morano

	The interrupt code below was changed so that stupid UNIX would
	not ____ up when the file descriptor got a HANUP on the other end.
	This problem surfaced in connection with networking stuff.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOSR		(10 * 60)
#define	TO_NOSPC	(10 * 60)
#define	TO_NOLCK	10
#define	TO_AGAIN	2


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */

#if	defined(DARWIN)
typedef unsigned long		nfds_t ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int u_write(fd,wbuf,wlen)
int		fd ;
const void	*wbuf ;
int		wlen ;
{
	struct pollfd	fds[2] ;

	size_t	wsize = wlen ;

	int	rs ;
	int	rs1 ;
	int	nfds = -1 ;
	int	to_nosr = TO_NOSR ;
	int	to_nospc = TO_NOSPC ;
	int	to_nolck = TO_NOLCK ;
	int	to_again = TO_AGAIN ;
	int	f_init = FALSE ;


#if	CF_DEBUGS
	debugprintf("u_write: ent FD=%d len=%d\n",fd,wlen) ;
#endif

again:
	if ((rs = write(fd,wbuf,wsize)) < 0)
	    rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_write: fd=%d rs=%d\n",fd,rs) ;
#endif

	if (rs < 0) {
	    switch (rs) {

#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)

	    case SR_NOSR:
	        if (to_nosr-- > 0) goto retry ;
	        break ;

#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */

	    case SR_NOSPC:
	        if (to_nospc-- > 0) goto retry ;
	        break ;

	    case SR_NOLCK:
	        if (to_nolck-- > 0) goto retry ;
	        break ;

	    case SR_AGAIN:
	        if (to_again-- > 0) goto retry ;
	        break ;

	    case SR_INTR:
		rs = SR_OK ;
	        if (! f_init) {
	            f_init = TRUE ;
	            nfds = 0 ;
	            fds[nfds].fd = fd ;
	            fds[nfds].events = 0 ;
	            fds[nfds].revents = 0 ;
	            nfds += 1 ;
	        } /* end if */

#if	CF_UPOLL
	            rs1 = u_poll(fds,nfds,0) ;
#else
		{
		    nfds_t n = nfds ;
	            if ((rs1 = poll(fds,n,0)) < 0) rs1 = (- errno) ;
		}
#endif /* CF_UPOLL */

	        if (rs1 > 0) {
	            int	re = fds[0].revents ;

	            if (re & POLLHUP) {
	                rs = SR_HANGUP ;
	            } else if (re & POLLERR) {
	                rs = SR_POLLERR ;
	            } else if (re & POLLNVAL)
	                rs = SR_NOTOPEN ;

	        } /* end if (we had some poll results) */

		if (rs >= 0)
	            goto again ;

	    } /* end switch */
	} /* end if (some kind of error) */

#if	CF_DEBUGS
	debugprintf("u_write: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
retry:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (u_write) */



