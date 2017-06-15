/* uc_gmtime */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_gmtime(const time_t *tp,struct tm *tsp)
{
	struct tm	*rp ;
	int		rs = SR_OK ;

	if ((tp == NULL) || (tsp == NULL)) return SR_FAULT ;

	errno = 0 ;
#if	defined(SYSHAS_GMTIMER) && (SYSHAS_GMTIMER > 0)
	rp = (struct tm *) gmtime_r(tp,tsp) ;
	if (rp == NULL) rs = (- errno) ;
#else
	rp = gmtime(tp) ;
	if (rp == NULL) rs = (- errno) ;
	if (rs >= 0)
	    memcpy(tsp,rp,sizeof(struct tm)) ;
#endif 

#if	CF_DEBUGS
	debugprintf("uc_gmtime: year=%d isdst=%d\n",
		tsp->tm_year,tsp->tm_isdst) ;
	if (tsp->tm_isdst >= 0)
	    debugprintf("uc_gmtime: tzname=%s\n",
		tzname[tsp->tm_isdst]) ;
#endif

	return rs ;
}
/* end subroutine (uc_gmtime) */


