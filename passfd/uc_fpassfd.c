/* uc_fpassfd */

/* interface component for UNIX® library-3c */
/* pass a file-descriptor to a file-descriptor */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We pass a file-descriptor to another file-descriptor.

	Synopsis:

	int uc_fpassfd(int pfd,int fd)

	Arguments:

	pfd		pass-file-descriptor
	fd		file-descriptor to pass

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif


/* external variables */


/* forward reference */


/* local variables */


/* exported subroutines */


int uc_fpassfd(int fd_pass,int fd)
{
	USTAT		sb ;
	int		rs ;
	if ((rs = u_fstat(fd_pass,&sb)) >= 0) {
	    if (S_ISCHR(sb.st_mode) || S_ISFIFO(sb.st_mode)) {
		rs = u_ioctl(fd_pass,I_SENDFD,fd) ;
	    } else {
		rs = SR_NOSTR ;
	    }
	}
	return rs ;
}
/* end subroutine (uc_fpassfd) */


