/* ut_bind */

/* UNIX® XTI subroutine */
/* XTI bind */


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

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int ut_bind(fd,req,ret)
int		fd ;
struct t_bind	*req, *ret ;
{
	int		rs ;
	int		to_nosr = TO_NOSR ;

again:
	rs = t_bind(fd,req,ret) ;

	if (rs < 0) {
	    switch (t_errno) {
	    case TBADF:
	        rs = SR_NOTSOCK ;
	        break ;
	    case TACCES:
	        rs = SR_ACCES ;
	        break ;
	    case TBADADDR:
	        rs = SR_DESTADDRREQ ;
	        break ;
	    case TBUFOVFLW:
	        rs = SR_OVERFLOW ;
	        break ;
	    case TNOADDR:
	        rs = SR_ADDRNOTAVAIL ;
	        break ;
	    case TOUTSTATE:
	        rs = SR_NOPROTOOPT ;
	        break ;
	    case TSYSERR:
	        rs = (- errno) ;
	        break ;
	    } /* end switch */
	} /* end if */

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOSR:
	        if (to_nosr-- > 0) goto retry ;
		break ;
	    case SR_INTR:
	        goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;

retry:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (ut_bind) */


