/* unixtime */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We return the UNIX time in a 64-bit number.

	Synopsis:
	int64_t unixtime(int64_t *rp)

	Arguemnts:
	rp		pointer to 64-bit value to store result or NULL

	Returns:
	-		64-bit UNIX time


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


#if	defined(_LP64)

unixtime_t unixtime(unixtime_t *rp)
{
	return time(rp) ;
}

#else /* defined(_LP64) */

unixtime_t unixtime(unixtime_t *rp)
{
	const time_t	t = time(NULL) ;
	uint64_t	uut ;
	unixtime_t	ut ;

	uut = (uint64_t) t ;
	ut = (unixtime_t) uut ;
	if (rp != NULL) {
	   *rp = ut ;
	}

	return ut ;
}
/* end subroutine (unixtime) */

#endif /* defined(_LP64) */


