/* opensockaddr */

/* open a connection to a socket by a SOCKADDR */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We open a connection to something by a SOCKADDR.

	Synopsis:

	int opensockaddr(pf,st,proto,sap,to)
	int		pf ;
	int		st ;
	int		proto ;
	struct sockaddr	*sap ;
	int		to ;
	
	Arguments:

	pf		protocol family
	st		socket type
	proto		protocol
	sap		socket-address
	to		time-out

	Returns:

	<0		error in dialing
	>=0		file descriptor


	What is up with the 'to' argument?

	<0              use the system default to (xx minutes --
			whatever)
	==0             asynchronously span the connect and do not wait
			for it
	>0		use the to as it is


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensockaddr(int pf,int st,int proto,struct sockaddr *sap,int to)
{
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("opensockaddr: pf=%u type=%u proto=%u\n",
	    pf,st,proto) ;
#endif

	if ((rs = u_socket(pf,st,proto)) >= 0) {
	    SOCKADDRESS	*saddrp = (SOCKADDRESS *) sap ;
	    fd = rs ;

	    if ((rs = sockaddress_getlen(saddrp)) > 0) {
	        const int	sal = rs ;
	        if (to >= 0) {
	            rs = uc_connecte(fd,sap,sal,to) ;
	        } else {
	            rs = u_connect(fd,sap,sal) ;
	        }
	    }

	    if (rs < 0)
	        u_close(fd) ;
	} /* end if (u_socket) */

#if	CF_DEBUGS
	debugprintf("opensockaddr: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensockaddr) */


