/* ids */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	IDS_INCLUDE
#define	IDS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>


#define	IDS		struct ids

#ifndef	NGROUPS_MAX
#define	NGROUPS_MAX	16
#endif


struct ids {
	uid_t	uid, euid ;
	gid_t	gid, egid ;
	gid_t	egids[NGROUPS_MAX + 1] ;
} ;


#if	(! defined(IDS_MASTER)) || (IDS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ids_load(IDS *) ;
extern int	ids_release(IDS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* IDS_MASTER */


#endif /* IDS_INCLUDE */



