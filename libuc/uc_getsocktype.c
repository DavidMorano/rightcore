/* uc_getsocktype */

/* interface component for UNIX® library-3c */
/* set the NONBLOCK file descriptor open-flag */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	We retrieve a socket type (second argument to |socket(3xnet)|)
	given a socket by way of a file-descriptor.

	Synopsis:

	int uc_getsocktype(int fd)

	Arguments:

	fd		file-descriptor

	Returns:

	>=0		socket type number
	<0		error (like "not-a-socket")


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int uc_getsocktype(int fd)
{
	const int	slev = SOL_SOCKET ;
	const int	scmd = SO_TYPE ;
	int		rs ;
	int		tval = 0 ;
	int		tlen = sizeof(int) ;
	if ((rs = u_getsockopt(fd,slev,scmd,&tval,&tlen)) >= 0) {
	    if (tlen != sizeof(int)) rs = SR_NOMSG ;
	}
	return (rs >= 0) ? tval : rs ;
}
/* end subroutine (uc_getsocktype) */


