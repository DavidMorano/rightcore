/* isproc */

/* is a process (specified by PID) currently in the system? */


#define	CF_DEBUGS	0		/* debug prints */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks to see if the specific process identified by its
        PID, is still on the system. Note that even zombie processes can satisfy
        the search requirement on some systems!

	Synopsis:

	int isproc(pid)
	pid_t	pid ;

	Arguments:

	pid			PID of process to search for

	Returns:

	TRUE			process was found on system
	FALSE			process was not found on system
	<0			error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<signal.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isproc(pid_t pid)
{
	int		rs ;
	int		f = FALSE ;
	if ((rs = u_kill(pid,0)) >= 0) {
	    f = TRUE ;
	} else if (rs == SR_PERM) {
	    f = TRUE ;
	    rs = SR_OK ;
	} else if (rs == SR_SRCH) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (isproc) */


