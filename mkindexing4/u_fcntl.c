/* u_fcntl */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_INTR		5


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_fcntl(int fd,int cmd,...)
{
	caddr_t		any ;
	int		rs ;
	int		to_intr = TO_INTR ;
	int		f_exit = FALSE ;

	{
	va_list	ap ;
	va_begin(ap,cmd) ;
	any = va_arg(ap,caddr_t) ;
	va_end(ap) ;
	}

	repeat {
	    if ((rs = fcntl(fd,cmd,any)) == -1) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_INTR:
	            if (to_intr-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	if (rs == SR_ACCESS) {
	    switch (cmd) {
	    case F_GETLK:
	    case F_SETLK:
	    case F_SETLKW:
#if	defined(LARGEFILE64_SOURCE) && (LARGEFILE64_SOURCE == 1)
	    case F_GETLK64:
	    case F_SETLK64:
	    case F_SETLKW64:
#endif
	        rs = SR_AGAIN ;
		break ;
	    } /* end switch */
	} /* end if (old mess-up) */

	return rs ;
}
/* end subroutine (u_fcntl) */


