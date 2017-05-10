/* unixtime */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int64_t unixtime(int64_t *rp)
{
	uint64_t	uut ;
	int64_t		ut ;
	time_t		dt = time(NULL) ;

	uut = (uint64_t) dt ;
	ut = (int64_t) uut ;
	if (rp != NULL) {
	   *rp = ut ;
	}

	return ut ;
}
/* end subroutine (unixtime) */


