/* uc_sigignore */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_sigignore(int sn)
{
	int		rs ;
	if ((rs = sigignore(sn)) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_sigignore) */


