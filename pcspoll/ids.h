/* ids */


/* revision history:

	= 1998-02-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	IDS_INCLUDE
#define	IDS_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>


#define	IDS		struct ids


struct ids {
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	gid_t		*gids ;
} ;


#if	(! defined(IDS_MASTER)) || (IDS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ids_load(IDS *) ;
extern int	ids_ngroups(IDS *) ;
extern int	ids_release(IDS *) ;
extern int	ids_refresh(IDS *) ;
extern int	ids_copy(IDS *,IDS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* IDS_MASTER */

#endif /* IDS_INCLUDE */


