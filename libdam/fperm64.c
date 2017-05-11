/* fperm64 */

/* test the permissions on a file -- similar to 'access(2)' */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-08-15, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine is the "effective_user" version of 'access(2)'.


	Synopsis:

	int fperm64(fd,uid,gid,groups,m)
	int	fd ;
	uid_t	uid ;
	gid_t	gid, groups[] ;
	int	m ;


	Arguments:

	fd	file descriptor to check
	uid	UID to use for the check
	gid	GID to use for the check
	groups	the secondary GIDs to use for check
	mode	the mode as specified like with 'open(2)' but only
		the lower 3 bits are used, like with 'access(2)'


	Returns:

	0	access if allowed
	<0	access if denied for specified error code


************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<poll.h>

#include	<vsystem.h>



/* external variables */






int fperm64(fd,uid,gid,groups,m)
int	fd ;
uid_t	uid ;
gid_t	gid, groups[] ;
int	m ;
{
	struct stat64	sb ;

	gid_t	groups2[NGROUPS_MAX + 1] ;

	int	rs ;
	int	i ;
	int	n = NGROUPS_MAX ;
	int	euid = -1 ;
	int	um, gm ;


	if (fd < 0) 
		return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("perm: FD=%d\n",fd) ;

	debugprintf("perm: passwd UID=%d GID=%d\n",uid,gid) ;

	debugprintf("perm: process UID=%d EUID=%d GID=%d EGID=%d\n",
	    getuid(),geteuid(),getgid(),getegid()) ;
#endif

#if	CF_DEBUGS
	debugprintf("perm: 1 file_mode=%5o fileuid=%d filegid=%d\n",
	    sb.st_mode,sb.st_uid,sb.st_gid) ;
#endif

	if ((rs = u_fstat64(fd,&sb)) < 0) 
		return rs ;

	m &= 007 ;
	if (m == 0)
		return rs ;

/* check "user" permissions */

	um = (sb.st_mode >> 6) & m ;

#if	CF_DEBUGS
	debugprintf("perm: 2 m=%d, um=%d\n",m,um) ;
#endif

	if (uid < 0) {

	    euid = uid = geteuid() ;

	    if ((uid == sb.st_uid) || (uid == 0))
	        return ((um == m) ? SR_OK : SR_ACCES) ;

	    uid = getuid() ;

	}

#if	CF_DEBUGS
	debugprintf("perm: 2\n") ;
#endif

	if (uid == sb.st_uid)
	        return ((um == m) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("perm: 3\n") ;
#endif

/* check "group" permissions */

	gm = (sb.st_mode >> 3) & m ;

#if	CF_DEBUGS
	debugprintf("perm: gm=%d\n",gm) ;
#endif

	if (gid < 0) {

	    gid = getegid() ;

	    if ((gid == sb.st_gid) || (euid == 0))
	        return ((gm == m) ? SR_OK : SR_ACCES) ;

	    gid = getgid() ;

	} /* end if (effective group access check) */

#if	CF_DEBUGS
	debugprintf("perm: 4\n") ;
#endif

	if ((gid == sb.st_gid) || (euid == 0))
	    return ((gm == m) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("perm: 5\n") ;
#endif

	if (groups == NULL) {

	    groups = groups2 ;
	    n = getgroups(NGROUPS_MAX,groups) ;

	    groups[n] = -1 ;

	}

	for (i = 0 ; (i < n) && (groups[i] >= 0) ; i += 1) {

	    if (sb.st_gid == groups[i]) 
		break ;

	}

	if ((i < n) && (groups[i] >= 0))
	    return ((gm == m) ? SR_OK : SR_ACCES) ;

#if	CF_DEBUGS
	debugprintf("perm: 6\n") ;
#endif

/* check "other" permissions */

	if ((sb.st_mode & m) == m)
		return SR_OK ;

#if	CF_DEBUGS
	debugprintf("perm: 7\n") ;
#endif

	return SR_ACCES ;
}
/* end subroutine (fperm64) */



