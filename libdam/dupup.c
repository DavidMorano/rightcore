/* dupup */

/* duplicate a file descriptor to be at some minimum number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine will duplicate a file desciptor but the new file
        descriptor is guaranteed to be at or above some minimum number. This
        sort of thing is required in a number of situations where file
        descriptors need to be juggled around (like around when programs do
        forks!).

	Synopsis:

	int dupup(fd,min)
	int	fd ;
	int	min ;

	Arguments:

	fd	file desciptor to duplicate
	min	minimum level to get the duplicate at

	Returns:

	<0	error
	>=0	the new file descriptor


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* exported subroutines */


int dupup(int fd,int min)
{
	int		rs ;
	int		ufd = -1 ;

	if (fd < 0) return SR_INVALID ;

	if ((rs = u_dup(fd)) >= 0) {
	    ufd = rs ;
	    if (ufd < min) {
	        rs = uc_moveup(ufd,min) ;
	        ufd = rs ;
	        if (rs < 0) u_close(ufd) ;
	    } /* end if (move needed) */
	} /* end if (dup) */

	return (rs >= 0) ? ufd : rs ;
}
/* end subroutine (dupup) */



