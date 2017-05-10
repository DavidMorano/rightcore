/* upwcache */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	UPWCACHE_INCLUDE
#define	UPWCACHE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<pq.h>
#include	<localmisc.h>		/* extra types */


#define	UPWCACHE_MAGIC		0x98643168
#define	UPWCACHE		struct upwcache_head
#define	UPWCACHE_STATS		struct upwcache_s

#define	UPWCACHE_DEFENTS	10	/* default entries */
#define	UPWCACHE_DEFMAX		20	/* default maximum entries */
#define	UPWCACHE_DEFTTL		(10*60)	/* default time-to-live */


struct upwcache_s {
	uint		nentries ;		/* number of current entries */
	uint		total ;			/* accesses */
	uint		refreshes ;		/* refreshes */
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct upwcache_head {
	uint		magic ;
	void		*recs ;		/* linear array (of recs) */
	UPWCACHE_STATS	s ;
	PQ		lru ;		/* least-recently-used */
	time_t		ti_check ;
	uint		wcount ;	/* write-count */
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(UPWCACHE_MASTER)) || (UPWCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int upwcache_start(UPWCACHE *,int,int) ;
extern int upwcache_lookup(UPWCACHE *,struct passwd *,char *,int,const char *) ;
extern int upwcache_uid(UPWCACHE *,struct passwd *,char *,int,uid_t) ;
extern int upwcache_invalidate(UPWCACHE *,const char *) ;
extern int upwcache_check(UPWCACHE *,time_t) ;
extern int upwcache_stats(UPWCACHE *,UPWCACHE_STATS *) ;
extern int upwcache_finish(UPWCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UPWCACHE_MASTER */

#endif /* UPWCACHE_INCLUDE */


