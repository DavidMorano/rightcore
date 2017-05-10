/* u_setrlimit */

/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-04-23, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets some process limits.

	Synopsis:

	int u_getrlimit(rn,rp)
	int		rn ;
	struct rlimit	*rp ;

	Arguments:

	rn		resource number of resource to address
	rp		pointer to 'rlimit' structure

	Returns:

	>=0	OK
	<0	error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#if	defined(DARWIN)
#include	<sys/time.h>
#endif

#include	<sys/resource.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_setrlimit(int rn,const struct rlimit *rp)
{
	int		rs ;

	if ((rs = setrlimit(rn,rp)) == -1)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_setrlimit) */


