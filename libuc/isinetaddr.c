/* isinetaddr */

/* subroutine to determine if the name is an INET address a */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INET6	1		/* test for both INET4 and INET6 */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks if the given ASCII string represents an INET4
        address or not.

	Synopsis:

	int isinetaddr(name)
	const char	name[] ;

	Arguments:

	name		address (character string) to test

	Returns:

	TRUE		it is an INET address
	FALSE		it is not


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ADDRBUFLEN
#define	ADDRBUFLEN	64
#endif

#define	BADTHING	(~ 0)


/* external subroutines */

extern int	inetpton(char *,int,int,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isinetaddr(const char *name)
{
	int		f ;

#if	CF_INET6
	{
	    int		rs1 ;
	    int		af = AF_UNSPEC ;
	    char	addrbuf[ADDRBUFLEN + 1] ;
	    rs1 = inetpton(addrbuf,ADDRBUFLEN,af,name,-1) ;
	    f = (rs1 >= 0) ;
	}
#else
	f = (inet_addr(name) != ((in_addr_t) BADTHING)) ;
#endif /* CF_INET6 */

	return f ;
}
/* end subroutine (isinetaddr) */


