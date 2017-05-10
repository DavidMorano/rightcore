/* uc_getpriority */

/* interface component for UNIX® library-3c */
/* get a process priority (old style) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine gets a process priority (the old style priority from the
        beginning days).


*******************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/resource.h>	/* 'getpriority(2)' */
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int uc_getpriority(which,who,rp)
int	which ;
id_t	who ;
int	*rp ;
{
	int	rs ;
	int	prio ;


again:
	rs = SR_OK ;
	errno = 0 ;
	prio = getpriority(which,who) ;

	if ((prio == -1) && (errno != 0))
	    rs = (- errno) ;

	if (rp != NULL)
	    *rp = prio ;

	return rs ;
}
/* end subroutine (uc_getpriority) */


