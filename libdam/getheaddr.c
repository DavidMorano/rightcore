/* getheaddr */

/* subroutine to get a single host entry by its address */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is a platform independent subroutine to get an INET host
        entry by its INET address, but does it dumbly on purpose.

	Synopsis:

	int getheaddr(addr,hep,hbuf,hlen)
	const char	addr[] ;
	struct hostent	*hep ;
	char		hbuf[] ;
	int		hlen ;

	Arguments:

	- addr		address to lookup
	- hep		pointer to 'hostent' structure
	- hbuf		user supplied hbuffer to hold result
	- hlen	length of user supplied hbuffer

	Returns:

	>=0		host was found OK
	<0		error


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

#ifndef	INET4DOTDECLEN
#define	INET4DOTDECLEN	16
#endif

#define	TIMEOUT		3


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


int getheaddr(cchar *addr,struct hostent *hep,char *hbuf,int hlen)
{
	const int	af = AF_INET ;
	int		rs ;
	int		addrlen ;

#if	CF_DEBUGS
	debugprintf("getheaddr: ent\n") ;
#endif

	if (addr == NULL) return SR_FAULT ;
	if (hep == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

	addrlen = sizeof(struct in_addr) ;
	rs = uc_gethostbyaddr(addr,addrlen,af,hep,hbuf,hlen) ;

#if	CF_DEBUGS
	debugprintf("getheaddr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getheaddr) */


