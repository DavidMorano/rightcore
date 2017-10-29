/* xfile */

/* eXecutable File? */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test if a given file is executable (has its 'x' access permission
	mode bit set).

	Synopsis:

	int xfile(idp,fname)
	IDS		*idp ;
	const char	fname[] ;

	Arguments:

	idp		pointer to IDS object
	fname		name of file to check

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sperm(IDS *,struct ustat *,int) ;


/* exported subroutines */


int xfile(IDS *idp,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;
	if ((rs = u_stat(fname,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
		rs = sperm(idp,&sb,X_OK) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	}
	return rs ;
}
/* end subroutine (xfile) */


