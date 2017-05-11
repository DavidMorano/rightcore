/* tcsetown */

/* set ownership of a terminal */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to 'logindevperm(3c)' (or whatever equivalent
        your system has). It sets the ownership of a terminal.

	Synopsis:

	int tcsetown(fd,termdev,uid,gid,perms)
	int		fd ;
	const char	*termdev ;
	uid_t		uid ;
	gid_t		gid ;
	mode_t		perms ;

	Arguments:

	fd		file-descriptor of terminal
	termdev		device path to terminal device
	uid		UID to set
	gid		GID to set
	perms		permissions to set

	Returns:

	<0		error
	>=0		success


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int tcsetown(int fd,cchar *termdev,uid_t uid,gid_t gid,mode_t perms)
{
	int		rs ;

	if (fd < 0) return SR_BADF ;

	perms &= S_IAMB ;
	if (isatty(fd)) {
	    if ((rs = u_fchmod(fd,perms)) >= 0) {
	        if ((rs = u_fchown(fd,uid,gid)) >= 0) {
	            if ((termdev != NULL) && (termdev[0] != '\0')) {
	                if ((rs = u_chmod(termdev,perms)) >= 0) {
	                    rs = u_chown(termdev,uid,gid) ;
			}
		    }
	        }
	    }
	} else
	    rs = SR_NOTTY ;

	return rs ;
}
/* end subroutine (tcsetown) */


