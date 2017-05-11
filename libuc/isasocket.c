/* isasocket */

/* test if a given file descriptor is a socket */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revistion history:

	= 1998-11-06, David A­D­ Morano
	Here is a little ditty to see if the FD that we have is a socket or
	not.

	= 2017-01-14, David A­D­ Morano
        I greatly simplified this by only using |getsockopt(3socket)| as the
        test. This still should properly work on systems that do not support
        S_ISSOCK for sockets (older Solaris®).

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is my own hack attempt at figuring out if the given FD is a socket
        or not. Wouldn't it be nice if a 'stat(2)' returned a file mode for
        sockets so that 'S_ISSOCK(mode)' was true? Oh, but
        NOOOOOOOOOOOOOOOOOOOOOO!!!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/conf.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isOneOf(const int *,int) ;


/* external variables */


/* local structures */


/* forward references */


static int isNotSock(int) ;


/* local variables */

static const int	rsock[] = {
	SR_NOTSOCK,
	SR_OPNOTSUPP,
	SR_NOTSUP,
	0
} ;


/* exported subroutines */


int isasocket(int fd)
{
	const int	slev = SOL_SOCKET ;
	const int	scmd = SO_TYPE ;
	int		rs ;
	int		len = sizeof(int) ;
	int		val = 0 ;
	int		f = FALSE ;
	if ((rs = u_getsockopt(fd,slev,scmd,&val,&len)) >= 0) {
	    f = TRUE ;
	} else if (isNotSock(rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (isasocket) */


/* local subroutines */


static int isNotSock(int rs)
{
	return isOneOf(rsock,rs) ;
}
/* end subroutine (isNotSock) */


