/* uc_utimes */

/* set UNIX® times on a file */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the middleware version of |u_utime(3u)|.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>		/* for |timeval| */
#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern int	mkexpandpath(char *,cchar *,int) ;


/* exported subroutines */


int uc_utimes(cchar *fname,const struct timeval *utp)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	char		*ebuf ;

	if ((rs = uc_libmalloc((elen+1),&ebuf)) >= 0) {
	    if ((rs = mkexpandpath(ebuf,fname,-1)) > 0) {
		rs = u_utimes(ebuf,utp) ;
	    } else if (rs == 0) {
		rs = u_utimes(fname,utp) ;
	    }
	    uc_libfree(ebuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (uc_utimes) */


