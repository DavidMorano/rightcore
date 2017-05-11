/* uc_syspages */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_syspages(int w)
{
	long		rn = 0 ;
	int		rs = SR_NOTSUP ;
	int		cmd = -1 ;
	int		n = 0 ;

	switch (w) {
	case 0:
#ifdef	_SC_AVPHYS_PAGES
	    rs = SR_OK ;
	    cmd = _SC_AVPHYS_PAGES ;
#endif /* _SC_AVPHYS_PAGES */
	    break ;
	case 1:
#ifdef	_SC_PHYS_PAGES
	    rs = SR_OK ;
	    cmd = _SC_PHYS_PAGES ;
#endif /* _SC_PHYS_PAGES */
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
        } /* end switch */

	if (rs >= 0) {
	    rs = uc_sysconf(cmd,&rn) ;
	    n = (rn & INT_MAX) ;
	    if (rs == SR_INVALID) rs = SR_NOTSUP ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uc_syspages) */


