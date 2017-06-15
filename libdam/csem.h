/* csem */

/* Counting-Semaphore (CSEM) */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	CSEM_INCLUDE
#define	CSEM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>

#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>		/* for unsigned types */


#define	CSEM		struct csem_head
#define	CSEM_MAGIC	0x26293175


struct csem_head {
	uint		magic ;
	PTM		m ;
	PTC		c ;
	volatile int	count ;		/* this is the real data! */
	volatile int	waiters ;	/* this is some extra */
} ;


#if	(! defined(CSEM_MASTER)) || (CSEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	csem_create(CSEM *,int,int) ;
extern int	csem_decr(CSEM *,int,int) ;
extern int	csem_incr(CSEM *,int) ;
extern int	csem_count(CSEM *) ;
extern int	csem_waiters(CSEM *) ;
extern int	csem_destroy(CSEM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CSEM_MASTER */

#endif /* CSEM_INCLUDE */


