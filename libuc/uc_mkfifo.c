/* uc_mkfifo */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_IO		(5 * 60)
#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int uc_mkfifo(cchar *path,mode_t m)
{
	int		rs ;
	int		to_io = TO_IO ;
	int		to_dquot = TO_DQUOT ;
	int		to_nospc = TO_NOSPC ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = mkfifo(path,m)) < 0) rs = (- errno) ;
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
	        case SR_AGAIN:
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
/* end subroutine (uc_mkfifo) */


