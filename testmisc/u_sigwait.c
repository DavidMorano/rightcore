/* u_sigwait (POSIX® version) */

/* wait until the arrival of a signal (POSIX® version) */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Important note:

	This is the POSIX® version of this subroutine (not the earlier one)!

	Synopsis:

	int u_sigwait(const sigset_t *ssp,int *rp)

	Arguments:

	ssp		pointer to signal-set object
	rp		pointer to integer to hold result signal

	Returns:

	<0		error or interrupt
	>=0		signal which became pending


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_sigwait(const sigset_t *ssp,int *rp)
{
	int		rs ;
	int		sig ;

	if ((rs = sigwait(ssp,&sig)) < 0)
	    rs = (- errno) ;

	if (rp != NULL) *rp = sig ;

	sig &= INT_MAX ;
	return (rs >= 0) ? sig : rs ;
}
/* end subroutine (u_sigwait) */


