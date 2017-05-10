/* uc_fsize */

/* translation layer interface for UNIX® equivalents */
/* get the size of an open file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_fsize(int fd)
{
	struct ustat	sb ;
	int		rs ;
	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    rs = (sb.st_size & INT_MAX) ;
	}
	return rs ;
}
/* end subroutine (uc_fsize) */


