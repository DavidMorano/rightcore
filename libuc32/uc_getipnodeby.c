/* uc_getipnodeby */

/* interface component for UNIX® library-3c */
/* subroutine to get a single host entry by name (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_getipnodebyname

	Description:

	This subroutine is a platform independent subroutine to
	get an INET host address entry, but does it dumbly on purpose.

	Synopsis:

	int uc_getipnodebyname(hostname,af,flags,rpp)
	const char	hostname[] ;
	int		af ;
	int		flags ;
	struct hostent	**hepp ;

	Arguments:

	- hostname	hostname to lookup
	- af		desired address family
	- flags		optional flags
	- hepp		pointer to pointer to 'hostent' structure

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found

	Name:

	uc_freehostent

	Description:

	These subroutine is a cleaned up version of
	'freehostent(3socket)'.

	Synopsis:

	int uc_freehostent(hep)
	struct hostent	*hep ;

	Arguments:

	hep		pointer to hostent structure

	Returns:

	>=0		OK
	<0		error SR_FAULT


*******************************************************************************/


#define	LIBUC_MASTER	0


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

#define	TO_GET		3


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


int uc_getipnodebyname(cchar *hostname,int af,int flags,struct hostent **hepp)
{
	struct hostent	*lp ;
	int		rs = SR_OK ;
	int		i ;
	int		h_errno = 0 ;

#if	CF_DEBUGS
	debugprintf("uc_getipnodebyname: ent hostname=%s\n",
	    hostname) ;
#endif

	if (hostname == NULL)
	    return SR_FAULT ;

/* do the real work */

	h_errno = 0 ;
	for (i = 0 ; i < TO_GET ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_getipnodebyname: top of loop\n") ;
#endif

	    errno = 0 ;
	    lp = getipnodebyname(hostname,af,flags,&h_errno) ;

#if	CF_DEBUGS
	    debugprint("uc_getipnodebyname: lp(%p)\n",lp) ;

#endif 

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;
	    msleep(1000) ;

	} /* end for */

	if (i >= TO_GET) {
	    rs = SR_TIMEDOUT ;
	} else if (lp == NULL) {
	    rs = SR_NOTFOUND ;
	}

	if (hepp != NULL)
	    *hepp = (rs >= 0) ? lp : NULL ;
		
#if	CF_DEBUGS
	debugprintf("uc_getipnodebyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getipnodebyname) */


int uc_freehostent(struct hostent *hep)
{

	if (hep == NULL)
	    return SR_FAULT ;

	freehostent(hep) ;

	return SR_OK ;
}
/* end subroutine (uc_freehostent) */


