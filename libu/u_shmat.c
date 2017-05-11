/* u_shmat */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_MFILE	5


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_shmat(shmid,shmaddr,flags,app)
int	shmid ;
void	*shmaddr ;
int	flags ;
void	**app ;
{
	int		rs ;
	int		to_mfile = TO_MFILE ;
	int		f_exit = FALSE ;

	if (app == NULL) return SR_FAULT ;

	repeat {
	    *app = shmat(shmid,shmaddr,flags) ;
	    rs = (((int) *app) == -1) ? (- errno) : 0 ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_MFILE:
	            if (to_mfile-- > 0) {
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
/* end subroutine (u_shmat) */


