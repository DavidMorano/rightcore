/* lockrw */

/* Reader-Writer Lock (LOCKRW) */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	LOCKRW_INCLUDE
#define	LOCKRW_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<pthread.h>

#include	<ptm.h>
#include	<ptc.h>
#include	<localmisc.h>		/* for unsigned types */


#define	LOCKRW		struct lockrw_head
#define	LOCKRW_MAGIC	0x26293176


struct lockrw_head {
	UINT		magic ;
	PTM		m ;
	PTC		c ;
	volatile int	readers ;
	volatile int	writers ;
	volatile int	waitwriters ;
	volatile int	waitreaders ;
} ;


#if	(! defined(LOCKRW_MASTER)) || (LOCKRW_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	lockrw_create(LOCKRW *,int) ;
extern int	lockrw_rdlock(LOCKRW *,int) ;
extern int	lockrw_wrlock(LOCKRW *,int) ;
extern int	lockrw_unlock(LOCKRW *) ;
extern int	lockrw_readers(LOCKRW *) ;
extern int	lockrw_destroy(LOCKRW *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOCKRW_MASTER */

#endif /* LOCKRW_INCLUDE */


