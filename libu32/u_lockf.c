/* u_lockf */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-26, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is the friendly version of the library subroutine
        (system call in some older OSes) that handles UNIX® System V style
        locking (not the stupid 'flock()' stuff).

	NOTE:

        Note that the stupid developers of the File Locking mechanism un UNIX
        System V did not distinguish a real deadlock from a temporary lack of
        system resources. We attempt to make up for this screw ball bug in UNIX®
        with our retries on DEADLOCK.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TIMEOUT		100


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_lockf(int fd,int cmd,offset_t size)
{
	int		rs ;
	int		timeout = TIMEOUT ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = lockf(fd,cmd,size)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_DEADLK:
	            if (timeout-- > 0) {
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

	if (rs == SR_ACCES) rs = SR_AGAIN ;

	return rs ;
}
/* end subroutine (u_lockf) */


