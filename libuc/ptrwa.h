/* ptrwa */

/* POSIX Thread read-write lock attribute manipulation */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTRWA_INCLUDE
#define	PTRWA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>


#define	PTRWA		pthread_rwlockattr_t

#ifndef	UINT
#define	UINT		unsigned int
#endif


#if	(! defined(PTRWA_MASTER)) || (PTRWA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptrwa_create(PTRWA *) ;
extern int	ptrwa_init(PTRWA *) ;
extern int	ptrwa_destroy(PTRWA *) ;
extern int	ptrwa_setpshared(PTRWA *,int) ;
extern int	ptrwa_getpshared(PTRWA *,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTRWA_MASTER */

#endif /* PTRWA_INCLUDE */


