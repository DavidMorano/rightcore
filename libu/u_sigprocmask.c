/* u_sigprocmask */

/* set the process signal mask */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_sigprocmask(how,setp,osetp)
int		how ;
sigset_t	*setp, *osetp ;
{
	int		rs ;

	if ((rs = sigprocmask(how,setp,osetp)) < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_sigprocmask) */


