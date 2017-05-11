/* uc_getaddrinfo */

/* interface component for UNIX® library-3c */
/* manipulate host-address entry structures */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_getaddrinfo

	Description:

	This is a nicer version of the standard 'getaddrinfo(3socket)'
	subroutine.

	Synopsis:

	int uc_getaddrinfo(hostname,svc,hintp,rpp)
	const char	hostname[] ;
	const char	svc[] ;
	struct addrinfo	*hintp ;
	struct addrinfo **rpp ;

	Arguments:

	hostname	host-name
	svc		service-name
	hintp		pointer to a 'addrinfo' structure (as a hint)
	rpp		pointer to pointer to 'addrinfo' structure

	Returns:

	>=0		OK
	<0		error


	Name:

	uc_freeaddrinfo

	Description:

	These subroutine is a cleaned up version of
	'getaddrinfo(3socket)'.

	Synopsis:

	int uc_freeaddrinfo(aip)
	struct addrinfo	*aip ;

	Arguments:

	aip		pointer to 'addrinfo' structure

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_getaddrinfo(hostname,svc,hintp,rpp)
const char	hostname[] ;
const char	svc[] ;
struct addrinfo	*hintp ;
struct addrinfo **rpp ;
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		rc = 0 ;
	int		f_exit = FALSE ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    rc = getaddrinfo(hostname,svc,hintp,rpp) ;
	    if (rc != 0) {
	        switch (rc) {
	        case EAI_ADDRFAMILY:
	            rs = SR_AFNOSUPPORT ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_AGAIN:
	            if (to_again-- > 0) {
		        msleep(100) ;
		    } else {
	                rs = SR_AGAIN ;
			f_exit = TRUE ;
		    }
	            break ;
	        case EAI_BADFLAGS:
	            rs = SR_INVALID ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_FAIL:
	            rs = SR_NOANODE ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_FAMILY:
	            rs = SR_PFNOSUPPORT ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_MEMORY:
	            rs = SR_NOMEM ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_NODATA:
	            rs = SR_NODATA ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_NONAME:
	            rs = SR_NOTFOUND ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_SERVICE:
	            rs = SR_ADDRNOTAVAIL ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_SOCKTYPE:
	            rs = SR_SOCKTNOSUPPORT ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_SYSTEM:
	            rs = (- errno) ;
	            switch (rs) {
	            case SR_AGAIN:
	                if (to_again-- > 0) {
			    msleep(100) ;
			} else {
			    f_exit = TRUE ;
			}
	                break ;
		    default:
		        f_exit = TRUE ;
			break ;
	            } /* end switch */
	            break ;
	        default:
	            rs = SR_NOANODE ;
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (some sort of error condition) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getaddrinfo) */


int uc_freeaddrinfo(struct addrinfo *aip)
{
	if (aip == NULL) return SR_FAULT ;
	freeaddrinfo(aip) ;
	return SR_OK ;
}
/* end subroutine (uc_freeaddrinfo) */


