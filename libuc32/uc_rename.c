/* uc_rename */

/* rename (link to and delete original) a file */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the middleware version of |u_rename(3u)|.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern int	mkexpandpath(char *,cchar *,int) ;


/* exported subroutines */


int uc_rename(cchar *ofname,cchar *nfname)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	int		size = 0 ;
	int		rv = 0 ;
	char		*bp ;

	size += ((elen+1) * 2) ;
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	    int		ol = -1 ;
	    char	*obuf = bp + ((elen+1)*0) ;
	    char	*nbuf = bp + ((elen+1)*1) ;
	    if ((rs = mkexpandpath(obuf,ofname,-1)) >= 0) {
		if (rs > 0) {
		    ol = rs ;
		    ofname = obuf ;
		}
	        if ((rs = mkexpandpath(nbuf,nfname,ol)) >= 0) {
		    if (rs > 0) nfname = nbuf ;
		    rs = u_rename(ofname,nfname) ;
		    rv = rs ;
		}
	    } /* end if */
	    uc_libfree(bp) ;
	} /* end if (m-a) */

	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (uc_rename) */


