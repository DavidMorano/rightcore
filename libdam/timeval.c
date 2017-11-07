/* timeval */

/* subroutines to manipulate TIMEVAL values */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

	= 2014-03-24, David A­D­ Morano
	I added the |init()| method.

*/

/* Copyright © 1998,2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage (a little bit) the TIMEVAL object.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	INTMILLION
#define	INTMILLION	1000000
#endif

#ifndef	TIMEVAL
#define	TIMEVAL		struct timeval
#endif


/* exported subroutines */


int timeval_load(TIMEVAL *dst,time_t sec,int usec)
{
	if (dst == NULL) return SR_FAULT ;
	while (usec >= INTMILLION) {
	    sec += 1 ;
	    usec -= INTMILLION ;
	}
	dst->tv_sec = sec ;
	dst->tv_usec = usec ;
	return SR_OK ;
}
/* end subroutine (timeval_load) */


int timeval_add(TIMEVAL *dst,TIMEVAL *src1,TIMEVAL *src2)
{
	int		f = FALSE ;

	dst->tv_sec = src1->tv_sec + src2->tv_sec ;
	dst->tv_usec = src1->tv_usec + src2->tv_usec ;
	if (dst->tv_usec >= 1000000) {
	    f = TRUE ;
	    dst->tv_usec -= 1000000 ;
	    dst->tv_sec += 1 ;
	}

	return f ;
}
/* end subroutine (timeval_add) */


int timeval_sub(TIMEVAL *dst,TIMEVAL *src1,TIMEVAL *src2)
{
	int		f = FALSE ;

	dst->tv_sec = src1->tv_sec - src2->tv_sec ;
	dst->tv_usec = src1->tv_usec - src2->tv_usec ;
	if (dst->tv_usec < 0) {
	    f = TRUE ;
	    dst->tv_usec += 1000000 ;
	    dst->tv_sec -= 1 ;
	}

	return f ;
}
/* end subroutine (timeval_sub) */


