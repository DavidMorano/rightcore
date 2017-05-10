/* uc_sysconf */

/* interface component for UNIX® library-3c */
/* get system configuration information */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int uc_sysconf(int request,long *rp)
{
	long		result = 0 ;
	int		rs ;
	int		f_exit = FALSE ;

	repeat {
	    errno = 0 ;
	    rs = SR_OK ;
	    if ((result = sysconf(request)) < 0) {
	        rs = (errno != 0) ? (- errno) : SR_NOTSUP ;
	    }
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	if (rp != NULL) {
	    *rp = (rs >= 0) ? result : 0L ;
	}

	if (rs >= 0) rs = (result & INT_MAX) ;
	return rs ;
}
/* end subroutine (uc_sysconf) */


