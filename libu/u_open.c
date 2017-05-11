/* u_open */

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

extern int	u_closeonexec(int,int) ;

extern int	msleep(int) ;


/* exported subroutines */


int u_open(cchar *fname,int of,mode_t m)
{
	const int	f_cloexec = (of & O_CLOEXEC) ;
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		fd = 0 ;
	int		f_exit = FALSE ;

	of &= (~ OM_SPECIAL) ;

	repeat {
	    if ((rs = open(fname,of,m)) < 0) rs = (- errno) ;
	    fd = rs ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */
	        case SR_AGAIN:
	        case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	if ((rs >= 0) && f_cloexec) {
	    rs = u_closeonexec(fd,TRUE) ;
	    if (rs < 0) u_close(fd) ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (u_open) */


