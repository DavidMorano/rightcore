/* u_waitpid */

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
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_waitpid(pid_t pid,int *sp,int flags)
{
	pid_t		rpid ;
	int		rs ;

	repeat {
	    rs = SR_OK ;
	    if ((rpid = waitpid(pid,sp,flags)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	if (rs >= 0) rs = (int) rpid ;

#if	CF_DEBUGS
	debugprintf("waitpid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_waitpid) */


