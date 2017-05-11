/* u_writev */

/* UNIX write system call subroutine */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_UPOLL	0		/* use 'u_poll(3u)' ? */


/* revision history:

	= 1998-03-26, David A­D­ Morano
        This subroutine was originally written to get around some stupid UNIX®
        sematics of their stupid system calls!

	= 2003-02-21, David A­D­ Morano
        This interrupt code below was changed so that stupid UNIX® would not
        ____ up when the file descriptor got a HANUP on the other end. This
        problem surfaced in connection with networking stuff.

*/

/* Copyright © 1998,2003 David A­D­ Morano.  All rights reserved. */


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


/* local structures */

#if	defined(DARWIN)
typedef unsigned long		nfds_t ;
#endif


/* exported subroutines */


int u_writev(fd,iop,n)
int		fd ;
const struct iovec	*iop ;
int		n ;
{
	struct pollfd	fds[2] ;
	nfds_t		nfds ;
	int		rs, rs1 ;
	int		f_init = FALSE ;
	int		to_nosr = TO_NOSR ;
	int		to_nospc = TO_NOSPC ;
	int		to_nolck = TO_NOLCK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_writev: ent FD=%d len=%d\n",fd,len) ;
#endif

	repeat {
	    if ((rs = writev(fd,iop,n)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */
	        case SR_NOSPC:
	            if (to_nospc-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_NOLCK:
	            if (to_nolck-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_AGAIN:
	            if (to_again-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            if (! f_init) {
	                f_init = TRUE ;
	                nfds = 0 ;
	                fds[nfds].fd = fd ;
	                fds[nfds].events = 0 ;
	                fds[nfds].revents = 0 ;
	                nfds += 1 ;
	            } /* end if */
#if	CF_UPOLL
	            rs1 = u_poll(fds,(int) nfds,0) ;
#else
	            if ((rs1 = poll(fds,nfds,0)) < 0) rs1 = (- errno) ;
#endif
	            if (rs1 > 0) {
	                const int	re = fds[0].revents ;
	                if (re & POLLHUP) {
	                    rs = SR_HANGUP ;	/* same as SR_IO */
	                } else if (re & POLLERR) {
	                    rs = SR_POLLERR ;
	                } else if (re & POLLNVAL) {
	                    rs = SR_NOTOPEN ;
			}
			f_exit = (rs < 0) ;
	            } /* end if (we had some poll results) */
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (some kind of error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("u_writev: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_writev) */


