/* u_sbrk */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOMEM	60
#define	TO_AGAIN	60


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_sbrk(int incr,void **rpp)
{
	void		*rp ;
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    rs = SR_OK ;
	    if ((rp = (void *) sbrk(incr)) == ((void *) -1)) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
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
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? rp : NULL ;
	}

	return rs ;
}
/* end subroutine (u_sbrk) */


