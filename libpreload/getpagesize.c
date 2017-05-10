/* getpagesize */

/* Get-Page-Size UNIX® System interposer */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a version of |getpagesize(3c)| that is preloaded to over-ride
	the standard UNIX® system version. We do this to basically just cache
	the value. The page-size of a machine does not generally change between
	reboots.

	Q. Is this multi-thread safe?
	A. Since it is a knock-off of an existing UNIX® system LIBC (3c)
	   subroutine that is already multi-thread safe -- then of course
	   it is!

	Q. Is this much slower than the default system version?
	A. No, not really.

	Q. How are we smarter than the default system version?
	A. Let me count the ways:
		+ value is cached!

	Q. Why did you not also interpose something for |sysconf(3c)|?
	A. Because we did not want to.

	Q. Why are you so smart?
	A. I do not know.

	Additional notes:

        + We do not really allow this subroutine to fail under any
        circumstances. We always return a value, even if the value is wrong.
        This seems like reasonable behavior for the time being. Do any programs
        ever expect this subroutine to fail?

        + On some modern UNIXi® this subroutine (|getpagesize(3c)| calls the
        lwoer level subroutine |sysconf(3c)|. That subroutine (|sysconf(3c)|,
        although it handles several different requests, often caches the value
        returned for the "get-page-size" request.  So caching the result
	here only saves an extra subroutine calls on those UNIXi® which cache
	it at the lower level.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	DEFPAGESIZE	(8*1024)
#define	GETPAGESIZE	struct getpagesize_head


/* external subroutines */

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct getpagesize_head {
	int		ps ;
} ;


/* forward references */


/* local variables */

static GETPAGESIZE	getpagesize_data ; /* zero-initialized */


/* exported subroutines */


int getpagesize(void)
{
	GETPAGESIZE	*psp = &getpagesize_data ;
	int		rs = SR_OK ;
	if (psp->ps == 0) {
	    const int	cmd = _SC_PAGESIZE ;
	    if ((rs = uc_sysconf(cmd,NULL)) >= 0) {
	        psp->ps = rs ;
	    } else {
	        psp->ps = DEFPAGESIZE ;
		rs = psp->ps ;
	    }
	} else {
	    rs = psp->ps ;
	}
	return rs ;
}
/* end subroutine (getpagesize) */


