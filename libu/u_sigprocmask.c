/* u_sigprocmask */

/* set the process signal mask */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Set the process signal mask (to something).  When there are more than
	one thread in the proces, this only sets the signal mask of the thread
	it is called from.

        A cousin of this system call is the call |pt_sigmask(3u)|, which is
	supposed to be for thread specific use.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_sigprocmask(int how,sigset_t *setp,sigset_t *osetp)
{
	int		rs ;

	if ((rs = sigprocmask(how,setp,osetp)) < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_sigprocmask) */


