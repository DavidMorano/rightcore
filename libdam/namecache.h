/* namecache */


/* revision history:

	= 2004-06-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	NAMECACHE_INCLUDE
#define	NAMECACHE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<hdb.h>
#include	<localmisc.h>


#define	NAMECACHE		struct namecache_head
#define	NAMECACHE_STATS		struct namecache_s

#define	NAMECACHE_MAGIC		0x98643167
#define	NAMECACHE_DEFENTS	20
#define	NAMECACHE_DEFMAX	21
#define	NAMECACHE_DEFTO		(5 * 60)


struct namecache_s {
	uint		nentries ;
	uint		total ;			/* access */
	uint		refreshes ;
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct namecache_head {
	uint		magic ;
	NAMECACHE_STATS	s ;
	HDB		db ;
	const char	*varname ;
	int		max ;		/* maximum number of entries */
	int		ttl ;		/* time-to-live (in seconds) */
} ;


#if	(! defined(NAMECACHE_MASTER)) || (NAMECACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int namecache_start(NAMECACHE *,const char *,int,int) ;
extern int namecache_add(NAMECACHE *,const char *,const char *,int) ;
extern int namecache_lookup(NAMECACHE *,const char *,const char **) ;
extern int namecache_stats(NAMECACHE *,NAMECACHE_STATS *) ;
extern int namecache_finish(NAMECACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NAMECACHE_MASTER */

#endif /* NAMECACHE_INCLUDE */


