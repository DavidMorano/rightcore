/* u_sysinfo */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include	<limits.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external variables */


/* exported subroutines */


int u_sysinfo(int request,char *ubuf,int ulen)
{
	long		result ;
	long		llen = (ulen+1) ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (ubuf == NULL) return SR_FAULT ;

	if (ulen < 1) return SR_OVERFLOW ;

	if ((result = sysinfo(request,ubuf,llen)) < 0) rs = (- errno) ;
	if (result > 0) len = (int) ((result-1) & INT_MAX) ;

	if ((rs >= 0) && (result > llen)) rs = SR_OVERFLOW ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (u_sysinfo) */


