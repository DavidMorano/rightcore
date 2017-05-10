/* ptrwlock */
/* broken on SOLARIS! */

/* Posix Semaphore (PTRWLOCK) */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTRWLOCK_INCLUDE
#define	PTRWLOCK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>

#include	"ptrwa.h"


#define	PTRWLOCK		pthread_rwlock_t
#define	PTRWLOCK_NAMELEN	(MAXNAMELEN+1)

#ifndef	UINT
#define	UINT	unsigned int
#endif


#if	(! defined(PTRWLOCK_MASTER)) || (PTRWLOCK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptrwlock_create(PTRWLOCK *,PTRWA *) ;
extern int	ptrwlock_init(PTRWLOCK *,PTRWA *) ;
extern int	ptrwlock_rdlock(PTRWLOCK *) ;
extern int	ptrwlock_tryrdlock(PTRWLOCK *) ;
extern int	ptrwlock_rdlockto(PTRWLOCK *,int) ;
extern int	ptrwlock_wrlock(PTRWLOCK *) ;
extern int	ptrwlock_trywrlock(PTRWLOCK *) ;
extern int	ptrwlock_wrlockto(PTRWLOCK *,int) ;
extern int	ptrwlock_unlock(PTRWLOCK *) ;
extern int	ptrwlock_destroy(PTRWLOCK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTRWLOCK_MASTER */

#endif /* PTRWLOCK_INCLUDE */


