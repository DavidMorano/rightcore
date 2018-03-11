/* gncache */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	GNCACHE_INCLUDE
#define	GNCACHE_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<cq.h>
#include	<localmisc.h>


#define	GNCACHE			struct gncache_head
#define	GNCACHE_STATS		struct gncache_s
#define	GNCACHE_ENT		struct gncache_e

#define	GNCACHE_MAGIC		0x98643162
#define	GNCACHE_DEFENT		10
#define	GNCACHE_DEFMAX		20	/* default maximum entries */
#define	GNCACHE_DEFTTL		600	/* default time-to-live */
#define	GNCACHE_MAXFREE		4


struct gncache_e {
	gid_t		gid ;
	char		groupname[GROUPNAMELEN + 1] ;
} ;

struct gncache_s {
	uint		nentries ;
	uint		total ;			/* access */
	uint		refreshes ;
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct gncache_head {
	uint		magic ;
	GNCACHE_STATS	s ;
	CQ		recsfree ;
	vechand		recs ;
	time_t		ti_check ;
	int		ttl ;		/* time-to-live in seconds */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(GNCACHE_MASTER)) || (GNCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int gncache_start(GNCACHE *,int,int) ;
extern int gncache_add(GNCACHE *,gid_t,const char *) ;
extern int gncache_lookname(GNCACHE *,GNCACHE_ENT *,const char *) ;
extern int gncache_lookgid(GNCACHE *,GNCACHE_ENT *,gid_t) ;
extern int gncache_check(GNCACHE *,time_t) ;
extern int gncache_stats(GNCACHE *,GNCACHE_STATS *) ;
extern int gncache_finish(GNCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* GNCACHE_MASTER */

#endif /* GNCACHE_INCLUDE */


