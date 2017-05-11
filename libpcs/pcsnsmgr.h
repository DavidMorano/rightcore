/* pcsnsmgr */

/* PCS Name-Server Cache */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSNSMGR_INCLUDE
#define	PCSNSMGR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#define	PCSNSMGR_STATS		struct pcsnsmgr_stats

#define	PCSNSMGR_MAX		120		/* max cache entries */
#define	PCSNSMGR_TTL		(20*60)		/* entry time-out in seconds */
#define	PCSNSMGR_TOLOCK		5


struct pcsnsmgr_stats {
	uint		max ;
	uint		ttl ;
	uint		nent ;
	uint		acc ;
	uint		phit, nhit ;
	uint		pmis, nmis ;
} ;


#if	(! defined(PCSNSMGR_MASTER)) || (PCSNSMGR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsnsmgr_init() ;
extern int pcsnsmgr_set(cchar *,int,cchar *,int,int) ;
extern int pcsnsmgr_get(char *,int,cchar *,int) ;
extern int pcsnsmgr_invalidate(cchar *,int) ;
extern int pcsnsmgr_stats(PCSNSMGR_STATS *) ;
extern void pcsnsmgr_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSNSMGR_MASTER */

#endif /* PCSNSMGR_INCLUDE */


