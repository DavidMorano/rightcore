/* uc_raise */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int uc_raise(int sig)
{
	int	rs ;


	if ((rs = raise(sig)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_raise) */


