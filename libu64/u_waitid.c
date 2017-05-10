/* u_waitid */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<wait.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int u_waitid(idtype_t idtype,id_t id,siginfo_t *sip,int opts)
{
	int		rs ;

	repeat {
	    if ((rs = waitid(idtype,id,sip,opts)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (u_waitid) */


