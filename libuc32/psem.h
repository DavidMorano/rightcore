/* ucsem */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	PSEM_INCLUDE
#define	PSEM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<semaphore.h>


#define	PSEM		sem_t
#define	PSEM_NAMELEN	14


#if	(! defined(PSEM_MASTER)) || (PSEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	uc_openpsem(const char *,int,mode_t,uint,PSEM **) ;

extern int	psem_close(PSEM *) ;

extern int	psem_create(PSEM *,int,uint) ;
extern int	psem_destroy(PSEM *) ;
extern int	psem_count(PSEM *) ;
extern int	psem_getvalue(PSEM *,int *) ;
extern int	psem_wait(PSEM *) ;
extern int	psem_waiter(PSEM *,int) ;
extern int	psem_waiti(PSEM *) ;
extern int	psem_trywait(PSEM *) ;
extern int	psem_post(PSEM *) ;

extern int	psem_init(PSEM *,int,uint) ;

#ifdef	__cplusplus
}
#endif

#endif /* PSEM_MASTER */

#endif /* PSEM_INCLUDE */


