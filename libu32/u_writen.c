/* u_writen */

/* write a fixed number of bytes out */
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


int u_writen(fd,buf,len)
int	fd ;
char	buf[] ;
int	len ;
{
	int		rs = SR_OK ;
	int		i = 0 ;

#if	CF_DEBUGS
	debugprintf("u_writen: entered len=%d\n",len) ;
	if (len > 1)
	    debugprintf("u_writen: buf[0:%d], >%t<\n",
	        (len - 2),buf,(len - 1)) ;
#endif

	if (buf == NULL) return SR_FAULT ;

	if (len < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("u_writen: about to loop, i=%d\n",i) ;
#endif

	while ((i < len) && ((rs = u_write(fd,(buf + i),(len - i))) > 0)) {
	    i += rs ;
	}

#if	CF_DEBUGS
	debugprintf("u_writen: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs < 0) ? rs : i ;
}
/* end subroutine (u_writen) */


