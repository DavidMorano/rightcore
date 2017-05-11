/* uc_killpg */

/* interface component for UNIX® library-3c */
/* send a kill-signal to a process group */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-11, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Send a signal to a process group.

	Synopsis:

	int uc_killpg(pgrp,signo)
	pid_t		pgrp ;
	int		signo ;

	Arguments:

	pgrp		program group ID
	signo		signal number

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_killpg(pid_t pgrp,int signo)
{
	int	rs ;

	repeat {
	    if ((rs = killpg(pgrp,signo)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (uc_killpg) */


