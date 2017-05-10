/* finduid */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	FINDUID_INCLUDE
#define	FINDUID_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pwd.h>

#include	<ptm.h>
#include	<pwcache.h>
#include	<tmpx.h>
#include	<localmisc.h>		/* extra types */


#define	FINDUID		struct finduid_head
#define	FINDUID_STATS	struct finduid_s
#define	FINDUID_FL	struct finduid_flags

#define	FINDUID_MAGIC	0x98643169
#define	FINDUID_DEFMAX	20	/* default maximum entries */
#define	FINDUID_DEFTTL	600	/* default time-to-live */


struct finduid_s {
	uint		total ;		/* accesses */
	uint		refreshes ;	/* refreshes */
	uint		phits ;		/* positive hit */
	uint		nhits ;		/* negative hit */
} ;

struct finduid_flags {
	uint		ut:1 ;
} ;

struct finduid_head {
	uint		magic ;
	FINDUID_FL	open ;
	FINDUID_STATS	s ;
	PTM		m ;
	TMPX		ut ;
	PWCACHE		uc ;
	const char	*dbfname ;
	time_t		ti_utopen ;	/* open-time */
	time_t		ti_utcheck ;	/* check-time */
	int		ttl ;		/* time-to-live */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(FINDUID_MASTER)) || (FINDUID_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int finduid_start(FINDUID *,cchar *,int,int) ;
extern int finduid_lookup(FINDUID *,char *,uid_t) ;
extern int finduid_invalidate(FINDUID *,cchar *) ;
extern int finduid_check(FINDUID *,time_t) ;
extern int finduid_stats(FINDUID *,FINDUID_STATS *) ;
extern int finduid_finish(FINDUID *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FINDUID_MASTER */

#endif /* FINDUID_INCLUDE */


