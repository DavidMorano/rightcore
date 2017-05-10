/* u_bind */

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
#include	<errno.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOBUFS	60
#define	TO_NOSR		(5 * 60)

#if	CF_XNET
#define	SALEN_T		socklen_t
#else
#define	SALEN_T		int
#endif


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_bind(int s,void *asap,int asal)
{
	struct sockaddr	*sap = (struct sockaddr *) asap ;
	SALEN_T		sal ;
	int		rs ;
	int		to_nobufs = TO_NOBUFS ;
	int		to_nosr = TO_NOSR ;
	int		f_exit = FALSE ;

	repeat {
	    sal = (SALEN_T) asal ;
	    if ((rs = bind(s,sap,sal)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOBUFS:
	            if (to_nobufs -- > 0) {
		        msleep(1) ;
		    } else {
		        f_exit = TRUE ;
		    }
	            break ;
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
		        msleep(1) ;
		    } else {
		        f_exit = TRUE ;
		    }
	            break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */
	        case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_bind) */


