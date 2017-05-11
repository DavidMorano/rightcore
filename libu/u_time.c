/* u_time */

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
#include	<time.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_time(time_t *rp)
{
	time_t		ut ;
	int		rs ;

	ut = time(rp) ;
	rs = (ut & INT_MAX) ;
	if (ut < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_time) */


