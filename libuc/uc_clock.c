/* uc_clock */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	The UNIX® System "clock" subroutines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>


/* external subroutines */


/* external variables */


/* local defines */


/* forward references */


/* exported subroutines */


int uc_clockset(clockid_t cid,const struct timespec *tsp)
{
	int		rs ;
	if (tsp == NULL) return SR_FAULT ;
	if ((rs = clock_settime(cid,tsp)) < 0) rs = (-errno) ;
	return rs ;
}
/* end subroutine (uc_clockset) */


int uc_clockres(clockid_t cid,struct timespec *tsp)
{
	int		rs ;
	if (tsp == NULL) return SR_FAULT ;
	if ((rs = clock_getres(cid,tsp)) < 0) rs = (-errno) ;
	return rs ;
}
/* end subroutine (uc_clockres) */


int uc_clockget(clockid_t cid,struct timespec *tsp)
{
	int		rs ;
	if (tsp == NULL) return SR_FAULT ;
	if ((rs = clock_gettime(cid,tsp)) < 0) rs = (-errno) ;
	return rs ;
}
/* end subroutine (uc_clockget) */


