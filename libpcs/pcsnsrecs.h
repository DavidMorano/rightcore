/* pcsnsrecs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSNSRECS_INCLUDE
#define	PCSNSRECS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<pq.h>
#include	<localmisc.h>		/* extra types */


#define	PCSNSRECS_MAGIC		0x98643168
#define	PCSNSRECS		struct pcsnsrecs_head
#define	PCSNSRECS_ST		struct pcsnsrecs_s

#define	PCSNSRECS_DEFENTS	10	/* default entries */
#define	PCSNSRECS_DEFMAX	20	/* default maximum entries */
#define	PCSNSRECS_DEFTTL	(10*60)	/* default time-to-live */


struct pcsnsrecs_s {
	uint		nentries ;		/* number of current entries */
	uint		total ;			/* accesses */
	uint		refreshes ;		/* refreshes */
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct pcsnsrecs_head {
	uint		magic ;
	void		*recs ;		/* linear array (of recs) */
	PCSNSRECS_ST	s ;
	PQ		lru ;		/* least-recently-used */
	time_t		ti_check ;
	uint		wcount ;	/* write-count */
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(PCSNSRECS_MASTER)) || (PCSNSRECS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsnsrecs_start(PCSNSRECS *,int,int) ;
extern int pcsnsrecs_store(PCSNSRECS *,cchar *,int,cchar *,int,int) ;
extern int pcsnsrecs_lookup(PCSNSRECS *,char *,int,cchar *,int) ;
extern int pcsnsrecs_invalidate(PCSNSRECS *,cchar *,int) ;
extern int pcsnsrecs_check(PCSNSRECS *,time_t) ;
extern int pcsnsrecs_stats(PCSNSRECS *,PCSNSRECS_ST *) ;
extern int pcsnsrecs_finish(PCSNSRECS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSNSRECS_MASTER */

#endif /* PCSNSRECS_INCLUDE */


