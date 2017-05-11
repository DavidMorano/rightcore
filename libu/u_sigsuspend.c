/* u_sigsuspend */

/* suspend process until signal arrival */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_INTR		30


/* exported subroutines */


int u_sigsuspend(const sigset_t *ssp)
{
	int		rs ;
	int		to_intr = TO_INTR ;
	int		f_exit = FALSE ;

	repeat {
	    rs = SR_OK ;
	    if (sigsuspend(ssp) == -1) rs = (- errno) ;
	    if (rs < 0) {
		switch (rs) {
	        case SR_INTR:
	            if (to_intr-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_sigsuspend) */


