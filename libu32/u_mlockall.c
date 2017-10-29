/* u_mlockall */

/* set protection(s) on a mapped memory range */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	I do not know of any bugs in Slowlaris® at this time.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	6


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_mlockall(int flags)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_mlockall: ent\n") ;
#endif

	repeat {
	    if ((rs = mlockall(flags)) < 0) rs = (- errno) ;
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
/* end subroutine (u_mlockall) */


