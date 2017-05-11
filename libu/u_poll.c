/* u_poll */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INTR		0		/* do not return on an interrupt */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>

#if	(defined(SYSHAS_POLL) && (SYSHAS_POLL > 0))
#else
#include	<sys/time.h>
#include	<sys/sockio.h>
#endif

#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	UPOLL_NATIVE		(defined(SYSHAS_POLL) && (SYSHAS_POLL == 1))

#ifndef	UPOLL_RESOLUTION
#define	UPOLL_RESOLUTION	1000
#endif

#ifndef	INFTIM
#define	INFTIM		(-1)
#endif

#ifndef	POLLRDNORM
#define	POLLRDNORM	POLLIN
#endif

#ifndef	POLLRDBAND
#define	POLLRDBAND	0
#endif

#ifndef	POLLWRNORM
#define	POLLWRNORM	POLLOUT
#endif

#ifndef	POLLWRBAND
#define	POLLWRBAND	0
#endif

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

#if	defined(DARWIN)
typedef unsigned long		nfds_t ;
#endif


/* forward references */

#if	(UPOLL_NATIVE == 0)
static int	uc_select(int,fd_set *,fd_set *,fd_set *,struct timeval *) ;
#endif


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_POLL) && (SYSHAS_POLL == 1)

int u_poll(struct pollfd *fds,int n,int timeout)
{
	const nfds_t	nfds = (nfds_t) n ; /* duh! 'int' wasn't good enough */
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = poll(fds,nfds,timeout)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
#if	CF_INTR
	        case SR_INTR:
	             break ;
#endif /* CF_INTR */
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (poll got an error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_poll) */

#else /* SYSHAS_POLL */

int u_poll(struct pollfd *fds,int n,int timeout)
{
	struct timeval	tv ;
	fd_set		rset, wset, eset ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		fd ;
	int		c = 0 ;

	if (n < 0)
	    return SR_INVALID ;

	if (n > FD_SETSIZE)
	    return SR_NOTSUP ;

	FD_ZERO(&rset) ;

	FD_ZERO(&wset) ;

	FD_ZERO(&eset) ;

	rs = SR_OK ;
	for (i = 0 ; i < n ; i += 1) {

	    fd = fds[i].fd ;
	    if (fd >= FD_SETSIZE) {
	        rs = SR_TOOBIG ;
	        break ;
	    }

	    fds[i].revents = 0 ;
	    if (fd >= 0) {

	        if ((fds[i].events & POLLIN) ||
	            (fds[i].events & POLLPRI) ||
	            (fds[i].events & POLLRDNORM) ||
	            (fds[i].events & POLLRDBAND))
	            FD_SET(fd,&rset) ;

	        if ((fds[i].events & POLLOUT) ||
	            (fds[i].events & POLLWRBAND))
	            FD_SET(fd,&wset) ;

	    }

	} /* end for */

	if (timeout != INFTIM) {
	    tv.tv_sec = timeout / UPOLL_RESOLUTION ;
	    tv.tv_usec = timeout % UPOLL_RESOLUTION ;
	} else {
	    tv.tv_sec = INT_MAX ;
	    tv.tv_usec = 0 ;
	}

	if (rs >= 0)
	    rs = uc_select(n,&rset,&wset,&eset,&tv) ;

	if (rs >= 0) {
	    int	v ;
	    int	f ;

	    c = 0 ;
	    for (i = 0 ; i < n ; i += 1) {

	        fd = fds[i].fd ;
	        if (fd >= 0) {

	            f = FALSE ;
	            if (FD_ISSET(fd,&rset)) {
	                f = TRUE ;
	                if (fds[i].events & POLLIN)
	                    fds[i].revents |= POLLIN ;

#ifdef	COMMENT
	                if (fds[i].events & POLLPRI)
	                    fds[i].revents |= POLLPRI ;
#endif /* COMMENT */

	                if (fds[i].events & POLLRDNORM)
	                    fds[i].revents |= POLLRDNORM ;

	                if (fds[i].events & POLLRDBAND) {
	                    rs1 = u_ioctl(fd,SIOCATMARK,&v) ;
	                    if ((rs1 < 0) || (v > 0)) {
	                        fds[i].revents |= POLLRDBAND ;
			    }
	                }

	            } /* end if */

	            if (FD_ISSET(fd,&wset)) {
	                f = TRUE ;
	                if (fds[i].events & POLLOUT) {
	                    fds[i].revents |= POLLOUT ;
			}
	                if (fds[i].events & POLLWRBAND) {
	                    fds[i].revents |= POLLWRBAND ;
			}
	            }

	            if (FD_ISSET(fd,&eset)) {
	                f = TRUE ;
	                fds[i].revents |= POLLERR ;
	            }

	            if (f)
	                c += 1 ;

	        } /* end if */

	    } /* end for */

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (u_poll) */


static int uc_select(nfds,readfds,writefds,errorfds,tp)
int	nfds ;
fd_set	*readfds ;
fd_set	*writefds ;
fd_set	*errorfds ;
struct timeval	*tp ;
{
	int		rs ;

	rs = select(nfds,readfds,writefds,errorfds,tp) ;
	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_select) */

#endif /* SYSHAS_POLL */


