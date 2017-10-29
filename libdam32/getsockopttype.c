/* getsocktype */

/* get the so-called socket "type" of a given socket (by file-descriptor) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revistion history:

	= 1998-11-06, David A­D­ Morano
	Originally written.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get the so-called socket "type" of a given socket.

	Symopsis:

	int getsocktype(int fd)

	Arguments:

	fd		file-descrptor of socket

	Returns:

	<0		error (probably not a socket)
	>=0		socket "type"


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/conf.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getsocktype(int fd)
{
	const int	slev = SOL_SOCKET ;
	const int	scmd = SO_TYPE ;
	int		rs ;
	int		len = sizeof(int) ;
	int		val = 0 ;
	if ((rs = u_getsockopt(fd,slev,scmd,&val,&len)) >= 0) {
	    if (len != sizeof(int)) rs = SR_NOMSG ;
	}
	return (rs >= 0) ? val : rs ;
}
/* end subroutine (getsocktype) */


