/* u_readn */

/* read a fixed number of bytes in */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int u_readn(int fd,char *buf,int len)
{
	int		rs = SR_OK ;
	int		i = 0 ;

#if	CF_DEBUGS
	debugprintf("u_readn: entered len=%d\n",len) ;
#endif

	if (buf == NULL) return SR_FAULT ;

	if (len < 0) return SR_INVALID ;

	while ((i < len) && ((rs = u_read(fd,(buf + i),(len - i))) > 0)) {
	    i += rs ;
	}

#if	CF_DEBUGS
	debugprintf("u_readn: ret rs=%d len=%d\n",rs,i) ;
#endif

	return (rs < 0) ? rs : i ;
}
/* end subroutine (u_readn) */


