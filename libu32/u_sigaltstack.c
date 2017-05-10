/* u_sigaltstack */

/* establish alternate signal stack */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_sigaltstack(const stack_t *ssp,stack_t *ossp)
{
	int		rs ;

	if ((rs = sigaltstack(ssp,ossp)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_sigaltstack) */


