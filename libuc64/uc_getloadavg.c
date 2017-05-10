/* uc_getloadavg */

/* interface component for UNIX® library-3c */
/* get load averages as floating doubles (relatively now standard!) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine gets the system load averages as double-floats! This is
        pretty much totally contrary to how they have ever been gotten before on
        any platform! Yes, the load averages are still just 32-bit integer
        quantities inside the kernel but the stupid new interface specified that
        the averages should be returned as double-floats. Yes, it is stupid but
        that is the way it works with these stupid interfaces!


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#undef	LOCAL_DARWIN
#define	LOCAL_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>

#if	LOCAL_SOLARIS
#include	<sys/loadavg.h>
#elif	LOCAL_DARWIN
#include	<stdlib.h>
#endif

#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */


/* exported subroutines */


int uc_getloadavg(double *la,int n)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = getloadavg(la,n)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getloadavg) */


