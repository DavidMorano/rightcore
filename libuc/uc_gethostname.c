/* uc_gethostname */

/* interface component for UNIX® library-3c */
/* get the "hostname" (really the "nodename") of the current machine */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets the 'hostname' of the current machine.


*******************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int uc_gethostname(char *hbuf,int hlen)
{
	int		rs ;

	if (hbuf == NULL) return SR_FAULT ;

	hbuf[0] = '\0' ;
	if ((rs = gethostname(hbuf,(hlen+1))) < 0) rs = (- errno) ;

	if (rs >= 0) {
	    hbuf[hlen] = '\0' ;
	    rs = strlen(hbuf) ;
	}

	return rs ;
}
/* end subroutine (uc_gethostname) */


