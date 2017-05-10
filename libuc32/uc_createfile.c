/* uc_createfile */

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


int uc_createfile(cchar *fname,mode_t om)
{
	int	rs ;
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_FAULT ;
	if ((rs = uc_create(fname,om)) >= 0) {
	    rs = u_close(rs) ;
	}
	return rs ;
}
/* end subroutine (uc_createfile) */


