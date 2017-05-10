/* u_readlink */

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
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_readlink(cchar *fname,char *rbuf,int rlen)
{
	int		rs ;

	repeat {
	    if ((rs = readlink(fname,rbuf,rlen)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	if (rs >= 0) {
	    rbuf[rs] = '\0' ;
	} else {
	    rbuf[0] = '\0' ;
	}

	return rs ;
}
/* end subroutine (u_readlink) */


