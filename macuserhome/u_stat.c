/* u_stat */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_stat(const char *fname,struct ustat *sbp)
{
	int		rs ;

	repeat {
	    if ((rs = stat(fname,sbp)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (u_stat) */


