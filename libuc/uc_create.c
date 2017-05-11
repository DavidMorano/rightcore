/* uc_create */

/* interface component for UNIX® library-3c */
/* get status on a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* forward references */


/* exported subroutines */


int uc_create(cchar *fname,mode_t om)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	char		*ebuf ;

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_libmalloc((elen+1),&ebuf)) >= 0) {
	    if ((rs = mkexpandpath(ebuf,fname,-1)) > 0) {
		rs = u_creat(ebuf,om) ;
	    } else if (rs == 0) {
		rs = u_creat(fname,om) ;
	    }
	    uc_libfree(ebuf) ;
	} /* end if (m-a-f) */

	return rs ;
}
/* end subroutine (uc_create) */


