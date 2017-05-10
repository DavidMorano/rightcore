/* uc_getnameinfo */

/* interface component for UNIX® library-3c */
/* manipulate host-address entry structures */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutine is a cleaned up version of 'getnameinfo(3socket)'.


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


int uc_getnameinfo(sap,salen,hostbuf,hostlen,svcbuf,svclen,flags)
const struct sockaddr	*sap ;
int		salen ;
char		hostbuf[] ;
int		hostlen ;
char		svcbuf[] ;
int		svclen ;
int		flags ;
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		rc ;
	int		f_exit = FALSE ;

	if (sap == NULL) return SR_FAULT ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    rc = getnameinfo(sap,salen,hostbuf,hostlen,svcbuf,svclen,flags) ;
	    if (rc != 0) {
	        switch (rc) {
	        case EAI_ADDRFAMILY:
	            rs = SR_AFNOSUPPORT ;
		    f_exit = TRUE ;
	            break ;
	        case EAI_AGAIN:
	            if (to_again-- > 0) {
			msleep(1000) ;
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
#ifdef	EAI_OVERFLOW
	        case EAI_OVERFLOW:
	            rs = SR_OVERFLOW ;
		    f_exit = TRUE ;
	            break ;
#endif /* EAI_OVERFLOW */
	        case EAI_SYSTEM:
	            rs = (- errno) ;
	            switch (rs) {
	            case SR_AGAIN:
	                if (to_again-- > 0) {
			    msleep(1000) ;
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
/* end subroutine (uc_getnameinfo) */


