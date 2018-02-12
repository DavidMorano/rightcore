/* grcache */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	GRCACHE_INCLUDE
#define	GRCACHE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<grp.h>

#include	<vechand.h>
#include	<cq.h>
#include	<localmisc.h>


#define	GRCACHE			struct grcache_head
#define	GRCACHE_STATS		struct grcache_s

#define	GRCACHE_MAGIC		0x98643162
#define	GRCACHE_DEFENTS		10
#define	GRCACHE_DEFMAX		20	/* default maximum entries */
#define	GRCACHE_DEFTTL		600	/* default time-to-live */
#define	GRCACHE_MAXFREE		4


struct grcache_s {
	uint		nentries ;		/* number of current entries */
	uint		total ;			/* accesses */
	uint		refreshes ;		/* refreshes */
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct grcache_head {
	uint		magic ;
	GRCACHE_STATS	s ;
	CQ		recsfree ;
	vechand		recs ;
	time_t		ti_check ;
	uint		wcount ;
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(GRCACHE_MASTER)) || (GRCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int grcache_start(GRCACHE *,int,int) ;
extern int grcache_lookname(GRCACHE *,struct group *,char *,int,const char *) ;
extern int grcache_lookgid(GRCACHE *,struct group *,char *,int,gid_t) ;
extern int grcache_check(GRCACHE *,time_t) ;
extern int grcache_stats(GRCACHE *,GRCACHE_STATS *) ;
extern int grcache_finish(GRCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* GRCACHE_MASTER */

#endif /* GRCACHE_INCLUDE */


