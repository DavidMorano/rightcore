/* itimerspec */
/* lang=C99 */

/* UNIX® ITIMERSPEC object initialization */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manipulate ITIMERSPEC objects.

	Synopsis:

	int itimerspec_load(ITIMERSPEC *tsp,time_t sec,long nsec)

	Arguments:

	tsp		pointer to ITIMERSPEC
	sec		seconds
	nsec		nanoseconds

	Returns:

	<0		error
	>=0		OK

	Comments:

	typedef struct itimerspec {		
		struct timespec	it_interval;	
		struct timespec	it_value;	
	} itimerspec_t ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"itimerspec.h"


/* local defines */

#ifndef	INTBILLION
#define	INTBILLION	1000000000
#endif

#ifndef	ITIMERSPEC
#define	ITIMERSPEC	struct itimerspec
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int itimerspec_load(ITIMERSPEC *tsp,TIMESPEC *valp,TIMESPEC *intp)
{
	memset(tsp,0,sizeof(ITIMERSPEC)) ;
	if (valp != NULL) {
	    tsp->it_value = *valp ;
	}
	if (intp != NULL) {
	    tsp->it_interval = *intp ;
	}
	return SR_OK ;
}
/* end subroutine (itimerspec_load) */


