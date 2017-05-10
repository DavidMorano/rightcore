/* u_msgctl */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Note: We have no reason to believe that this subroutine can generate an
	SR_NOSPC or even a SR_INTR.  But we have code included to handle these
	(with retries).  If these error types never occur, then no harm was
	done.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOSPC	5


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_msgctl(int msgid,int cmd,struct msqid_ds *buf)
{
	int		rs ;
	int		to_nospc = TO_NOSPC ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = msgctl(msgid,cmd,buf)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
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
/* end subroutine (u_msgctl) */


