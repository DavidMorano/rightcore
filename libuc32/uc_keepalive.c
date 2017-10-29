/* uc_keepalive */

/* interface component for UNIX® library-3c */
/* set the KeepAlive mode on a (socket) file descriptor */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int uc_keepalive(fd,f)
int		fd ;
int		f ;
{
	const int	slev = SOL_SOCKET ;
	const int	scmd = SO_KEEPALIVE ;

	int	rs ;
	int	keepalive ;
	int	optlen ;
	int	f_previous = FALSE ;


	optlen = sizeof(int) ;
	if ((rs = u_getsockopt(fd,slev,scmd,&keepalive,&optlen)) >= 0) {
	    f_previous = (keepalive != 0) ? 1 : 0 ;
	    if (! LEQUIV(f_previous,f)) {
	        keepalive = f ;
	        rs = u_setsockopt(fd,slev,scmd,&keepalive,sizeof(int)) ;
	    } /* end if (needed a change) */
	} /* end if */

	return (rs >= 0) ? f_previous : rs ;
}
/* end subroutine (uc_keepalive) */


