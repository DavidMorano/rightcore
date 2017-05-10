/* pta */

/* POSIX Thread Attribute manipulation */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTA_INCLUDE
#define	PTA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>


#define	PTA		pthread_attr_t

#ifndef	UINT
#define	UINT		unsigned int
#endif


#if	(! defined(PTA_MASTER)) || (PTA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pta_create(PTA *) ;
extern int	pta_init(PTA *) ;
extern int	pta_destroy(PTA *) ;
extern int	pta_setstacksize(PTA *,size_t) ;
extern int	pta_getstacksize(PTA *,size_t *) ;
extern int	pta_setguardsize(PTA *,size_t) ;
extern int	pta_getguardsize(PTA *,size_t *) ;
extern int	pta_setstackaddr(PTA *,void *) ;
extern int	pta_getstackaddr(PTA *,void **) ;
extern int	pta_setdetachstate(PTA *,int) ;
extern int	pta_getdetachstate(PTA *,int *) ;
extern int	pta_setscope(PTA *,int) ;
extern int	pta_getscope(PTA *,int *) ;
extern int	pta_setinheritsched(PTA *,int) ;
extern int	pta_getinheritsched(PTA *,int *) ;
extern int	pta_setschedpolicy(PTA *,int) ;
extern int	pta_getschedpolicy(PTA *,int *) ;
extern int	pta_setschedparam(PTA *,const struct sched_param *) ;
extern int	pta_getschedparam(PTA *,struct sched_param *) ;
extern int	pta_setstack(PTA *,void *,size_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTA_MASTER */

#endif /* PTA_INCLUDE */


