/* u_accept */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_XNET		1		/* use 'xnet' library */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)

#if	CF_XNET
#define	SALEN_T		socklen_t
#else
#define	SALEN_T		int
#endif


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_accept(int s,const void *fromp,int *fromlenp)
{
	SOCKADDR	*sap = (struct sockaddr *) fromp ;
	SALEN_T		salen ;
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_accept: ent s=%d\n",s) ;
#endif

	if ((fromp != NULL) && (fromlenp == NULL))
	    return SR_FAULT ;

	repeat {
	    salen = *fromlenp ;
	    if ((rs = accept(s,sap,&salen)) < 0) rs = (- errno) ;
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
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	if (rs >= 0) {
	    *fromlenp = salen ;
	}

#if	CF_DEBUGS
	debugprintf("u_accept: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_accept) */


