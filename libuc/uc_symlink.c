/* uc_symlink */

/* interface component for UNIX® library-3c */
/* POSIX shared-meory ("shm(3rt)') file-unlink */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern int	mkexpandpath(char *,cchar *,int) ;
extern int	mkuserpath(char *,cchar *,cchar *,int) ;


/* exported subroutines */


int uc_symlink(cchar *ofname,cchar *nfname)
{
	const int	plen = MAXPATHLEN ;
	int		rs ;
	int		size = 0 ;
	char		*bp ;

	size += ((plen+1) * 2) ;
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	    char	*obuf = bp + ((plen+1)*0) ;
	    char	*nbuf = bp + ((plen+1)*1) ;
	    if ((rs = mkexpandpath(obuf,ofname,-1)) > 0) {
		ofname = obuf ;
	    }
	    if (rs >= 0) {
	        if ((rs = mkuserpath(nbuf,NULL,nfname,-1)) > 0) {
		    nfname = nbuf ;
	        }
		if (rs >= 0) {
		    rs = u_symlink(ofname,nfname) ;
		}
	    } /* end if */
	    uc_libfree(bp) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (uc_symlink) */


