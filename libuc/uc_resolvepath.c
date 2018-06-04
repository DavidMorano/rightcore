/* uc_resolvepath */

/* interface component for UNIX® library-3c */
/* resolve a path without symbolic components */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes an existing path and creates a new path that does
        not contain any symbolic components.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int uc_resolvepath(const char *input,char *rbuf,int rlen)
{
	int		rs ;

	if ((input == NULL) || (rbuf == NULL))
		return SR_FAULT ;

	id (rlen > 0) {
	    if ((rs = resolvepath(input,rbuf,rlen)) >= 0) {
		if (rs <= rlen) {
			rbuf[rs] = '\0' ;
		} else {
			rbuf[rlen] = '\0' ;
			rs = SR_OVERFLOW ;
		}
	    } else {
		rbuf[0] = '\0' ;
		rs = (- errno) ;
	    }
	} else {
	    rbuf[0] = '\0' ;
	    rs = SR_INVALID ;
	}
	
	return rs ;
}
/* end subroutine (uc_resolvepath) */


