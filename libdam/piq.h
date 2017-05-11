/* piq */

/* container interlocked (pointer) queue */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PIQ_INCLUDE
#define	PIQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<pq.h>
#include	<localmisc.h>


#define	PIQ		struct piq_head
#define	PIQ_ENT		struct piq_ent
#define	PIQ_MAGIC	0x9635230


struct piq_ent {
	PIQ_ENT		*next ;
	PIQ_ENT		*prev ;
} ;

struct piq_head {
	uint		magic ;
	PTM		m ;
	PQ		frees ;
} ;


#if	(! defined(PIQ_MASTER)) || (PIQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int piq_start(PIQ *) ;
extern int piq_ins(PIQ *,void *) ;
extern int piq_rem(PIQ *,void *) ;
extern int piq_count(PIQ *) ;
extern int piq_audit(PIQ *) ;
extern int piq_finish(PIQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PIQ_MASTER */

#endif /* PIQ_INCLUDE */


