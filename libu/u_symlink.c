/* u_symlink */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_IO		60
#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)
#define	TO_BUSY		20


/* exported subroutines */


int u_symlink(cchar *src,cchar *dst)
{
	int		rs ;
	int		to_io = TO_IO ;
	int		to_dquot = TO_DQUOT ;
	int		to_nospc = TO_NOSPC ;
	int		to_busy = TO_BUSY ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = symlink(src,dst)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_IO:
	            if (to_io-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_DQUOT:
	            if (to_dquot-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_NOSPC:
	            if (to_nospc-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_BUSY:
	            if (to_busy-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_AGAIN:
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error condition) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_symlink) */


