/* u_times */

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
#include	<sys/times.h>
#include	<limits.h>
#include	<time.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_times(struct tms *rp)
{
	clock_t		tics ;
	int		rs ;

	tics = times(rp) ;
	rs = (tics & INT_MAX) ;
	if (tics == -1) rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_times) */


