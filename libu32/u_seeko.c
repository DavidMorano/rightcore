/* u_seeko (seek-off) */

/* perform a seek and get the new offset also (like 'lseek(2)') */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int u_seeko(fd,wo,w,offp)
int		fd ;
offset_t	wo ;
int		w ;
offset_t	*offp ;
{
	offset_t	ro ;
	int		rs ;

	repeat {
	    rs = SR_OK ;
	    if ((ro = lseek(fd,wo,w)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	if (offp != NULL) {
	    *offp = (rs >= 0) ? ro : 0 ;
	}

	if (rs >= 0) rs = (ro & INT_MAX) ;
	return rs ;
}
/* end subroutine (u_seeko) */


int u_seekoff(fd,wo,w,offp)
int		fd ;
offset_t	wo ;
int		w ;
offset_t	*offp ;
{
	return u_seeko(fd,wo,w,offp) ;
}
/* end subroutine (u_seekoff) */


int u_oseek(fd,wo,w,offp)
int		fd ;
offset_t	wo ;
int		w ;
offset_t	*offp ;
{
	return u_seeko(fd,wo,w,offp) ;
}
/* end subroutine (u_oseek) */


