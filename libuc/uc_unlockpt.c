/* uc_unlockpt */

/* interface component for UNIX® library-3c */
/* unlock a PTS-type pseudo-terminal */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was adapted from a previously related subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine unlocks a PTS-type pseudo-terminal on System V Release 3
        (SVR3) UNIX® OS systems for use.


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


int uc_unlockpt(int fd)
{
	int		rs ;

	errno = 0 ;
	if ((rs = grantpt(fd)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_unlockpt) */


