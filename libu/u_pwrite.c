/* u_pwrite */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UPOLL	0		/* use u_poll() subroutine? */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

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

#define	TO_NOSR		(5 * 60)
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


int u_pwrite(fd,buf,len,off)
int		fd ;
const void	*buf ;
int		len ;
offset_t	off ;
{
	struct pollfd	fds[2] ;
	nfds_t		nfds ;
	int		rs ;
	int		rs1 ;
	int		f_init = FALSE ;
	int		to_nosr = TO_NOSR ;
	int		to_nospc = TO_NOSPC ;
	int		to_nolck = TO_NOLCK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	char		*bp = (char *) buf ;

#if	CF_DEBUGS
	debugprintf("u_pwrite: ent FD=%d len=%d\n",fd,len) ;
#endif

	repeat {
	    if ((rs = pwrite(fd,bp,(size_t) len,off)) < 0) rs = (- errno) ;
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
#if	CF_DEBUGS 
	            debugprintf("watch: back from poll w/ rs=%08X\n",rs) ;
#endif
	            if (rs1 > 0) {
	                const int	re = fds[0].revents ;
	                if (re & POLLHUP) {
	                    rs = SR_HANGUP ;
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
	debugprintf("u_pwrite: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_pwrite) */


