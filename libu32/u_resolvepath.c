/* u_resolvepath */

/* resolve a path without symbolic components */
/* translation layer interface for UNIX® equivalents */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine takes an existing path and creates a new path that does
	not contain any symbolic components.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int u_resolvepath(input,rbuf,rlen)
const char	input[] ;
char		rbuf[] ;
int		rlen ;
{
	int		rs ;

	if (input == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (input[0] == '\0') return SR_INVALID ;

	repeat {
	    errno = 0 ;
	    if ((rs = resolvepath(input,rbuf,rlen)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	if ((rs >= 0) && (rs <= rlen)) {
	    rbuf[rs] = '\0' ;
	}

	return rs ;
}
/* end subroutine (u_resolvepath) */


