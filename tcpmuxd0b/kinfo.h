/* kinfo */

/* get kernel information */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KINFO_INCLUDE
#define	KINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<kstat.h>

#include	<localmisc.h>


/* object defines */

#define	KINFO		struct kinfo_head
#define	KINFO_DATA	struct kinfo_data


struct kinfo_data {
	uint		boottime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
} ;

struct kinfo_flags {
	uint		kopen:1 ;
} ;

struct kinfo_head {
	uint		magic ;
	kstat_ctl_t	*kcp ;
	time_t		ti_access ;		/* last access */
	time_t		ti_sysmisc ;		/* last SYSMISC update */
	time_t		ti_loadave ;		/* last LOADAVE update */
	time_t		ti_update ;		/* last KSTAT chain update */
	struct kinfo_flags	f ;
	KINFO_DATA		d ;
	int		nactive ;		/* number of active requests */
	int		tosysmisc ;
	int		toloadave ;
} ;


#if	(! defined(KINFO_MASTER)) || (KINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int kinfo_open(KINFO *,time_t) ;
extern int kinfo_boottime(KINFO *,time_t,KINFO_DATA *) ;
extern int kinfo_loadave(KINFO *,time_t,KINFO_DATA *) ;
extern int kinfo_sysmisc(KINFO *,time_t,KINFO_DATA *) ;
extern int kinfo_check(KINFO *,time_t) ;
extern int kinfo_close(KINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KINFO_MASTER */

#endif /* KINFO_INCLUDE */


