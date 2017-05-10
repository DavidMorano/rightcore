/* u_exit */

/* straight hardcore exit! */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

#ifdef	COMMENT
extern void	_exit(int) ;
#endif


/* exported subroutines */


int u_exit(int ex)
{
	(void) _exit(ex) ;
	return SR_NOSYS ;
}
/* end subroutine (u_exit) */


