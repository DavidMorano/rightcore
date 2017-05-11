/* cpq */
/* Circular-Pointer-Queue */

/* regular (no-frills) pointer queue */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This code was modeled after assembly.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CPQ_INCLUDE
#define	CPQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#define	CPQ		struct cpq_head
#define	CPQ_ENT		struct cpq_ent


struct cpq_ent {
	struct cpq_ent	*next ;
	struct cpq_ent	*prev ;
} ;

struct cpq_head {
	struct cpq_ent	*next ;
	struct cpq_ent	*prev ;
} ;


#if	(! defined(CPQ_MASTER)) || (CPQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cpq_start(CPQ *) ;
extern int cpq_ins(CPQ *,CPQ_ENT *) ;
extern int cpq_insgroup(CPQ *,CPQ_ENT *,int,int) ;
extern int cpq_rem(CPQ *,CPQ_ENT **) ;
extern int cpq_remtail(CPQ *,CPQ_ENT **) ;
extern int cpq_gettail(CPQ *,CPQ_ENT **) ;
extern int cpq_audit(CPQ *) ;
extern int cpq_finish(CPQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CPQ_MASTER */

#endif /* CPQ_INCLUDE */


