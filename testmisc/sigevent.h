/* sigevent */
/* lang=C99 */

/* signal event object methods */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Methods for the signal-event object.


*******************************************************************************/


#ifndef	SIGEVENT_INCLUDE
#define	SIGEVENT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<signal.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SIGEVENT
#define	SIGEVENT	struct sigevent
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* exported subroutines */


#if	(! defined(SIGEVENT_MASTER)) || (SIGEVENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sigevent_load(SIGEVENT *,int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGEVENT_MASTER */

#endif /* SIGEVENT_INCLUDE */


