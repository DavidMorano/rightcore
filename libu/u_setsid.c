/* u_setsid */

/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_setsid()
{
	pid_t		pid ;
	int		rs = SR_OK ;

	if ((pid = setsid()) < 0) rs = (- errno) ;
	if (rs >= 0) rs = (pid & INT_MAX) ;

	return rs ;
}
/* end subroutine (u_setsid) */


