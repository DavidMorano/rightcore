/* uc_piper */

/* interface component for UNIX® library-3c */
/* create pipes but moved up to a minimum FD number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-11, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates pipes but at or above a minimum FD
	number.

	Synopsis:

	int uc_piper(pipes,minfd)
	int	pipes[2] ;
	int	minfd ;

	Arguments:

	pipes		resulting array of two pipes
	minfd		minimum number where new FD will move to

	Returns:

	>=0		the new FD
	<0		error


	Notes:

	In the old days (or on many BSD type UNIX® systems) the
	two resulting pipe ends were unidirectional as follows:

	end	access
	---------------
	0	open for reading
	1	open for writing

	With the advent of SYSV UNIX® pipes are completely
	bidirectional with both ends suitable for reading and writing.
	Data written to either end is available for reading from
	the opposite end.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_piper(int *pipes,int minfd)
{
	int		rs ;

	if (pipes == NULL) return SR_FAULT ;

	if (((rs = u_pipe(pipes)) >= 0) && (minfd >= 1)) {
	    int	nfd ;
	    int	i ;
	    for (i = 0 ; (rs >= 0) && (i < 2) ; i += 1) {
	        if (pipes[i] < minfd) {
	            rs = uc_moveup(pipes[i],minfd) ;
	            nfd = rs ;
	            if (rs >= 0) pipes[i] = nfd ;
	        }
	    } /* end for */
	    if (rs < 0) {
	        u_close(pipes[0]) ;
	        u_close(pipes[1]) ;
	    }
	} /* end if (created successfully) */

	return rs ;
}
/* end subroutine (uc_piper) */


