/* dialuss */

/* subroutine to dial over to a UNIX® domain socket in stream mode */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Dial UNIX® Socket Stream (dialuss)

        This subroutine will dial out to an UNIX® domain socket stream address.

	Synopsis:

	int dialuss(pathname,to,options)
	const char	pathname[] ;
	int		to ;
	int		options ;

	Arguments:

	pathname	path to UNIX® domain socket to dial to
	to		to ('>=0' mean use one, '-1' means don't)
	options		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
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

#include	"dialopts.h"


/* local defines */


/* external subroutines */

extern int	opensockaddr(int,int,int,struct sockaddr *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dialuss(cchar *pathname,int to,int opts)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

	if (pathname == NULL) return SR_FAULT ;

	if (pathname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialuss: ent pathname=%s\n",pathname) ;
#endif

	if ((rs = uc_stat(pathname,&sb)) >= 0) {
	    if (S_ISSOCK(sb.st_mode)) {
		SOCKADDRESS	sa ;
		const int	af = AF_UNIX ;
	        const void	*vp = (const void *) pathname ;
	        if ((rs = sockaddress_start(&sa,af,vp,0,0)) >= 0) {
	            SOCKADDR	*sap = (SOCKADDR *) &sa ;
		    const int	pf = PF_UNIX ;
		    const int	st = SOCK_STREAM ;
		    const int	proto = 0 ;

		    if ((rs = opensockaddr(pf,st,proto,sap,to)) >= 0) {
	                fd = rs ;
			rs = dialopts(fd,opts) ;
		    }

#if	CF_DEBUGS
		    debugprintf("dialuss: opensockaddr()-out rs=%d\n",rs) ;
#endif

	            rs1 = sockaddress_finish(&sa) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (sockaddress) */
		if ((rs < 0) && (fd >= 0)) {
		    u_close(fd) ;
		    fd = -1 ;
		}
	    } else {
		rs = SR_NOTSOCK ;
	    } /* end if */
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("dialuss: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialuss) */


