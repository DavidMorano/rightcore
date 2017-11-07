/* timespec */
/* lang=C99 */

/* time-spec object methods */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Methods for the TIMESPEC object.


*******************************************************************************/


#ifndef	TIMESPEC_INCLUDE
#define	TIMESPEC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TIMESPEC
#define	TIMESPEC	struct timespec
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* exported subroutines */


#if	(! defined(TIMESPEC_MASTER)) || (TIMESPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int timespec_load(TIMESPEC *,time_t,long) ;

#ifdef	__cplusplus
}
#endif

#endif /* TIMESPEC_MASTER */

#endif /* TIMESPEC_INCLUDE */


