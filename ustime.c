/* ustime */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


ustime_t ustime(void)
{
	ustime_t	ust = (ustime_t) time(NULL) ;
	return ust ;
}
/* end subroutine (ustime) */


