/* uc_unlockpt */

/* interface component for UNIX® library-3c */
/* unlock a PTS-type pseudo-terminal */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was adapted from a previously related subroutine.

	= 2018-10-03, David A.D. Morano
	Fixed a bad error.

*/

/* Copyright © 1998,2018 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine unlocks a PTS-type pseudo-terminal on System V Release 3
        (SVR3) UNIX® OS systems for use.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_unlockpt(int fd)
{
	int		rs ;

	errno = 0 ;
	if ((rs = unlockpt(fd)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_unlockpt) */

