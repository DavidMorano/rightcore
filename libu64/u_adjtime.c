/* u_adjtime */

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
#include	<sys/time.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external variables */


/* exported subroutines */


int u_adjtime(struct timeval *new,struct timeval *old)
{
	int		rs ;

	if ((rs = adjtime(new,old)) < 0) 
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_adjtime) */


