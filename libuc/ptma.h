/* ptma */

/* POSIX Thread Mutex Attribute manipulation */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTMA_INCLUDE
#define	PTMA_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>


#define	PTMA		pthread_mutexattr_t


#if	(! defined(PTMA_MASTER)) || (PTMA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptma_create(PTMA *) ;
extern int	ptma_init(PTMA *) ;
extern int	ptma_destroy(PTMA *) ;
extern int	ptma_getprioceiling(PTMA *,int *) ;
extern int	ptma_setprioceiling(PTMA *,int) ;
extern int	ptma_getprotocol(PTMA *,int *) ;
extern int	ptma_setprotocol(PTMA *,int) ;
extern int	ptma_getpshared(PTMA *,int *) ;
extern int	ptma_setpshared(PTMA *,int) ;
extern int	ptma_getrobustnp(PTMA *,int *) ;
extern int	ptma_setrobustnp(PTMA *,int) ;
extern int	ptma_gettype(PTMA *,int *) ;
extern int	ptma_settype(PTMA *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTMA_MASTER */

#endif /* PTMA_INCLUDE */


