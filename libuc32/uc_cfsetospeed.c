/* uc_cfsetospeed */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_cfsetospeed(tp,speed)
struct termios	*tp ;
speed_t		speed ;
{
	int	rs ;


again:
	if ((rs = cfsetospeed(tp,speed)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_cfsetospeed) */


