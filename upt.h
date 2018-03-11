/* upt */
/* UNIX® POSIX Thread manipulation */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UPT_INCLUDE
#define	UPT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<pta.h>
#include	<localmisc.h>


typedef int (*uptsub_t)(void *) ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int uptcreate(pthread_t *,pthread_attr_t *,int (*)(void *),void *) ;
extern int uptexit(int) ;
extern int uptonce(pthread_once_t *,void (*)()) ;
extern int uptjoin(pthread_t,int *) ;
extern int uptdetach(pthread_t) ;
extern int uptcancel(pthread_t) ;
extern int uptkill(pthread_t,int) ;
extern int uptsetschedparam(pthread_t,int,struct sched_param *) ;
extern int uptgetschedparam(pthread_t,int *,struct sched_param *) ;
extern int uptgetconcurrency() ;
extern int uptsetconcurrency(int) ;
extern int uptsetcancelstate(int,int *) ;
extern int uptsetcanceltype(int,int *) ;
extern int upttestcancel() ;
extern int uptequal(pthread_t,pthread_t) ;
extern int uptself(pthread_t *) ;
extern int uptatfork(void (*)(),void (*)(),void (*)()) ;
extern int uptncpus(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* UPT_INCLUDE */


