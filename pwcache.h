/* pwcache */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	PWCACHE_INCLUDE
#define	PWCACHE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<hdb.h>
#include	<pq.h>
#include	<localmisc.h>		/* extra types */


#define	PWCACHE			struct pwcache_head
#define	PWCACHE_STATS		struct pwcache_s

#define	PWCACHE_MAGIC		0x98643168
#define	PWCACHE_DEFENTS	10	/* default entries */
#define	PWCACHE_DEFMAX		20	/* default maximum entries */
#define	PWCACHE_DEFTTL		600	/* default time-to-live */


struct pwcache_s {
	uint		nentries ;		/* number of current entries */
	uint		total ;			/* accesses */
	uint		refreshes ;		/* refreshes */
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct pwcache_head {
	uint		magic ;
	PWCACHE_STATS	s ;
	HDB		db ;
	PQ		lru ;		/* least-recently-used */
	time_t		ti_check ;
	uint		wcount ;	/* write-count */
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(PWCACHE_MASTER)) || (PWCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pwcache_start(PWCACHE *,int,int) ;
extern int pwcache_lookup(PWCACHE *,struct passwd *,char *,int,const char *) ;
extern int pwcache_invalidate(PWCACHE *,const char *) ;
extern int pwcache_check(PWCACHE *,time_t) ;
extern int pwcache_stats(PWCACHE *,PWCACHE_STATS *) ;
extern int pwcache_finish(PWCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PWCACHE_MASTER */

#endif /* PWCACHE_INCLUDE */


