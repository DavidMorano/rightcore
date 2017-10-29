/* sperm */

/* test the permissions on a file -- similar to 'access(2)' */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-01-15, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is sort of the "effective_user" version of
	'access(2)'.  It has more features that enable it to perform the
	function of 'access(2)' as well as other variations.

	Synopsis:

	int sperm(idp,sbp,am)
	IDS		*idp ;
	struct ustat	*sbp ;
	int		am ;

	Arguments:

	idp	pointer to IDS object
	sbp	pointer to file status structure ('stat(2)')
	am	the access-mode as specified like with 'open(2)' but only
		the lower 3 bits are used, like with 'access(2)'

	Returns:

	0	access if allowed
	<0	access if denied for specified error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* exported subroutines */


int sperm(IDS *idp,struct ustat *sbp,int am)
{
	int		rs = SR_OK ;
	int		i, ft, um, gm ;

	if (idp == NULL) return SR_INVAL ;
	if (sbp == NULL) return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("sperm: fm=%07o am=%07o\n",sbp->st_mode,m) ;
#endif

	if (idp->euid == 0) /* always succeeds */ {

#if	CF_DEBUGS
	debugprintf("sperm: euid==0 rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

/* extra file-type check (if a file-type was specified, fail if different) */

	ft = am & S_IFMT ;
	if ((ft != 0) && ((sbp->st_mode & S_IFMT) != ft)) {
	    rs = SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("sperm: ft mismatch rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

/* now on to the regular permissions checks */

	am &= 007 ;
	if (am == 0) {

#if	CF_DEBUGS
	debugprintf("sperm: no mode specified rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

/* check "user" permissions */

	um = (sbp->st_mode >> 6) & am ;

	if (idp->euid == sbp->st_uid) {
	    rs = ((um == am) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("sperm: user rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

/* check "group" permissions */

	gm = (sbp->st_mode >> 3) & am ;

	if (idp->egid == sbp->st_gid) {
	    rs = ((gm == am) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("sperm: group rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

	if (idp->gids != NULL) {
	for (i = 0 ; idp->gids[i] >= 0 ; i += 1) {
	    if (sbp->st_gid == idp->gids[i]) break ;
	} /* end for */
	if (idp->gids[i] >= 0) {
	    rs = ((gm == am) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("sperm: sup-group rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}
	} /* end if (gids) */

/* check "other" permissions */

	if ((sbp->st_mode & am) == am) {

#if	CF_DEBUGS
	debugprintf("sperm: other rs=%d\n",rs) ;
#endif

	    goto ret0 ;
	}

/* other checks? */

#if	defined(SYSHAS_ACL) && (SYSHAS_ACL != 0)
/* there's nothing that we can do since we don't have a "handle" to the file */
#endif

	rs = SR_ACCES ;

ret0:

#if	CF_DEBUGS
	debugprintf("sperm: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sperm) */


