/* uc_sigpause */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	60		/* sixty (60) seconds */


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int uc_sigpause(int sn)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = sigpause(sn)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
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
/* end subroutine (uc_sigpause) */


