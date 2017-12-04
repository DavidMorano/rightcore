/* listenuss */

/* subroutine to listen on a UNIX® socket file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to initiate listening on a UNIX® domain
	socket stream (USS).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#if	CF_DEBUGS
#include	<debug.h>
#endif


/* local defines */

#define	LISTEN_NQUEUE	10


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int listenuss(cchar *portspec,int om,int lopts)
{
	const int	pf = PF_UNIX ;
	const int	st = SOCK_STREAM ;
	int		rs ;
	int		s = 0 ;

#if	CF_DEBUGS
	debugprintf("listenuss: fname=%s\n",portspec) ;
	debugprintf("listenuss: om=%04o\n",om) ;
#endif

	if (portspec == NULL) return SR_FAULT ;

	if (portspec[0] == '\0') return SR_INVALID ;

	if ((rs = u_socket(pf,st,0)) >= 0) {
	    s = rs ;

	    if (lopts & 1) {
	        const int	so = SO_REUSEADDR ;
	        const int	isize = sizeof(int) ;
	        int		one = 1 ;
	        rs = u_setsockopt(s,SOL_SOCKET,so,&one,isize) ;
	    }

	    if (rs >= 0) {
	        SOCKADDRESS	sa ;
	        const int	af = AF_UNIX ;
	        if ((rs = sockaddress_start(&sa,af,portspec,0,0)) >= 0) {
	            struct sockaddr	*sap = (struct sockaddr *) &sa ;
	            int			sal = rs ;

	            u_unlink(portspec) ;
	            if ((rs = u_bind(s,sap,sal)) >= 0) {
	                if ((rs = u_listen(s,LISTEN_NQUEUE)) >= 0) {
	                    om &= S_IAMB ;
	                    rs = u_chmod(portspec,om) ;
	                } /* end if (u_listen) */
	            } /* end if (u_bind) */

	            sockaddress_finish(&sa) ;
	        } /* end if (sockaddress) */
	    } /* end if (ok) */

	    if (rs < 0)
	        u_close(s) ;
	} /* end if (socket) */

#if	CF_DEBUGS
	debugprintf("listenuss: ret rs=%d s=%u\n",rs,s) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (listenuss) */


