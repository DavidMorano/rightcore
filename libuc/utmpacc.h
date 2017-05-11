/* utmpacc */

/* UTMPACC management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UTMPACC_INCLUDE
#define	UTMPACC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<utmpx.h>
#include	<localmisc.h>		/* extra types */
#include	<utmpaccent.h>


#undef	UTMPACC_DARWIN
#define	UTMPACC_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))

#undef	UTMPACC_SUNOS
#define	UTMPACC_SUNOS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#define	UTMPACC_ENT	UTMPACCENT
#define	UTMPACC_STATS	struct utmpacc_stats

#define	UTMPACC_BUFLEN	332		/* large enough for all fields */
#define	UTMPACC_MAX	20
#define	UTMPACC_TTL	(20*60)

#if	defined(UTMPACC_SUNOS) && UTMPACC_SUNOS
#define	UTMPACC_DEFUTMP	"/var/adm/utmpx"
#elif	defined(UTMPACC_DARWIN) && UTMPACC_DARWIN
#define	UTMPACC_DEFUTMP	"/var/run/utmpx"
#else
#define	UTMPACC_DEFUTMP	"/var/run/utmpx"
#endif

#ifndef	UTMPACC_TEMPTY
#define	UTMPACC_TEMPTY		0	/* entry is unused */
#define	UTMPACC_TRUNLEVEL	1
#define	UTMPACC_TBOOTTIME	2
#define	UTMPACC_TOLDTIME	3
#define	UTMPACC_TNEWTIME	4
#define	UTMPACC_TINITPROC	5	/* process spawned by "init" */
#define	UTMPACC_TLOGINPROC	6	/* a "getty" waiting for login */
#define	UTMPACC_TUSERPROC	7	/* a regular user process */
#define	UTMPACC_TDEADPROC	8	/* used in WTMPX only? */
#define	UTMPACC_TACCOUNT	9	/* used in WTMPX only? */
#define	UTMPACC_TSIGNATURE	10	/* used in WTMPX only? */
#endif /* UTMPACC_TEMPTY */

#ifndef	UTMPACC_LID
#define	UTMPACC_LID		4
#define	UTMPACC_LUSER		32
#define	UTMPACC_LLINE		32
#define	UTMPACC_LHOST		256
#endif


struct utmpacc_stats {
	uint		max ;
	uint		ttl ;
	uint		nent ;
	uint		acc ;
	uint		phit, nhit ;
	uint		pmis, nmis ;
} ;


#if	(! defined(UTMPACC_MASTER)) || (UTMPACC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int utmpacc_init() ;
extern int utmpacc_boottime(time_t *) ;
extern int utmpacc_runlevel() ;
extern int utmpacc_users(int) ;
extern int utmpacc_entsid(UTMPACC_ENT *,char *,int,pid_t) ;
extern int utmpacc_stats(UTMPACC_STATS *) ;
extern void utmpacc_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UTMPACC_MASTER */

#endif /* UTMPACC_INCLUDE */


