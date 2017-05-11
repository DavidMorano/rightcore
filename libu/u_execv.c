/* u_execv */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	(1 * 60)	/* fairly long! */
#define	TO_NOMEM	(1 * 60)	/* fairly long! */
#define	TO_TXTBUSY	(1 * 60)	/* fairly long! */


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_execv(cchar *p,cchar **argv)
{
	char *const	*eav = (char *const *) argv ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		to_nomem = TO_NOMEM ;
	int		to_txtbusy = TO_TXTBUSY ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = execv(p,eav)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_NOMEM:
	            if (to_nomem -- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_TXTBUSY:
	            if (to_txtbusy -- > 0) {
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
/* end subroutine (u_execv) */


