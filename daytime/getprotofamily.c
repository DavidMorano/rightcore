/* getprotofamily */

/* get a protocol family from an address family */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs */


/* revision history:

	= 1998-04-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find a protocol family (if one exists) that
	corresponds with a specified address family.

	Synopsis:

	int getprotofamily(af)
	int	af ;

	Arguments:

	af		address family

	Returns:

	<0		no corresponding protocol family exists
	>=0		protocol family


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local typedefs */


/* local structures */

struct typematch {
	const int	af, pf ;
} ;


/* forward references */


/* local variables */

static const struct typematch	prototab[] = {
	{ PF_UNSPEC, AF_UNSPEC },
	{ PF_INET, AF_INET },
#ifdef	PF_INET6
	{ PF_INET6, AF_INET6 },
#endif
#ifdef	PF_LOCAL
	{ PF_LOCAL, AF_LOCAL },
#endif
#ifdef	PF_UNIX
	{ PF_UNIX, AF_UNIX },
#endif
#ifdef	PF_APPLETALK
	{ PF_APPLETALK, AF_APPLETALK },
#endif
#if	defined(AF_IMPLINK)
	{ PF_IMPLINK, AF_IMPLINK },
#endif
#if	defined(AF_PUP)
	{ PF_PUP, AF_PUP },
#endif
#if	defined(AF_CHAOS)
	{ PF_CHAOS, AF_CHAOS },
#endif
#if	defined(AF_NS)
	{ PF_NS, AF_NS },
#endif
#if	defined(AF_NBS)
	{ PF_NBS, AF_NBS },
#endif
#if	defined(AF_ECMA)
	{ PF_ECMA, AF_ECMA },
#endif
#if	defined(AF_DATAKIT)
	{ PF_DATAKIT, AF_DATAKIT },
#endif
#if	defined(AF_CCITT)
	{ PF_CCITT, AF_CCITT },
#endif
#if	defined(AF_SNA)
	{ PF_SNA, AF_SNA },
#endif
#if	defined(AF_DECnet)
	{ PF_DECnet, AF_DECnet },
#endif
#if	defined(AF_DLI)
	{ PF_DLI, AF_DLI },
#endif
#if	defined(AF_LAT)
	{ PF_LAT, AF_LAT },
#endif
#if	defined(AF_HYLINK)
	{ PF_HYLINK, AF_HYLINK },
#endif
#if	defined(AF_APPLETALK)
	{ PF_APPLETALK, AF_APPLETALK },
#endif
#if	defined(AF_NIT)
	{ PF_NIT, AF_NIT },
#endif
#if	defined(AF_802)
	{ PF_802, AF_802 },
#endif
#if	defined(AF_OSI)
	{ PF_OSI, AF_OSI },
#endif
#if	defined(AF_X25)
	{ PF_X25, AF_X25 },
#endif
#if	defined(AF_OSINET)
	{ PF_OSINET, AF_OSINET },
#endif
#if	defined(AF_GOSIP)
	{ PF_GOSIP, AF_GOSIP },
#endif
#if	defined(AF_IPX)
	{ PF_IPX, AF_IPX },
#endif
#if	defined(AF_ROUTE)
	{ PF_ROUTE, AF_ROUTE },
#endif
#if	defined(AF_LINK)
	{ PF_LINK, AF_LINK },
#endif
#if	defined(AF_KEY)
	{ PF_KEY, AF_KEY },
#endif
#if	defined(AF_NCA)
	{ PF_NCA, AF_NCA },
#endif
#if	defined(AF_LOCAL)
	{ PF_LOCAL, AF_LOCAL },
#endif
#if	defined(AF_ISDN)
	{ PF_ISDN, AF_ISDN },
#endif
#if	defined(AF_SYSTEM)
	{ PF_SYSTEM, AF_SYSTEM },
#endif
#if	defined(AF_NETBIOS)
	{ PF_NETBIOS, AF_NETBIOS },
#endif
#if	defined(AF_NDRV)
	{ PF_NDRV, AF_NDRV },
#endif
	{ -1, -1 }
} ;


/* exported subroutines */


int getprotofamily(int af)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; prototab[i].af >= 0 ; i += 1) {
	    f = (prototab[i].af == af) ;
	    if (f) break ;
	} /* end for */

	return (f) ? prototab[i].pf : SR_AFNOSUPPORT ;
}
/* end subroutine (getprotofamily) */


