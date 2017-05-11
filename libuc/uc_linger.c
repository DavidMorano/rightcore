/* uc_linger */

/* interface component for UNIX® library-3c */
/* set (or unset) a LINGER time-out on a socket */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int uc_linger(int fd,int to)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    if (S_ISSOCK(sb.st_mode)) {
	        struct linger	ls ;
		const int	sol = SOL_SOCKET ;
		const int	cmd = SO_LINGER ;
		const int	llen = sizeof(struct linger) ;
	        memset(&ls,0,sizeof(struct linger)) ;
		if (to >= 0) {
	            ls.l_onoff = TRUE ;
	            ls.l_linger = to ;
		}
	        rs = u_setsockopt(fd,sol,cmd,(cchar *) &ls,llen) ;
	    } /* end if (socket) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (uc_linger) */


