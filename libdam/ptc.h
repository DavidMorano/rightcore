/* ptc */

/* POSIX Thread Condition manipulation */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTC_INCLUDE
#define	PTC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>
#include	<time.h>

#include	"ptm.h"			/* needed for interface */
#include	"ptca.h"


#define	PTC		pthread_cond_t

#ifndef	UINT
#define	UINT		unsigned int
#endif


#if	(! defined(PTC_MASTER)) || (PTC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptc_create(PTC *,PTCA *) ;
extern int	ptc_init(PTC *,PTCA *) ;
extern int	ptc_destroy(PTC *) ;
extern int	ptc_broadcast(PTC *) ;
extern int	ptc_signal(PTC *) ;
extern int	ptc_wait(PTC *,PTM *) ;
extern int	ptc_waiter(PTC *,PTM *,int) ;
extern int	ptc_timedwait(PTC *,PTM *,struct timespec *) ;
extern int	ptc_reltimedwaitnp(PTC *,PTM *,struct timespec *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTC_MASTER */

#endif /* PTC_INCLUDE */


