/* msleep */

/* millisecond sleep */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */


/* revision history:

	= 1998-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit-pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine sleeps for some number of milliseconds.

	Synopsis:

	int msleep(msec)
	uint	msec ;

	Arguments:

	msec		number of millisecond to sleep

	Returns:

	>=0		amount of data returned
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int msleep(uint msec)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msleep: msec=%u\n",msec) ;
#endif

	if (msec > 0) {
	    struct pollfd	fds[1] ;
	    const int		irs = SR_INTR ;
	    fds[0].fd = -1 ;
	    fds[0].events = 0 ;
	    if ((rs = u_poll(fds,0,msec)) == irs) {
		rs = 1 ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("msleep: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msleep) */


