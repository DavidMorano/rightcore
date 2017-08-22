/* sysmemutil */

/* retrieve the utilization (in a percentage of total) of system memory */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-18, David A­D­ Morano
	This little subroutine was put together to get the system physical
	memory utilization in terms of a percentage of the total.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We hope that the OS has some 'sysconf(3c)' subroutines to help us here.
        We really are tired of searching through the kernel for this sort of
        information. We return SR_NOSYS if the OS does not provide an easy want
        to get this stuff.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sysmemutil.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sysmemutil(SYSMEMUTIL *mup)
{
	int		rs = SR_OK ;
	int		percent = 0 ;
	int		f = FALSE ;

#if	defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES)
	{
	    long	mt, ma ;
	    if ((rs = uc_sysconf(_SC_PHYS_PAGES,&mt)) >= 0) {
	        if ((rs = uc_sysconf(_SC_AVPHYS_PAGES,&ma)) >= 0) {
	    	    if (mt > 0) {
	        	ulong	mu100, mu ;
	        	mu = (mt - ma) ;
	        	mu100 = (mu * 100) ;
	        	percent = (mu100 / mt) ;
		        if (mup != NULL) {
			    memset(mup,0,sizeof(SYSMEMUTIL)) ;
			    mup->mt = mt ;
			    mup->ma = ma ;
			    mup->mu = percent ;
			}
		    } else {
			f = TRUE ;
		    }
		} /* end if (sysconf) */
	    } /* end if (sysconf) */

	} /* end if (memory usage) */
#else
	rs = SR_NOSYS ;
#endif /* defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES) */

	if (((rs < 0) || f) && (mup != NULL)) {
	    memset(mup,0,sizeof(SYSMEMUTIL)) ;
	}

	return (rs >= 0) ? percent : rs ;
}
/* end subroutine (sysmemutil) */


