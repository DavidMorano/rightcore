/* u_dup2 */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_MFILE	60
#define	TO_NFILE	60
#define	TO_NOSR		(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_dup2(int fd1,int fd2)
{
	int		rs ;
	int		to_mfile = TO_MFILE ;
	int		to_nfile = TO_NFILE ;
	int		to_nosr = TO_NOSR ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = dup2(fd1,fd2)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_MFILE:
	            if (to_mfile-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
		case SR_NFILE:
	            if (to_nfile-- > 0) {
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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_dup2) */


