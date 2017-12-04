/* acceptfifo */

/* subroutine to accept an FD that was passed through a FIFO */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This little subroutine accepts a file descriptor from some type of FIFO,
        pipe, or other (weirdo) STREAMS-like thing.

	Synopsis:

	int acceptfifo(ffd,sp)
	int			ffd ;
	struct strrecvfd	*sp ;

	Arguments:

	ffd		FIFO file descriptor
	sp		pointer to structure to receive the passed FD

	Returns:

	>=0		reeived file descriptot
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int acceptfifo(ffd,sp)
int			ffd ;
struct strrecvfd	*sp ;
{
	struct strrecvfd	extra ;

	int	rs ;
	int	pfd ;


#if	CF_DEBUGS
	debugprintf("acceptfifo: ent FFD=%d\n",ffd) ;
#endif

/* look up some miscellaneous stuff in various databases */

	if (sp == NULL)
	    sp = &extra ;

again:
	rs = u_ioctl(ffd,I_RECVFD,sp) ;

#if	CF_DEBUGS
	debugprintf("acceptfifo: u_ioctl() rs=%d PFD=%d\n",rs,sp->fd) ;
#endif

	pfd = sp->fd ;
	if (rs == SR_BADMSG)
	    goto again ;

done:
	return (rs >= 0) ? pfd : rs ;
}
/* end subroutine (acceptfifo) */



