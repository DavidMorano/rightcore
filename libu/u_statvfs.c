/* u_statvfs */

/* this is used to "stat" a file system */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	This part of the stuff-out work using UNIX.

	= 2004-12-16, David A­D­ Morano
	I added a hack to get this stuff working on Darwin.

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This file provides the user-friendly version of STATing the filesystem.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>

#if	defined(SYSHAS_STATVFS) && (SYSHAS_STATVFS == 1)
#include	<sys/statvfs.h>
#else
#include	<sys/param.h>
#include	<sys/mount.h>
#include	<sys/statvfs.h>
#endif

#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_STATVFS) && (SYSHAS_STATVFS == 1)

int u_statvfs(cchar *fname,struct statvfs *sbp)
{
	int		rs ;

	repeat {
	    if ((rs = statvfs(fname,sbp)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (u_statvfs) */

#else

int u_statvfs(cchar *fname,struct statvfs *sbp)
{
	int		rs ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    const int	fd = rs ;
	    rs = u_fstatvfs(fd,sbp) ;
	    u_close(fd) ;
	}

	return rs ;
}
/* end subroutine (u_statvfs) */

#endif /* (USTATVFS_NATIVE == 1) */


