/* u_tell */

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


int u_tell(int fd,offset_t *rp)
{
	offset_t	ro ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("u_tell: ent fd=%d\n",fd) ;
#endif

	repeat {
	    rs = SR_OK ;
	    if ((ro = lseek(fd,0L,SEEK_CUR)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;
	if (rs >= 0) rs = (ro & INT_MAX) ;

	if (rp != NULL) {
	    *rp = (rs >= 0) ? ro : 0 ;
	}

#if	CF_DEBUGS
	debugprintf("u_tell: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_tell) */


