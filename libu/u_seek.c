/* u_seek */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_seek(int fd,offset_t o,int w)
{
	offset_t	ro ;
	int		rs ;

	repeat {
	    rs = SR_OK ;
	    if ((ro = lseek(fd,o,w)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	if (rs >= 0) rs = (ro & INT_MAX) ;
	return rs ;
}
/* end subroutine (u_seek) */


