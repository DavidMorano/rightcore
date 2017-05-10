/* ut_alloc */

/* UNIX® XTI subroutine */
/* XTI allocate */


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


int ut_alloc(fd,stype,fields,rpp)
int	fd ;
int	stype ;
int	fields ;
void	**rpp ;
{
	int		rs = SR_OK ;
	void		*p ;

	if (rpp == NULL) return SR_FAULT ;

again:
	p = t_alloc(fd,stype,fields) ;

	if (p == NULL) {
	    switch (t_errno) {
	    case TBADF:
	        rs = SR_NOTSOCK ;
		break ;
	    case TSYSERR:
	        rs = (- errno) ;
		break ;
	    default:
		rs = SR_NOANODE ;
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

	*rpp = (rs >= 0) ? p : NULL ;

	return rs ;
}
/* end subroutine (ut_alloc) */


