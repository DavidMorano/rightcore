/* u_creat */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOMEM	1000
#define	TO_NOSR		1000


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_creat(cchar *fname,mode_t m)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = creat(fname,m)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_AGAIN:
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    }
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_creat) */


