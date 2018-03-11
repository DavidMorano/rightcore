/* getaf */

/* get an address-family by name */


/* revision history:

	= 1999-10-14, David A­D­ Morano
        This was written to get a roughly standardized subroutine to handle both
        IPv4 and IPv6. Note that the order of the AF list isn't in the order to
        the definitions of the defines. Rather, since searching is linear
        (probably the fastest way), the order is such that the most popular AFs
        are near the top!

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines allow for lookup of either an address-family name or
	an address-family number into a database of name-number pairs.

	The database is not dynamic, but rather compiled into this module.  A
	dynamic database is really only possible if we know where the stupid
	real '<sys/socket.h>' file is located (since we would need to read and
	process that file to get the name-number pairs).  This problem really
	illustrates the stupidity of the whole BSD "socket" API and only goes
	to show the far superiority of the AT&T TLI (or XTI) API.

	Synopsis:

	int getaf(np,nl)
	const char	*np ;
	int		nl ;

	Arguments:

	np		name of the address family to lookup
	nl		length of name

	Returns:

	>=0		resulting address-family index
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	AFNAMELEN
#define	AFNAMELEN	12		/* maximum of lengths in table below */
#endif

#ifndef	AF_INET4
#define	AF_INET4	AF_INET
#endif

#define	ADDRFAM		struct addrfam


/* external subroutines */

extern int	cfdeci(cchar *,int,int *) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	hasalldig(cchar *,int) ;

extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* local structures */

struct addrfam {
	cchar		*name ;
	int		af ;
} ;


/* local variables */

static const struct addrfam	addrfamilies[] = {
	{ "unix", AF_UNIX },
	{ "inet", AF_INET },
	{ "inet4", AF_INET4 },
	{ "inet6", AF_INET6 },
#if	defined(AF_IMPLINK)
	{ "implink", AF_IMPLINK },
#endif
#if	defined(AF_PUP)
	{ "pup", AF_PUP },
#endif
#if	defined(AF_CHAOS)
	{ "chaos", AF_CHAOS },
#endif
#if	defined(AF_NS)
	{ "ns", AF_NS },
#endif
#if	defined(AF_NBS)
	{ "nbs", AF_NBS },
#endif
#if	defined(AF_ECMA)
	{ "ecma", AF_ECMA },
#endif
#if	defined(AF_DATAKIT)
	{ "datakit", AF_DATAKIT },
#endif
#if	defined(AF_CCITT)
	{ "ccitt", AF_CCITT },
#endif
#if	defined(AF_SNA)
	{ "sna", AF_SNA },
#endif
#if	defined(AF_DECnet)
	{ "decnet", AF_DECnet },
#endif
#if	defined(AF_DLI)
	{ "dli", AF_DLI },
#endif
#if	defined(AF_LAT)
	{ "lat", AF_LAT },
#endif
#if	defined(AF_HYLINK)
	{ "hylink", AF_HYLINK },
#endif
#if	defined(AF_APPLETALK)
	{ "appletalk", AF_APPLETALK },
#endif
#if	defined(AF_NIT)
	{ "nit", AF_NIT },
#endif
#if	defined(AF_802)
	{ "802", AF_802 },
#endif
#if	defined(AF_OSI)
	{ "osi", AF_OSI },
#endif
#if	defined(AF_X25)
	{ "x25", AF_X25 },
#endif
#if	defined(AF_OSINET)
	{ "osinet", AF_OSINET },
#endif
#if	defined(AF_GOSIP)
	{ "gosip", AF_GOSIP },
#endif
#if	defined(AF_IPX)
	{ "ipx", AF_IPX },
#endif
#if	defined(AF_ROUTE)
	{ "route", AF_ROUTE },
#endif
#if	defined(AF_LINK)
	{ "link", AF_LINK },
#endif
#if	defined(AF_KEY)
	{ "key", AF_KEY },
#endif
#if	defined(AF_NCA)
	{ "nca", AF_NCA },
#endif
#if	defined(AF_LOCAL)
	{ "local", AF_LOCAL },
#endif
#if	defined(AF_ISDN)
	{ "isdn", AF_ISDN },
#endif
#if	defined(AF_SYSTEM)
	{ "system", AF_SYSTEM },
#endif
#if	defined(AF_NETBIOS)
	{ "netbios", AF_NETBIOS },
#endif
#if	defined(AF_NDRV)
	{ "ndrv", AF_NDRV },
#endif
	{ "unspecified", AF_UNSPEC },
	{ NULL, 0 }
} ;


/* exported subroutines */


int getaf(cchar *np,int nl)
{
	int		rs ;

	if (np == NULL) return SR_FAULT ;
	if ((nl == 0) || (np[0] == '\0')) return SR_INVALID ;

	if (hasalldig(np,nl)) {
	    int	v ;
	    if ((rs = cfdeci(np,nl,&v)) >= 0) {
		rs = v ;
	    }
	} else {
	    const int	alen = AFNAMELEN ;
	    int		al ;
	    char	abuf[AFNAMELEN + 1] ;
	    if ((al = (strdcpy1w(abuf,alen,np,nl)-abuf)) > 0) {
	        const ADDRFAM	*afs = addrfamilies ;
	        const int	n = 2 ;
	        int		i ;
	        int		m ;
	        cchar		*anp ;
	        for (i = 0 ; afs[i].name != NULL ; i += 1) {
	            anp = afs[i].name ;
	            m = nleadstr(anp,abuf,al) ;
	            if ((m == al) && ((m >= n) || (anp[m] == '\0'))) break ;
	        } /* end for */
	        rs = (afs[i].name != NULL) ? afs[i].af : SR_AFNOSUPPORT ;
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (digit or string) */

	return rs ;
}
/* end subroutine (getaf) */


/* reads out as: str-af-name */
const char *strafname(int af)
{
	const ADDRFAM	*afs = addrfamilies ;
	int		i ;

	for (i = 0 ; afs[i].name != NULL ; i += 1) {
	    if (afs[i].af == af) break ;
	} /* end for */

	return (afs[i].name != NULL) ? afs[i].name : "unknown" ;
}
/* end subroutine (strafname) */


