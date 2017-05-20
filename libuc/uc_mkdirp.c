/* uc_mkdirp */

/* interface component for UNIX® library-3c */
/* make parent directories in a file path */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine requires linking with the 'libgen(3gen)' library. That
        library is not considered overly useful to the world and therefore has
        been deprecated.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* time outs */

#define	TO_IO		60
#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)


/* exported subroutines */


int uc_mkdirp(cchar *fname,mode_t m)
{
	int		rs ;
	int		to_io = TO_IO ;
	int		to_dquot = TO_DQUOT ;
	int		to_nospc = TO_NOSPC ;
	int		f_exit = FALSE ;

	repeat {
	    rs = mkdirp(fname,m) ;
	    if (rs == -1) {
	        rs = (- errno) ;
	    } else if (rs < -1) {
	        rs = SR_INVALID ;
	    }
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
/* end subroutine (uc_mkdirp) */


