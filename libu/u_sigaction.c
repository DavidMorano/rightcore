/* u_sigaction */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_sigaction(int sn,SIGACTION *nsp,SIGACTION *osp)
{
	int		rs ;

	if ((rs = sigaction(sn,nsp,osp)) < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_sigaction) */


