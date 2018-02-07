/* lkmail */


/* revision history:

	= 1998-02-05, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LKMAIL_INCLUDE
#define	LKMAIL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<localmisc.h>


#define	LKMAIL_MAGIC	0x95437651
#define	LKMAIL		struct lkmail_head
#define	LKMAIL_IDS	struct lkmail_ids


struct lkmail_ids {
	uid_t		uid, euid, uid_maildir ;
	gid_t		gid, egid, gid_maildir ;
} ;

struct lkmail_head {
	uint		magic ;
	LKMAIL_IDS	id ;
	int		lfd ;
	char		lockfname[MAXPATHLEN + 1] ;
} ;


#if	(! defined(LKMAIL_MASTER)) || (LKMAIL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lkmail_start(LKMAIL *,LKMAIL_IDS *,cchar *) ;
extern int lkmail_create(LKMAIL *) ;
extern int lkmail_unlink(LKMAIL *) ;
extern int lkmail_old(LKMAIL *,time_t,int) ;
extern int lkmail_finish(LKMAIL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LKMAIL_MASTER */

#endif /* LKMAIL_INCLUDE */


