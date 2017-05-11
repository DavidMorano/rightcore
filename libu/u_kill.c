/* u_kill */

/* kill a process */
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


int u_kill(pid_t pid,int sig)
{
	int		rs ;

	if ((rs = kill(pid,sig)) < 0) 
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_kill) */


