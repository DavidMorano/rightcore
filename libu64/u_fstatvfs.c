/* u_fstatvfs */

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

#if	defined(SYSHAS_STATVFS) && (SYSHAS_STATVFS > 0)
#include	<sys/statvfs.h>
#else
#include	<sys/statvfs.h>
#include	<sys/param.h>
#include	<sys/mount.h>
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

#if	defined(SYSHAS_STATVFS) && (SYSHAS_STATVFS > 0)
#else
static int u_fstatfs(int,struct ustatfs *) ;
#endif


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_STATVFS) && (SYSHAS_STATVFS > 0)

int u_fstatvfs(int fd,struct statvfs *sbp)
{
	int		rs ;

	repeat {
	    if ((rs = fstatvfs(fd,sbp)) < 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

	return rs ;
}
/* end subroutine (u_fstatvfs) */

#else /* SYSHAS_STATVFS */

int u_fstatvfs(int fd,struct statvfs *sbp)
{
	struct ustatfs	sfs ;
	int		rs ;
	int		cl ;

	if (sbp == NULL) return SR_FAULT ;

	memset(sbp,0,sizeof(struct statvfs)) ;

	if ((rs = u_fstatfs(fd,&sfs)) >= 0) {

	    sbp->f_bsize = sfs.f_bsize ;
	    sbp->f_blocks = sfs.f_blocks ;
	    sbp->f_bfree = sfs.f_bfree ;
	    sbp->f_bavail = sfs.f_bavail ;
	    sbp->f_files = sfs.f_files ;
	    sbp->f_ffree = sfs.f_ffree ;
	    sbp->f_favail = sfs.f_ffree ;	/* Darwin doesn't have */
	    sbp->f_fsid = 0 ; 		/* hassles w/ Darwin */
	    cl = MIN(MFSNAMELEN,FSTYPSZ) ;
	    strncpy(sbp->f_basetype,sfs.f_fstypename,cl) ;

	    if (sfs.f_flags & MNT_RDONLY)
	        sbp->f_flag |= ST_RDONLY ;

	    if (sfs.f_flags & MNT_NOSUID)
	        sbp->f_flag |= ST_NOSUID ;

	} /* end if (successful STAT) */

	return rs ;
}
/* end subroutine (u_fstatvfs) */


static int u_fstatfs(int fd,struct ustatfs *ssp)
{
	int		rs ;

	rs = fstatfs(fd,ssp) ;
	if (rs < 0) rs = (- errno) ;

	return rs ;
}

#endif /* SYSHAS_STATVFS */


