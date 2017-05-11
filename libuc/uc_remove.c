/* uc_remove */

/* interface component for UNIX® library-3c */
/* remove a file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-05-09, David A­D­ Morano
	This subroutine was writen for some reason.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine removes a file or a directory.

	Updated note:

	Like the 'rename(2)' call, this subroutine has nothing to do with STDIO
	either!  Some documentation thinks that it does (or did).  What is the
	facination with STDIO??  One would think that the sun rises and sets on
	it!

	Also, I have no idea why this subroutine is written out (implemented)
	here.  Yes, I wrote this but I am not sure why.  We must not have had
	'remove(3c)' on some platform back in the bad 'ole days somewhere.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* exported subroutines */


int uc_remove(cchar *fname)
{
	struct ustat	sb ;
	int		rs ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_stat(fname,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        rs = u_rmdir(fname) ;
	    } else {
	        rs = u_unlink(fname) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (uc_remove) */


