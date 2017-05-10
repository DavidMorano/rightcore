/* uc_fattach */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>


/* forward references */


/* exported subroutines */


int uc_fattach(fd,fname)
int		fd ;
const char	fname[] ;
{
	int	rs ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

again:
	if ((rs = fattach(fd,fname)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_fattach) */


