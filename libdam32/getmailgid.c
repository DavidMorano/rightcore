/* getmailgid */

/* get the GID for the group 'mail' or the mail-spool area */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was written to collect this code into one subroutines.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns a GID for a specified groupname.  A default GID
	is returned if the groupname does not exist.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	getgroup_gid(cchar *,int) ;


/* exported subroutines */


int getmailgid(cchar *gname,gid_t gid)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	if (gname == NULL) return SR_FAULT ;
	if ((rs = getgroup_gid(gname,-1)) == nrs) {
	    if (gid >= 0) {
	        gid = rs ;
	    } else {
	        gid = getgid() ;
	    }
	}
	return rs ;
}
/* end subroutine (getmailgid) */


