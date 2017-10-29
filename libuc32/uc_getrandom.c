/* uc_getrandom */

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
#include	<sys/random.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int uc_getrandom(void *rbuf,int rlen,int fl)
{
	int		rs ;

	repeat {
	    errno = 0 ;
	    if ((rs = getrandom(rbuf,rlen,fl)) < 0) {
	        rs = (errno != 0) ? (- errno) : SR_NOTSUP ;
	    }
	} until ((rs != SR_INTR) && (rs != SR_AGAIN)) ;

	return rs ;
}
/* end subroutine (uc_getrandom) */


int uc_getentropy(void *rbuf,int rlen)
{
	return uc_getrandom(rbuf,rlen,0) ;
}
/* end subroutine (uc_getentropy) */


