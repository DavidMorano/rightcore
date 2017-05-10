/* u_nice */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_nice(int value)
{
	int		rs = SR_OK ;
	int		v ;

	errno = 0 ;
	v = nice(value) ;
	if ((v == -1) && (errno != 0)) rs = (- errno) ;

	if ((rs >= 0) && (v >= 0)) rs = v ;
	return rs ;
}
/* end subroutine (u_nice) */


