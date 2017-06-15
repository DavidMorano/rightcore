/* u_pread */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOLCK	10


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_pread(int fd,void *buf,int len,offset_t offset)
{
	int		rs ;
	int		to_nolck = TO_NOLCK ;
	int		f_exit = FALSE ;
	char		*bp = (char *) buf ;

#if	CF_DEBUGS
	debugprintf("u_pread: ent fd=%d len=%d\n",fd,len) ;
#endif

	repeat {
	    if ((rs = pread(fd,bp,(size_t) len,offset)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOLCK:
	            if (to_nolck-- > 0) {
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
/* end subroutine (u_pread) */


