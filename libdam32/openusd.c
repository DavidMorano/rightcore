/* openusd */

/* open UNIX®-Socket-Datagram */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens a UNIX® socket of the datagram variety.

	Synopsis:

	int openusd(cchar *sfn,int of,mode_t om)

	Arguments:

	sfn		socket file (in the UNIX® file-system)
	of		open-mode (O_CREAT will create the socket)
	om		permissions on socket-file

	Returns:

	<0		error
	>=0		file-descriptor


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	uc_joinus(int,struct sockaddr *,int,int,mode_t) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int openusd(cchar *sfn,int of,mode_t om)
{
	int		rs ;
	int		fd = -1 ;
	const int	pf = PF_UNIX ;
	const int	st = SOCK_DGRAM ;
	if (sfn == NULL) return SR_FAULT ;
	if (sfn[0] == '\0') return SR_INVALID ;
#if	CF_DEBUGS
	debugprintf("sesnotes/openusd: ent sfn=%s\n",sfn) ;
#endif
	if ((rs = u_socket(pf,st,0)) >= 0) {
	    SOCKADDRESS	sa ;
	    const int	af = AF_UNIX ;
	    fd = rs ;
	    if ((rs = sockaddress_start(&sa,af,sfn,0,0)) >= 0) {
		SOCKADDR	*sap = (SOCKADDR *) &sa ;
		const int	sal = rs ;
		{
		    rs = uc_joinus(fd,sap,sal,of,om) ;
		}
	  	sockaddress_finish(&sa) ;
	    } /* end if (sockaddress) */
	    if (rs < 0) {
	        u_close(fd) ;
		fd = -1 ;
	    }
	} /* end if (socket) */
#if	CF_DEBUGS
	debugprintf("sesnotes/openusd: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openusd) */


