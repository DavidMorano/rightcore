/* dialusd */

/* subroutine to dial over to a UNIX® domain socket in data-gram mode */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Dial UNIX® Socket Data-gram (dialusd)

	This subroutine will dial out to an UNIX® domain socket datagram
	address.

	Synopsis:

	int dialusd(dstpath,timeout,options)
	const char	srcpath[] ;
	const char	dstpath[] ;
	int		timeout ;
	int		options ;

	Arguments:

	srcpath		path of UNIX® source domain socket
	dstpath		path to UNIX® destination domain socket to dial to
	timeout		timeout ('>=0' mean use one, '-1' means don't)
	options		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/poll.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local defines */

#define	BUFLEN		(8 * 1024)


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int dialusd(dstpath,timeout,options)
const char	dstpath[] ;
int		timeout ;
int		options ;
{
	const int	pf = PF_UNIX ;
	const int	af = AF_UNIX ;
	int	rs ;
	int	fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialusd: dstpath=%s\n", dstpath) ;
#endif

	if (dstpath == NULL)
	    return SR_FAULT ;

	if (dstpath[0] == '\0')
	    return SR_INVALID ;

/* create the primary socket */

	if ((rs = u_socket(pf,SOCK_DGRAM,0)) >= 0) {
	    SOCKADDRESS	sa ;
	    fd = rs ;
	    if ((rs = sockaddress_start(&sa,af,dstpath,0,0)) >= 0) {
	 	struct sockaddr	*sap = (struct sockaddr *) &sa ;
	        int 		sal = rs ;

	        rs = u_connect(fd,sap,sal) ;

	        sockaddress_finish(&sa) ;
	    } /* end if (sockaddress) */
	    if (rs < 0) u_close(fd) ;
	} /* end if (socket) */

#if	CF_DEBUGS
	debugprintf("dialusd: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialusd) */


