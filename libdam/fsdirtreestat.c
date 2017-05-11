/* fsdirtreestat */

/* get status on a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine provides an independent stand-alone STAT function
	for the FSDIRTREE object.

	Synopsis:

	int fsdirtreestat(fname,type,sbp)
	const char	fname[] ;
	int		type ;
	FSDIRTREE_STAT	*sbp ;

	Arguments:

	fname		file-name to STAT
	type		type of STAT: 0=regular, 1=lstat
	sbp		ponter to a STAT block structure

	Returns:

	<0		error
	=>0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fsdirtree.h"


/* local defines */


/* external subroutines */


/* forward references */


/* exported subroutines */


int fsdirtreestat(cchar *fname,int type,FSDIRTREE_STAT *sbp)
{
	int		rs ;

	if (fname == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;

	if (type == 0) {
	    rs = uc_stat(fname,sbp) ;
	} else {
	    rs = uc_lstat(fname,sbp) ;
	}

	return rs ;
}
/* end subroutine (fsdirtreestat) */


