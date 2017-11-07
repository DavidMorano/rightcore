/* itimerspec */
/* lang=C99 */

/* interval-timer object methods */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Methods for the ITIMERSPEC object.


*******************************************************************************/


#ifndef	ITIMERSPEC_INCLUDE
#define	ITIMERSPEC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<time.h>
#include	<timespec.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ITIMERSPEC
#define	ITIMERSPEC	struct itimerspec
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* exported subroutines */


#if	(! defined(ITIMERSPEC_MASTER)) || (ITIMERSPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int itimerspec_load(ITIMERSPEC *,TIMESPEC *,TIMESPEC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ITIMERSPEC_MASTER */

#endif /* ITIMERSPEC_INCLUDE */


