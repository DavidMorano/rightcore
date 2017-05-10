/* opensysfs */

/* Open-System-File-System (OpenSysFS) */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	OPENSYSFS_INCLUDE
#define	OPENSYSFS_INCLUDE	1

#define	OPENSYSFS_SYSDNAME	"/tmp/sys"
#define	OPENSYSFS_DEFTTL	(2*3600)

#define	OPENSYSFS_FUSERHOMES	"userhomes"
#define	OPENSYSFS_FUSERNAMES	"usernames"
#define	OPENSYSFS_FGROUPNAMES	"groupnames"
#define	OPENSYSFS_FPROJECTNAMES	"projectnames"
#define	OPENSYSFS_FPASSWD	"passwd"
#define	OPENSYSFS_FGROUP	"group"
#define	OPENSYSFS_FPROJECT	"project"
#define	OPENSYSFS_FREALNAME	"realname"
#define	OPENSYSFS_FSHELLS	"shells"
#define	OPENSYSFS_FSHADOW	"shadow"
#define	OPENSYSFS_FUSERATTR	"userattr"

#define	OPENSYSFS_WUSERHOMES	0	/* user-homes */
#define	OPENSYSFS_WUSERNAMES	1	/* user-names */
#define	OPENSYSFS_WGROUPNAMES	2	/* group-names */
#define	OPENSYSFS_WPROJECTNAMES	3	/* project-names */
#define	OPENSYSFS_WPASSWD	4	/* "passwd" */
#define	OPENSYSFS_WGROUP	5	/* "group" */
#define	OPENSYSFS_WPROJECT	6	/* "project" */
#define	OPENSYSFS_WREALNAME	7	/* real-name map */
#define	OPENSYSFS_WSHELLS	8	/* user-shells */
#define	OPENSYSFS_WSHADOW	9	/* "shadow" */
#define	OPENSYSFS_WUSERATTR	10	/* "userattr" */


#if	(! defined(OPENSYSFS_MASTER)) || (OPENSYSFS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int opensysfs(int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENSYSFS_MASTER */

#endif /* OPENSYSFS_INCLUDE */


