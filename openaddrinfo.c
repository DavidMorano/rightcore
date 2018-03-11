/* openaddrinfo */

/* open address-information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We open a connection the an address specified by an ADDRINFO structure.

	Synopsis:

	int openaddrinfo(ADDRINFO *aip,int to)

	Arguments:

	aip		pointer to address-information (ADDRINFO) structure
	to		optional time-out

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif


/* external subroutines */


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int openaddrinfo(ADDRINFO *aip,int to)
{
	const int	pf = aip->ai_family ;
	const int	st = aip->ai_socktype ;
	const int	pt = aip->ai_protocol ;
	int		rs ;
	int		fd = -1 ;

	if ((rs = u_socket(pf,st,pt)) >= 0) {
	    struct sockaddr	*sap = aip->ai_addr ;
	    const int		sal = aip->ai_addrlen ;
	    fd = rs ;
	    if (to >= 0) {
	        rs = uc_connecte(fd,sap,sal,to) ;
	    } else {
	        rs = u_connect(fd,sap,sal) ;
	    }
	    if (rs < 0)
	        u_close(fd) ;
	} /* end if (u_socket) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openaddrinfo) */


