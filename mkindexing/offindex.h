/* offindex */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	OFFINDEX_INCLUDE
#define	OFFINDEX_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	OFFINDEX	struct offindex_head
#define	OFFINDEX_FL	struct offindex_flags
#define	OFFINDEX_MAGIC	0x89795142


struct offindex_flags {
	int		setsorted:1 ;
} ;

struct offindex_head {
	uint		magic ;
	vecobj		list ;
	OFFINDEX_FL	f ;
} ;


#if	(! defined(OFFINDEX_MASTER)) || (OFFINDEX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	offindex_start(OFFINDEX *,int) ;
extern int	offindex_add(OFFINDEX *,offset_t,int) ;
extern int	offindex_lookup(OFFINDEX *,offset_t) ;
extern int	offindex_finish(OFFINDEX *) ;

#ifdef	__cplusplus
}
#endif

#endif	/* OFFINDEX_MASTER */

#endif /* OFFINDEX_INCLUDE */


