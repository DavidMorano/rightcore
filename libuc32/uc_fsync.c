/* uc_fsync */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOSPC	10


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int uc_fsync(int fd)
{
	int		rs ;
	int		to_nospc = TO_NOSPC ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = fsync(fd)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOSPC:
	            if (to_nospc-- > 0) {
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
/* end subroutine (uc_fsync) */


