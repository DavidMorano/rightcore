/* ptca */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PTCA_INCLUDE
#define	PTCA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>


#define	PTCA		pthread_condattr_t

#ifndef	UINT
#define	UINT		unsigned int
#endif


#if	(! defined(PTCA_MASTER)) || (PTCA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ptca_create(PTCA *) ;
extern int	ptca_init(PTCA *) ;
extern int	ptca_destroy(PTCA *) ;
extern int	ptca_setpshared(PTCA *,int) ;
extern int	ptca_getpshared(PTCA *,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PTCA_MASTER */

#endif /* PTCA_INCLUDE */


