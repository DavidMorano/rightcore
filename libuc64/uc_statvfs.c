/* uc_statvfs */

/* interface component for UNIX® library-3c */
/* get status on a file */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/statvfs.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkvarpath(char *,const char *,int) ;
extern int	hasvarpathprefix(const char *,int) ;


/* forward references */


/* exported subroutines */


int uc_statvfs(fname,sbp)
const char	fname[] ;
struct statvfs	*sbp ;
{
	int	rs ;

	char	efname[MAXPATHLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

	rs = mkvarpath(efname,fname,-1) ;
	if (rs > 0) fname = efname ;

	if (rs >= 0)
	    rs = u_statvfs(fname,sbp) ;

	return rs ;
}
/* end subroutine (uc_statvfs) */


