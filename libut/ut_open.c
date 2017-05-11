/* ut_open */

/* UNIX® XTI subroutine */
/* XTI open */


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


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int ut_open(fname,f,ip)
const char	fname[] ;
int		f ;
struct t_info	*ip ;
{
	int		rs ;

again:
	rs = t_open(fname,f,ip) ;

	if (rs < 0) {
	    switch (t_errno) {
	    case TBADFLAG:
	        rs = SR_INVALID ;
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
	        msleep(1000) ;
/* FALLTHROUGH */
	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ut_open) */


