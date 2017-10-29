/* u_pipe */

/* UNIX® kernel interface */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services (RNS).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Notes:

	In the old days (or on many BSD type UNIX® systems) the
	twp resulting pipe ends were unidirectional as follows:

	end	access
	---------------
	0	reading
	1	writing

	With the advent of SYSV UNIX® both ends are bidirectional.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	TO_NOMEM	10
#define	TO_NOSR		(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_pipe(p)
int	p[2] ;
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;
	int	to_nosr = TO_NOSR ;


again:
	if ((rs = pipe(p)) < 0) 
	    rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {

	    case SR_NOMEM:
	        if (to_nomem-- > 0) goto retry ;
	        break ;

#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	    case SR_NOSR:
	        if (to_nosr-- > 0) goto retry ;
	        break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */

	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;

	    } /* end switch */
	} /* end if */

	return rs ;

retry:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (u_pipe) */



