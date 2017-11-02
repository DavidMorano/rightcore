/* ciq */
/* Container-Interlocked-Queue */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This module was adapted from the PPI/LPPI OS code.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	CIQ_INCLUDE
#define	CIQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<pq.h>
#include	<localmisc.h>


#define	CIQ_MAGIC	0x9635230
#define	CIQ		struct ciq_head


struct ciq_head {
	uint		magic ;
	PTM		m ;
	PQ		frees ;
	PQ		fifo ;
} ;


#if	(! defined(CIQ_MASTER)) || (CIQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ciq_start(CIQ *) ;
extern int ciq_ins(CIQ *,void *) ;
extern int ciq_rem(CIQ *,void *) ;
extern int ciq_gettail(CIQ *,void *) ;
extern int ciq_remtail(CIQ *,void *) ;
extern int ciq_remhand(CIQ *,void *) ;
extern int ciq_count(CIQ *) ;
extern int ciq_audit(CIQ *) ;
extern int ciq_finish(CIQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CIQ_MASTER */

#endif /* CIQ_INCLUDE */


