/* uc_setpriority */

/* interface component for UNIX® library-3c */
/* set a process priority (old style) */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine sets a process priority (the old style priority from the
        beginning days).


*******************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/resource.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int uc_setpriority(which,who,prio)
int	which ;
id_t	who ;
int	prio ;
{
	int	rs = SR_OK ;


again:
	errno = 0 ;
	rs = setpriority(which,who,prio) ;

	if ((rs == -1) && (errno != 0))
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_setpriority) */


