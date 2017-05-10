/* u_lf64 */

/* 64-bit "largefile" version */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        There is not 64-bit version of this! So we just use the regular version
        instead!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* exported subroutines */


int u_close64(fd)
int	fd ;
{
	return u_close(fd) ;
}
/* end subroutine (u_close64) */


int u_creat64(fname,m)
const char	fname[] ;
int		m ;
{
	return u_creat64(fname,m) ;
}
/* end subroutine (u_creat64) */


int u_fcntl64(fd,cmd,arg)
int	cmd ;
int	fd ;
int	arg ;
{
	return u_fcntl(fd,cmd,arg) ;
}
/* end subroutine (u_fcntl64) */


int u_fstat64(fd,sbp)
int		fd ;
struct stat64	*sbp ;
{
	struct ustat	*sp = (struct ustat *) sbp ;
	return u_fstat(fd,sp) ;
}
/* end subroutine (u_fstat64) */


int u_fstatvfs64(fd,sbp)
int		fd ;
struct statvfs64	*sbp ;
{
	struct ustatvfs	*sp = (struct ustatvfs *) sbp ;
	return u_fstatvfs(fd,sp) ;
}
/* end subroutine (u_fstatvfs64) */


int u_lstat64(fname,sbp)
const char	fname[] ;
struct stat64	*sbp ;
{
	struct ustat	*sp = (struct ustat *) sbp ;
	return u_lstat(fname,sp) ;
}
/* end subroutine (u_lstat64) */


int u_open64(fname,f,m)
const char	fname[] ;
int		f ;
mode_t		m ;
{
	return u_open(fname,f,m) ;
}
/* end subroutine (u_open64) */


int u_seeko64(fd,wo,w,offp)
int		fd ;
off64_t		wo ;
int		w ;
off64_t		*offp ;
{
	return u_seeko(fd,wo,w,offp) ;
}
/* end subroutine (u_seeko64) */


int u_seekoff64(fd,wo,w,offp)
int		fd ;
off64_t		wo ;
int		w ;
off64_t		*offp ;
{
	return u_seeko(fd,wo,w,offp) ;
}
/* end subroutine (u_seekoff64) */


int u_seek64(fd,o,w)
int	fd ;
off64_t	o ;
int	w ;
{
	return u_seek(fd,o,w) ;
}
/* end subroutine (u_seek64) */


int u_stat64(fname,sbp)
const char	fname[] ;
struct stat64	*sbp ;
{
	struct ustat	*sp = (struct ustat *) sbp ;
	return u_stat(fname,sp) ;
}
/* end subroutine (u_stat64) */


int u_statvfs64(fname,sbp)
const char	fname[] ;
struct statvfs64	*sbp ;
{
	struct ustatvfs	*sp = (struct ustatvfs *) sbp ;
	return u_statvfs(fname,sp) ;
}
/* end subroutine (u_statvfs64) */


int u_tell64(fd,rp)
int	fd ;
off64_t	*rp ;
{
	return u_tell(fd,rp) ;
}
/* end subroutine (u_tell64) */


