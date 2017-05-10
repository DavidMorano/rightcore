/* uc_munlockall */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/lock.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */


/* exported subroutines */


int uc_munlockall()
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_munlockall: ent\n") ;
#endif

	repeat {
	    if ((rs = munlockall()) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
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

	return rs ;
}
/* end subroutine (uc_munlockall) */


