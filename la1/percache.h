/* percache */


/* revision history:

	= 1998-01-22, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PERCACHE_INCLUDE
#define	PERCACHE_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>	/* for special types */


#define	PERCACHE	struct percache_h
#define	PERCACHE_ITEM	struct percache_i


enum pertypes {
	pertype_hostid,
	pertype_nprocs,
	pertype_btime,
	pertype_runlevel,
	pertype_nusers,
	pertype_sysdomain,
	pertype_netload,
	pertype_systat,
	pertype_overlast
} ;

struct percache_i {
	uint		t ;
	uint		v ;
} ;

struct percache_h {
	volatile uint	f_init ;
	volatile uint	f_initdone ;
	volatile uint	f_finireg ;
	PERCACHE_ITEM	items[pertype_overlast] ;
	const char	*sysdomain ;
	const char	*netload ;
	const char	*systat ;
} ;


#if	(! defined(PERCACHE_MASTER)) || (PERCACHE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	percache_init(PERCACHE *) ;
extern int	percache_finireg(PERCACHE *) ;
extern int	percache_invalidate(PERCACHE *) ;
extern int	percache_gethostid(PERCACHE *,time_t,uint *) ;
extern int	percache_getnprocs(PERCACHE *,time_t) ;
extern int	percache_getbtime(PERCACHE *,time_t,time_t *) ;
extern int	percache_getrunlevel(PERCACHE *,time_t) ;
extern int	percache_getnusers(PERCACHE *,time_t) ;
extern int	percache_getsysdomain(PERCACHE *,time_t,cchar **) ;
extern int	percache_getnetload(PERCACHE *,time_t,cchar *,cchar **) ;
extern int	percache_getsystat(PERCACHE *,time_t,cchar *,cchar **) ;
extern int	percache_fini(PERCACHE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PERCACHE_MASTER */

#endif /* PERCACHE_INCLUDE */


