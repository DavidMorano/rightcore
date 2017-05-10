/* bfile */

/* utility subroutines for BFILE */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	These are some general utility subroutines for BFILE, mainly
	to maintenance portability consideration.


******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* local variables */


/* exported subroutines */


int bfilestat(fname,type,sbp)
const char	fname[] ;
int		type ;
BFILE_STAT	*sbp ;
{
	int	rs ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	if (sbp == NULL)
	   return SR_FAULT ;

	if (type > 0) {
	    rs = uc_lstat(fname,sbp) ;
	} else
	    rs = uc_stat(fname,sbp) ;

	return rs ;
}
/* end subroutine (bfilestat) */


int bfilefstat(fd,sbp)
int		fd ;
BFILE_STAT	*sbp ;
{
	int	rs ;


	if (fd < 0)
	   return SR_BADF ;

	if (sbp == NULL)
	   return SR_FAULT ;

	    rs = u_fstat(fd,sbp) ;

	return rs ;
}
/* end subroutine (bfilefstat) */



