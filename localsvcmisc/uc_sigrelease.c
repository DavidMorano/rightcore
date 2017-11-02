/* uc_sigrelease */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We release a signal from the process (or thread) signal mask.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_sigrelease(int sn)
{
	int		rs ;

	if ((rs = sigrelse(sn)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_sigrelease) */


