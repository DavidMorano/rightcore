/* grmems */


/* revision history:

	= 2004-08-27, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	GRMEMS_INCLUDE
#define	GRMEMS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<pq.h>
#include	<localmisc.h>		/* extra types */


#define	GRMEMS_MAGIC		0x98643169
#define	GRMEMS_CURMAGIC		0x9864316a
#define	GRMEMS			struct grmems_head
#define	GRMEMS_STATS		struct grmems_s
#define	GRMEMS_CUR		struct grmems_c
#define	GRMEMS_SYSPASSWD	"/sys/passwd"


struct grmems_c {
	uint		magic ;
	int		ri ;		/* record index */
	int		i ;		/* index through members */
} ;

struct grmems_s {
	uint		nentries ;		/* number of current entries */
	uint		total ;			/* accesses */
	uint		refreshes ;		/* refreshes */
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct grmems_head {
	uint		magic ;
	void		*recs ;		/* linear array (of recs) */
	GRMEMS_STATS	s ;
	PQ		lru ;		/* least-recently-used */
	const void	*usergids ;
	const void	*mapdata ;
	size_t		mapsize ;
	time_t		ti_check ;
	time_t		ti_open ;
	time_t		ti_access ;
	time_t		ti_usergids ;
	uint		wcount ;	/* write-count */
	int		pagesize ;
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
	int		cursors ;	/* cursors outstanding */
	int		fd ;
	int		fsize ;
	int		nusergids ;
} ;


#if	(! defined(GRMEMS_MASTER)) || (GRMEMS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int grmems_start(GRMEMS *,int,int) ;
extern int grmems_curbegin(GRMEMS *,GRMEMS_CUR *) ;
extern int grmems_lookup(GRMEMS *,GRMEMS_CUR *,cchar *,int) ;
extern int grmems_lookread(GRMEMS *,GRMEMS_CUR *,char *,int) ;
extern int grmems_curend(GRMEMS *,GRMEMS_CUR *) ;
extern int grmems_invalidate(GRMEMS *,cchar *,int) ;
extern int grmems_check(GRMEMS *,time_t) ;
extern int grmems_stats(GRMEMS *,GRMEMS_STATS *) ;
extern int grmems_finish(GRMEMS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* GRMEMS_MASTER */

#endif /* GRMEMS_INCLUDE */


