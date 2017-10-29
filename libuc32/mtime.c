/* mtime */

/* Millisecond Time */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Sort of like |time(2)| but returns milliseconds rather than seconds.
	Unlike |time(2)|, this subroutine takes no arguments.


*******************************************************************************/


#include	<envstandards.h>
#include	<localmisc.h>

#include	"mtime.h"


mtime_t mtime(void)
{
	struct timeval	tv ;
	mtime_t		t ;
	mtime_t		m = 0 ;
	if (gettimeofday(&tv,NULL) >= 0) {
	    t = tv.tv_sec ;
	    m += (t*1000) ;
	    m += (tv.tv_usec / 1000) ;
	} else {
	    t = time(NULL) ; /* good until year 2038! */
	    m += (t*1000) ;
	}
	return m ;
}
/* end subroutine (mtime) */


