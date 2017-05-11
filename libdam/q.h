/* q */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	Q_INCLUDE
#define	Q_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ptm.h>


/* object defines */

#define	Q		struct q_head
#define	Q_ENT		struct q_ent

#define	Q_TPRIVATE	0		/* private to a process */
#define	Q_TSHARED	1		/* shared among processes */


struct q_ent {
	long		next ;
	long		prev ;
} ;

struct q_head {
	long		head ;
	long		tail ;
	PTM		lock ;
} ;


#if	(! defined(Q_MASTER)) || (Q_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int q_start(Q *,int) ;
extern int q_finish(Q *) ;
extern int q_ins(Q *,Q_ENT *) ;
extern int q_rem(Q *,Q_ENT **) ;

#ifdef	__cplusplus
}
#endif

#endif /* Q_MASTER */

#endif /* Q_INCLUDE */


