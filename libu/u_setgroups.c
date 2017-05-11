/* u_setgroups */

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
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_setgroups(int n,const gid_t *list)
{
	int		rs ;

	if ((rs = setgroups(n,list)) < 0)
	    rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_setgroups: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_setgroups) */


