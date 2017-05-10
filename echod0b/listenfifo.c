/* listenfifo */

/* subroutine to listen on a FIFO for pass-FD requests */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little subroutine checks for or establishes (if possible)
	a FIFO for listening to passed file descriptors.  This is a
	common method for standing servers to receive new client
	connections.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>

#include	"localmisc.h"



/* local defines */

#define	BUFLEN		(8 * 1024)
#define	O_FLAGS1	(O_RDWR | O_NONBLOCK)



/* external subroutines */

#if	CF_DEBUGS
extern char	*d_reventstr(int,char *,int) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */






int listenfifo(passfname,options)
const char	passfname[] ;
int		options ;
{
	struct ustat	sb ;

	int	rs ;
	int	fd ;


	if (passfname == NULL)
	    return SR_FAULT ;

	if (passfname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("listenfifo: fname=%s\n",passfname) ;
#endif

	rs = u_open(passfname,O_FLAGS1,0622) ;
	fd = rs ;
	if (rs >= 0) {

	    rs = u_fstat(fd,&sb) ;

	    if ((rs < 0) || 
	        ((! S_ISFIFO(sb.st_mode)) && (! S_ISCHR(sb.st_mode)))) {

	        rs = SR_INVALID ;
	        u_close(fd) ;

	    } else {

	        if ((sb.st_mode & 002) == 0)
	            (void) u_fchmod(fd,0622) ;

	    }
	}

#if	CF_DEBUGS
	debugprintf("listenfifo: between rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    (void) u_unlink(passfname) ;

	    rs = uc_mkfifo(passfname,0622) ;

#if	CF_DEBUGS
	    debugprintf("listenfifo: uc_mkfifo() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = u_open(passfname,O_FLAGS1,0622) ;
	        fd = rs ;
	    }
	}

/* I think that we have it! */
done:

#if	CF_DEBUGS
	debugprintf("listenfifo: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (listenfifo) */



/* LOCAL SUBROUTINES */




