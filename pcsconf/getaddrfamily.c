/* getaddrfamily */

/* get an address-family by name */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutines gets (retrieves) an address family index (whatever)
	given an address family name.

	Synopsis:

	int getaddrfamily(name)
	const char	name[] ;

	Arguments:

	name		name of the address family to lookup

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
#define	AFNAMELEN	12
#endif


/* external subroutines */

extern int	nleadstr(cchar *,cchar *,int) ;
extern int	nleadcasestr(cchar *,cchar *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* local structures */

struct addrfamily {
	const char	*name ;
	int		af ;
} ;


/* local variables */

static const struct addrfamily	addrfamilies[] = {
	{ "unspecified", AF_UNSPEC },
	{ "unix", AF_UNIX },
	{ "inet", AF_INET },
	{ "inet4", AF_INET },
	{ "inet6", AF_INET6 },
	{ NULL, 0 }
} ;


/* exported subroutines */


int getaddrfamily(cchar *name)
{
	const struct addrfamily	*afs = addrfamilies ;
	int		i ;
	int		m, m_max = 0 ;
	int		si = -1 ;
	int		cnamelen ;
	const char	*anp ;
	char		cname[AFNAMELEN + 1] ;

	cnamelen = strwcpylc(cname,name,AFNAMELEN) - cname ;

	for (i = 0 ; afs[i].name != NULL ; i += 1) {

	    anp = afs[i].name ;
	    if ((m = nleadstr(anp,cname,cnamelen)) >= 2) {

	        if (m > m_max) {
	            m_max = m ;
	            si = i ;
	        }

	    } /* end if */

	} /* end for */

	return (si >= 0) ? afs[si].af : SR_AFNOSUPPORT ;
}
/* end subroutine (getaddrfamily) */


