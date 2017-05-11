/* uc_grantpt */

/* interface component for UNIX® library-3c */
/* grant a PTS-type pseudo-terminal the required permissions for use */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was adapted from a previously related subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Grant the use of a pseudo-terminal on System V Release 3 (SVR3) UNIX® OS
        systems.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mkdev.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ptms.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_grantpt(fd)
int	fd ;
{
	int	rs ;


	errno = 0 ;
	if ((rs = grantpt(fd)) < 0) rs = (- errno) ;

ret0:
	return rs ;
}
/* end subroutine (uc_grantpt) */


