/* listenpass */

/* subroutine to listen on a FIFO for pass-FD requests */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little subroutine checks for or establishes (if possible) a FIFO
        for listening for passed file descriptors. This is a common method for
        standing servers to receive new client connections.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VERSION		"0"
#define	NTRIES		1
#define	CONNTIMEOUT	120		/* seconds */
#define	BUFLEN		(8 * 1024)
#define	LINGERTIME	(3 * 6)		/* seconds */
#define	POLLTIMEOUT	200		/* milliseconds */
#define	O_FLAGS1	(O_RDWR | O_NONBLOCK)


/* external subroutines */

extern int	isNotAccess(int) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int listenpass(cchar *passfname,mode_t om,int opts)
{
	struct ustat	sb ;
	int		rs ;
	int		fd = -1 ;

	if (passfname == NULL) return SR_FAULT ;

	if (passfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("listenpass: fname=%s\n",passfname) ;
#endif

	if ((rs = uc_open(passfname,O_FLAGS1,om)) >= 0) {
	    fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if ((sb.st_mode & 002) == 0) {
	            u_fchmod(fd,om) ;
		}
	    } else {
	        u_close(fd) ;
	    }
	} else if (isNotPresent(rs)) {
	    u_unlink(passfname) ;
	    if ((rs = uc_mkfifo(passfname,om)) >= 0) {
	        rs = u_open(passfname,O_FLAGS1,om) ;
	        fd = rs ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("listenpass: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenpass) */


