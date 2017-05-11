/* uc_safesleep */

/* interface component for UNIX® library-3c */
/* safely sleep for a while */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was written so that we could use a single directory
	interface due to all of the changes in the POSIX standard and previous
	UNIX® standards.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code safely sleeps for a while without interferring with the
	dangerous and fragile ALARM signal (which likely gets changed to
	different things under some execution modes -- like with mutlithreading
	on Slowlaris!).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<mtime.h>
#include	<localmisc.h>


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward referecnces */


/* local variables */


/* exported subroutines */


int uc_safesleep(int n)
{
	int		rs = SR_OK ;

	if (n > 0) {
	    struct pollfd	fds[2] ;
	    mtime_t		mn = (n*1000) ;
	    mtime_t		dt = mtime() ;
	    mtime_t		st ;

	    fds[0].fd = -1 ;
	    fds[0].events = 0 ;
	    fds[0].revents = 0 ;

	    st = dt ;
	    while ((rs >= 0) && ((dt-st) < mn)) {
	        int	intpoll = (mn-(dt-st)) ;
	        rs = u_poll(fds,0,intpoll) ;
	        dt = mtime() ;
		if (rs == SR_INTR) rs = SR_OK ;
	    } /* end while */

	} /* end if (positive) */

	return rs ;
}
/* end subroutine (uc_safesleep) */


