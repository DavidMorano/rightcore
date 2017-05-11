/* uc_fminmod */

/* interface component for UNIX® library-3c */
/* set (ensure) a minimum mode on a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int uc_fminmod(fd,mm)
int	fd ;
mode_t	mm ;
{
	struct ustat	sb ;

	int	rs ;
	int	f_changed = FALSE ;

	if (fd < 0) return SR_BADF ;

	mm &= (~ S_IFMT) ;
	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    mode_t	cm = (sb.st_mode & (~ S_IFMT)) ;

	    if ((cm & mm) != mm) {
	        f_changed = TRUE ;
	        mm |= cm ;
	        rs = u_fchmod(fd,mm) ;
	    } /* end if (needed a change) */

	} /* end if (successful stat) */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (uc_fminmod) */


