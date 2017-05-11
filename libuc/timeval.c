/* timeval */

/* subroutines to manipulate 'timeval' values */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>
#include	<time.h>

#include	<localmisc.h>


/* exported subroutines */


int timeval_add(dst,src1,src2)
struct timeval	*src1, *src2, *dst ;
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


int timeval_sub(dst,src1,src2)
struct timeval	*src1, *src2, *dst ;
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


