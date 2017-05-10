/* uc_mktime */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_mktime(tmp,rp)
struct tm	*tmp ;
time_t		*rp ;
{
	time_t	result ;

	int	rs = SR_OK ;


	if (tmp == NULL)
	    return SR_FAULT ;

	errno = 0 ;
	result = mktime(tmp) ;
	if ((result < 0) && (errno != 0)) rs = (- errno) ;

	if (rp != NULL)
	    *rp = (rs >= 0) ? result : 0 ;

#if	CF_DEBUGS
	debugprintf("uc_mktime: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_mktime) */


