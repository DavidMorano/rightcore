/* u_getgroups */

/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_getgroups(int n,gid_t *a)
{
	int		rs ;

	if ((rs = getgroups(n,a)) < 0) 
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (u_getgroups) */


