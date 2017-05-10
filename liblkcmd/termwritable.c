/* termwritable */

/* determine is a terminal is writable */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We determine if the terminal specified is writable or not.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int termwritable(cchar *fname)
{
	struct ustat	sb ;
	int		rs ;
	int		n = 0 ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    if (sb.st_mode & S_IWGRP) {
	        n += 1 ;
	        if (sb.st_mode & S_IXUSR) {
	            n += 1 ;
	        }
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (termwritable) */


