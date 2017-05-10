/* ut_sync */

/* UNIX® XTI subroutine */
/* XTI sync */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<xti.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	TO_NOMEM	5


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int ut_sync(int fd)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;

again:
	rs = t_sync(fd) ;

	if (rs < 0) {
	    switch (t_errno) {
	    case TBADF:
	        rs = SR_NOTSOCK ;
	        break ;
	    case TSYSERR:
	        rs = (- errno) ;
	        break ;
	    } /* end switch */
	} /* end if */

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	    case SR_NOSR:
	        if (to_nomem-- > 0) goto retry ;
		break ;
/* FALLTHROUGH */
	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;

retry:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (ut_sync) */


