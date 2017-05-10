/* pathtrack */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PATHTRACK_INCLUDE
#define	PATHTRACK_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<dirlist.h>
#include	<hdbstr.h>
#include	<localmisc.h>


#define	PATHTRACK		struct pathtrack_head

#define	PATHTRACK_MAGIC		0x86529874


struct pathtrack_head {
	uint		magic ;
	DIRLIST		epath ;
	HDBSTR		map ;
} ;


#if	(! defined(PATHTRACK_MASTER)) || (PATHTRACK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pathtrack_start(PATHTRACK *) ;
extern int pathtrack_finish(PATHTRACK *) ;
extern int pathtrack_count(PATHTRACK *) ;
extern int pathtrack_add(PATHTRACK *,pid_t) ;
extern int pathtrack_poll(PATHTRACK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PATHTRACK_MASTER */


#endif /* PATHTRACK_INCLUDE */



