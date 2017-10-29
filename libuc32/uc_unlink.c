/* uc_unlink */

/* interface component for UNIX® library-3c */
/* POSIX file-unlink */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern int	mkexpandpath(char *,cchar *,int) ;


/* exported subroutines */


int uc_unlink(cchar *fname)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	char		*ebuf ;

	if ((rs = uc_libmalloc((elen+1),&ebuf)) >= 0) {
	    if ((rs = mkexpandpath(ebuf,fname,-1)) > 0) {
		rs = u_unlink(ebuf) ;
	    } else if (rs == 0) {
		rs = u_unlink(fname) ;
	    }
	    uc_libfree(ebuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (uc_unlink) */


