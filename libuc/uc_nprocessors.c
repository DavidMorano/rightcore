/* uc_nprocessors */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the number of CPUs:
	0. online, or
	1. configured, 
	on the current system.

	Synopsis:

	int uc_nprocessorsessors(w)
	int	w ;

	Arguments:

	w		which number of CPUs to return: 
				0=online
				1=configured

	Returns:

	<0		error (generally not-supported)
	>=0		the number of CPUs requested


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_nprocessors(int w)
{
	int		rs = SR_NOTSUP ;
	int		cmd = -1 ;
	int		n = 0 ;

	switch (w) {
	case 0:
#ifdef	_SC_NPROCESSORS_ONLN
	    rs = SR_OK ;
	    cmd = _SC_NPROCESSORS_ONLN ;
#endif /* _SC_NPROCESSORS_ONLN */
	    break ;
	case 1:
#ifdef	_SC_NPROCESSORS_CONF
	    rs = SR_OK ;
	    cmd = _SC_NPROCESSORS_CONF ;
#endif /* _SC_NPROCESSORS_CONF */
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
        } /* end switch */

	if (rs >= 0) {
	    long	rn ;
	    rs = uc_sysconf(cmd,&rn) ;
	    n = (rn & INT_MAX) ;
	    if (rs == SR_INVALID) rs = SR_NOTSUP ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uc_nprocessors) */


