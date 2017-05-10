/* uc_getipnodeby */

/* subroutine to get a single host entry by name (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

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


**************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>

#include	"localmisc.h"



/* local defines */

#define	TO_GET		3



/* external subroutines */


/* external variables */


/* local structures */


/* forward references */







int uc_getipnodebyname(hostname,af,flags,hepp)
const char	hostname[] ;
int		af ;
int		flags ;
struct hostent	**hepp ;
{
	struct hostent	*lp ;

	int	rs ;
	int	i ;
	int	h_errno ;


#if	CF_DEBUGS
	debugprintf("uc_getipnodebyname: entered hostname=%s\n",
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

	    lp = getipnodebyname(hostname,af,flags,&h_errno) ;

#if	CF_DEBUGS
	    debugprint("uc_getipnodebyname: lp(%p)\n",lp) ;

#endif 

	    if ((lp != NULL) || (h_errno != TRY_AGAIN))
	        break ;

	    sleep(1) ;

	} /* end for */

	rs = SR_OK ;
	if (i >= TO_GET)
	    rs = SR_TIMEDOUT ;

	else if (lp == NULL)
	    rs = SR_NOTFOUND ;

	if ((rs >= 0) && (hepp != NULL))
		*hepp = lp ;
		
#if	CF_DEBUGS
	debugprintf("uc_getipnodebyname: ret is %s\n",
	    (lp == NULL) ? "bad" : "good") ;
#endif

	return rs ;
}
/* end subroutine (uc_getipnodebyname) */



