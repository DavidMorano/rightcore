/* proginfo_real */

/* utility for KSH built-in commands */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to reset the UID, GID, or both so that the
	effective credentials of the program temporarily become those of the
	real user or real group.

	------------------------------------------------------------------------
	Name:

	proginfo_realbegin

	Synopsis:

	int proginfo_realbegin(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		program-information pointer

	Returns:

	<0	error
	>=0	length of PR

	------------------------------------------------------------------------
	Name:

	proginfo_realend

	Synopsis:

	int proginfo_realend(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		program-information pointer

	Returns:

	<0	error
	>=0	length of PR


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int proginfo_realbegin(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->f.setuid) {
	    uid_t	euid = geteuid() ;
	    if (euid != pip->uid) u_seteuid(pip->uid) ;
	}

	if (pip->f.setgid) {
	    gid_t	egid = getegid() ;
	    if (egid != pip->gid) u_setegid(pip->gid) ;
	}

	return rs ;
}
/* end subroutine (proginfo_realbegin) */


int proginfo_realend(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->f.setuid) {
	    uid_t	euid = geteuid() ;
	    if (euid != pip->euid) u_seteuid(pip->euid) ;
	} /* end if (setuid) */

	if (pip->f.setgid) {
	    gid_t	egid = getegid() ;
	    if (egid != pip->egid) u_setegid(pip->egid) ;
	} /* end if (setgid) */

	return rs ;
}
/* end subroutine (proginfo_realend) */


