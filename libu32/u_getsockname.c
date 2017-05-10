/* u_getsockname */

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
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#if	CF_XNET
#define	SALEN_T		socklen_t
#else
#define	SALEN_T		int
#endif

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_getsockname(s,fromp,lenp)
int		s ;
void		*fromp ;
int		*lenp ;
{
	struct sockaddr	*sap = (struct sockaddr *) fromp ;
	SALEN_T		sal ;
	int		rs ;
	int		dummy ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		len = 0 ;
	int		f_exit = FALSE ;

	if (lenp == NULL) {
	    dummy = 0 ;
	    lenp = &dummy ;
	}

	repeat {
	    sal = *lenp ;
	    if ((rs = getsockname(s,sap,&sal)) < 0) rs = (- errno) ;
	    *lenp = sal ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
			msleep(1000) ;
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

	len = *lenp ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (u_getsockname) */


