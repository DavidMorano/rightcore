/* u_rename */

/* rename (link to and delete original) a file */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Contrary to a good bit of confusion (at least at the present time) this
        subroutine is indeed a system call. Why it was advertized as a STDIO
        call at one point was pure craziness. Maybe it is still advertised as a
        STDIO call with your implementation but it should not be. Check its
        object file for any crazy STDIO functions of variables.

        The confusion over this subroutine is just part of some of the wacked
        out craziness from the earlier UNIX® days. Yes, UNIX® went on to become
        big (I watched it myself through the decades from nothing to
        take-over-the-world status) but the guys up in the old Computer-Science
        Area-11 research division could have taken some time to have cleaned
        some things up before they got too entrenched (like the confusion over
        this subroutine call).


*******************************************************************************/


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


/* time outs */

#define	TO_IO		60
#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)
#define	TO_BUSY		20


/* exported subroutines */


int u_rename(cchar *old,cchar *new)
{
	int		rs ;
	int		to_io = TO_IO ;
	int		to_dquot = TO_DQUOT ;
	int		to_nospc = TO_NOSPC ;
	int		to_busy = TO_BUSY ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = rename(old,new)) < 0) rs = (- errno) ;
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
	            if (to_dquot -- > 0) {
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
/* end subroutine (u_rename) */


