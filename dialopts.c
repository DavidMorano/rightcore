/* dialopts */

/* set options on dialer file-descriptor */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define CF_LINGER	1		/* set socket to linger */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Set some options on a socket.

	Synopsis:

	int dialopts(fd,opts)
	int		fd ;
	int		opts ;

	Arguments:

	fd		file-descriptor
	opts		options

	Returns:

	<0		error in dialing
	>=0		file descriptor


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dialopts.h"


/* local defines */

#define	LINGERTIME	(2*60)		/* seconds */


/* external subroutines */

extern int	uc_linger(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dialopts(int fd,int opts)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("dialopts: ent fd=%d opts=\\b%04ß\n",fd,opts) ;
#endif

	if (fd >= 0) {

	    if ((rs >= 0) && (opts & DIALOPT_KEEPALIVE)) {
	        const int	size = sizeof(int) ;
	        const int	sol = SOL_SOCKET ;
	        const int	cmd = SO_KEEPALIVE ;
	        int		one = 1 ;
	        int		*onep ;
	        onep = &one ;
	        rs = u_setsockopt(fd,sol,cmd,onep,size) ;
	    }

#if	CF_DEBUGS
	    debugprintf("dialtcp/setopts: mid1 rs=%d\n",rs) ;
#endif

#if	CF_LINGER
	    if (rs >= 0) {
	        const int	to = (opts & DIALOPT_LINGER) ? LINGERTIME : 30 ;
	        rs = uc_linger(fd,to) ;
	    } /* end if (linger) */
#endif /* CF_LINGER */

	} else {
	    rs = SR_NOTOPEN ;
	}

#if	CF_DEBUGS
	debugprintf("dialopts: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dialopts) */


