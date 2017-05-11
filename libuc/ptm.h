/* ptm */

/* POSIX Thread Mutex manipulation */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTM_INCLUDE
#define	PTM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>

#include	"ptma.h"


#define	PTM		pthread_mutex_t


#if	(! defined(PTM_MASTER)) || (PTM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptm_create(PTM *,PTMA *) ;
extern int	ptm_init(PTM *,PTMA *) ;
extern int	ptm_destroy(PTM *) ;
extern int	ptm_lock(PTM *) ;
extern int	ptm_locker(PTM *,int) ;
extern int	ptm_lockto(PTM *,int) ;
extern int	ptm_trylock(PTM *) ;
extern int	ptm_unlock(PTM *) ;
extern int	ptm_setprioceiling(PTM *,int,int *) ;
extern int	ptm_getprioceiling(PTM *,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTM_MASTER */

#endif /* PTM_INCLUDE */


