/* listenconn */

/* subroutine to listen on a mounted FIFO for passed file-descriptors */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little subroutine checks for or establishes (if possible) a
        mounted FIFO for listening for passed file descriptors. This is
        a common method for standing servers to receive new client
        connections.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>		/* for |utime(2)| */
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	O_FLAGS1	(O_RDWR | O_NONBLOCK)


/* external subroutines */

extern int	isNotAccess(int) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	mntcheck(cchar *,mode_t) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int listenconn(cchar *mntfname,mode_t om,int opts)
{
	int		rs ;
	int		fd = -1 ;

	if (mntfname == NULL) return SR_FAULT ;

	if (mntfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("listenconn: ent fname=%s\n",mntfname) ;
	debugprintf("listenconn: opts=%08ß\n",otps) ;
#endif

	if ((rs = mntcheck(mntfname,om)) >= 0) {
	    int		pipes[2] ;
	    if ((rs = u_pipe(pipes)) >= 0) {
	        int	cfd = pipes[1] ;	/* client-side */
	        fd = pipes[0] ;			/* server-side */
	        if ((rs = u_ioctl(cfd,I_PUSH,"connld")) >= 0) {
	            if ((rs = uc_fattach(cfd,mntfname)) >= 0) {
	                u_close(cfd) ;
	                cfd = -1 ;
	                rs = uc_closeonexec(fd,TRUE) ;
			if (rs < 0) {
	            	    uc_fdetach(mntfname) ;
			}
		    } /* end if (uc_fattach) */
	        } /* end if (u_ioctl) */
		if (rs < 0) {
		    u_close(fd) ;
		    fd = -1 ;
		    if (cfd >= 0) {
			u_close(cfd) ;
			cfd = -1 ;
		    }
		}
	    } /* end if (u_pipe) */
	} /* end if (mntcheck) */

#if	CF_DEBUGS
	debugprintf("listenconn: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenconn) */


/* local subroutines */


static int mntcheck(cchar *mntfname,mode_t om)
{
	const int	am = (R_OK|W_OK) ;
	int		rs ;
	int		f = FALSE ;
	if ((rs = uc_access(mntfname,am)) >= 0) {
	    f = TRUE ;
	} else if (rs == SR_NOENT) {
	    const int	of = (O_CREAT|O_RDWR) ;
	    if ((rs = uc_open(mntfname,of,om)) >= 0) {
	        USTAT		sb ;
		const int	fd = rs ;
	        if ((rs = u_fstat(fd,&sb)) >= 0) {
	            if ((sb.st_mode & S_IWOTH) == 0) {
	                u_fchmod(fd,om) ;
		    }
	        }
		u_close(fd) ;
	    } /* end if (uc_open) */
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mntcheck) */


