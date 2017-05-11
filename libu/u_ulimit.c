/* u_ulimit */

/* get the standard load averages maintained by the kernel */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-04-23, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets some process limits.

	Synopsis:

	int u_ulimit(cmd,nval)
	int		cmd ;
	int		nval ;

	Arguments:

	cmd		particular limit specifier 
	nval		possible new value for specific limit

	Returns:

	>=0	OK
	<0	error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/resource.h>
#include	<limits.h>
#include	<ulimit.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_ulimit(int cmd,int nval)
{
	int		rs = SR_OK ;
	int		rval ;

	errno = 0 ;
	rval = ulimit(cmd,nval) ;
	if ((rval == -1) && (errno != 0)) rs = (- errno) ;

	if (rs >= 0) rs = (rval & INT_MAX) ;
	return rs ;
}
/* end subroutine (u_ulimit) */


