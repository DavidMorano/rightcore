/* timespec */
/* lang=C99 */

/* UNIX® time specification initialization */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-10-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manipulate TIMESPEC objects.

	Synopsis:

	int timespec_init(TIMESPEC *tsp,time_t sec,long nsec)

	Arguments:

	tsp		pointer to TIMESPEC
	sec		seconds
	nsec		nanoseconds

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INTBILLION
#define	INTBILLION	1000000000
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int timespec_init(TIMESPEC *tsp,time_t sec,long nsec)
{
	if (tsp == NULL) return SR_FAULT ;
	while (nsec >= INTBILLION) {
	    sec += 1 ;
	    nsec -= INTBILLION ;
	}
	tsp->tv_sec = sec ;
	tsp->tv_nsec = nsec ;
	return SR_OK ;
}
/* end subroutine (timespec_init) */


