/* inet4int */

/* convert an INETv4 address (in network form) to an unsigned-integer */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2003-11-06, David A­D­ Morano
        This subroutine exists to make some sensible version out of the
        combination of 'inet_addr(3n)' and 'inet_pton(3socket)'.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We convert an INETv4 address (in network form) to a host
        unsigned-integer in network order.

	Synopsis:

	uint inet4int(const void *ap,int al)

	Arguments:

	ap		pointer to INETv4 address in network form
	al		length of INETv4 address

	Returns:

	-		integer that is the INETv4 address in network order


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	sizeof(in6_addr_t)
#endif


/* external subroutines */


/* local variables */


/* exported subroutines */


uint inet4int(const void *ap)
{
	uint		v = 0 ;
	int		i ;
	cchar		*cp = (cchar *) ap ;
	for (i = 0 ; i < 4 ; i += 1) {
	    v <<= 8 ;
	    v |= ((uchar) *cp) ;
	}
	return v ;
}
/* end subroutine (inet4int) */


