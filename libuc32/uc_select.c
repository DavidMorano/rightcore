/* uc_select */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INTR		0		/* do not return on an interrupt */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_select(nfds,readfds,writefds,errorfds,tp)
int		nfds ;
fd_set		*readfds ;
fd_set		*writefds ;
fd_set		*errorfds ;
struct timeval	*tp ;
{
	int	rs ;


	rs = select(nfds,readfds,writefds,errorfds,tp) ;
	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_select) */


