/* getsocktype */

/* get a socket type given a protocol number */
/* open a protocol */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	We retrieve a socket type (second argument to |socket(3xnet)|)
	given a protocol.

	Synopsis:

	int getsocktype(proto)
	int		proto ;

	Arguments:

	proto		protocol number

	Returns:

	>=0		socket type number
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct socktype {
	int		proto ;
	int		type ;
} ;


/* forward references */


/* local variables */

static const struct socktype	socktypes[] = {
	{ IPPROTO_TCP, SOCK_STREAM },
	{ IPPROTO_UDP, SOCK_DGRAM },
	{ IPPROTO_ICMP, SOCK_DGRAM },
	{ IPPROTO_EGP, SOCK_DGRAM },
	{ IPPROTO_GGP, SOCK_DGRAM },
	{ 0, 0 }
} ;


/* exported subroutines */


int getsocktype(int proto)
{
	int		rs = SR_NOTFOUND ;
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; socktypes[i].proto != 0 ; i += 1) {
	    f = (proto == socktypes[i].proto) ;
	    if (f) break ;
	} /* end for */
	if (f) {
	    rs = socktypes[i].type ;
	}
	return rs ;
}
/* end subroutine (getsocktype) */


