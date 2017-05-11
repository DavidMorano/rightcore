/* uc_ztime */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>


/* exported subroutines */


int uc_ztime(const time_t *tp,int z,struct tm *tsp)
{
	int	rs ;
	if (z) {
	    rs = uc_localtime(tp,tsp) ;
	} else {
	    rs = uc_gmtime(tp,tsp) ;
	}
	return rs ;
}
/* end subroutine (uc_ztime) */


