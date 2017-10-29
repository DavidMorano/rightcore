/* uc_close */

/* interface component for UNIX® library-3c */
/* higher-level "close" */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Filename formats:

	UNIX® domain sockets have the format:
		/filepath

	where:
		filepath

	is just a regular UNIX® file path to the socket file.

	All other protocols have the format:
		/proto/protofamily/protoname/host/service

	where:
		proto		constant name 'proto'
		protofamily	protocol family
					inet
					inet6
		protoname	protocol name
					tcp
					udp
		host		hostname of remote host to contact
		service		service within the specified 
					daytime


	Examples:

	/something/unix/domain/socket

	/proto/inet/tcp/rca/daytime

	/proto/inet/udp/rca/daytime

	/proto/inet6/udp/rca/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_close(int fd)
{
	return u_close(fd) ;
}
/* end subroutine (uc_close) */


