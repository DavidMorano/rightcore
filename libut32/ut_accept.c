/* ut_accept */

/* UNIX® XTI subroutine */
/* XTI accept */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
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


int ut_accept(fd,resfd,callp)
int		fd ;
int		resfd ;
const struct t_call	*callp ;
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOMEM ;

#if	CF_DEBUGS
	debugprintf("ut_accept: ent fd=%d\n",fd) ;
#endif

again:
	if ((rs = t_accept(fd,resfd,callp)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (t_errno) {
	    case TBADF:
	        rs = SR_NOTSOCK ;
	        break ;
	    case TBADQLEN:
	        rs = SR_INVALID ;
	        break ;
	    case TBUFOVFLW:
	        rs = SR_OVERFLOW ;
	        break ;
	    case TLOOK:
	        rs = SR_LOOK ;
	        break ;
	    case TNODATA:
	        rs = SR_NODATA ;
	        break ;
	    case TNOTSUPPORT:
	        rs = SR_OPNOTSUPP ;
	        break ;

	    case TOUTSTATE:
	        rs = SR_NOPROTOOPT ;
	        break ;
	    case TPROTO:
	        rs = SR_PROTO ;
	        break ;
	    case TQFULL:
	        rs = SR_DQUOT;
	        break ;
	    case TACCES:
	        rs = SR_ACCES ;
	        break ;
	    case TBADADDR:
	        rs = SR_DESTADDRREQ ;
	        break ;
	    case TNOADDR:
	        rs = SR_ADDRNOTAVAIL ;
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
	    case SR_NOMEM:
	        if (to_nomem-- > 0) goto retry ;
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
/* end subroutine (ut_accept) */


