/* uc_gethz */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the HZ value of the system.

	Synopsis:

	int uc_gethz(int w)

	Arguments:

	w		specifies which source:
				0 -> all
				1 -> 'HZ' define only
				2 -> 'CLK_TCK' define only
				3 -> |sysconf(3c)| 'CLK_TCK' only

	Returns:

	<0		error (generally not-supported)
	>=0		value of system HZ


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_gethz(int w)
{
	long	hz = -1 ;
	int	rs = SR_NOSYS ;

#if	defined(HZ)
	if ((hz < 0) && ((w == 0) || (w == 1))) {
	    rs = SR_OK ;
	    hz = HZ ;
	}
#endif
#if	defined(CLK_TCK) 
	if ((hz < 0) && ((w == 0) || (w == 2))) {
	    rs = SR_OK ;
	    hz = CLK_TCK ;
	}
#endif
#if	defined(_SC_CLK_TCK) 
	if ((hz < 0) && ((w == 0) || (w == 3))) {
	    rs = uc_sysconf(_SC_CLK_TCK,&hz) ;
	}
#endif

	if (rs >= 0) rs = (hz & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("uc_gethz: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethz) */


