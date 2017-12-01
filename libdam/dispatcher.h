/* dispatcher */

/* dispatcher object */
/* last modified %G% version %I% */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is work dispatcher.  It dispatches work jobs to a pool
	of threads (created and managed by this object).


*******************************************************************************/


#ifndef	DISPATCHER_INCLUDE
#define	DISPATCHER_INCLUDE	1


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<ciq.h>
#include	<psem.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	DISPATCHER	struct dispatcher_head
#define	DISPATCHER_THR	struct dispatcher_thr

struct dispatcher_thr {
	pthread_t	tid ;
	uint		f_active ;
	volatile int	f_exiting ;
} ;

struct dispatcher_head {
	uint		magic ;
	PTM		m ;		/* object mutex */
	PTC		cond ;		/* condition variable */
	CIQ		wq ;		/* work Q */
	PSEM		ws ;		/* work semaphore */
	vecobj		tids ;
	void		*callsub ;	/* called subroutine entry-address */
	void		*callarg ;	/* called subroutine argument */
	DISPATCHER_THR	*threads ;
	volatile int	f_exit ;	/* CMD to exit immiedately */
	volatile int	f_done ;	/* CMD to exit after work completed */
	volatile int	f_wakeup ;	/* wait flag */
	volatile int	f_ready ;
	int		nthr ;		/* concurrency */
} ;


#if	(! defined(DISPATCHER_MASTER)) || (DISPATCHER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	dispatcher_start(DISPATCHER *,int,void *,void *) ;
extern int	dispatcher_add(DISPATCHER *,void *) ;
extern int	dispatcher_finish(DISPATCHER *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* DISPATCHER_MASTER */

#endif /* DISPATCHER_INCLUDE */


