/* pq */

/* regular (no-frills) pointer queue */


/* revision history:

	= 1998-03-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PQ_INCLUDE
#define	PQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


#define	PQ		struct pq_head
#define	PQ_ENT		struct pq_ent
#define	PQ_CUR		struct pq_cur


struct pq_cur {
	struct pq_ent	*entp ;
} ;

struct pq_ent {
	struct pq_ent	*next ;
	struct pq_ent	*prev ;
} ;

struct pq_head {
	struct pq_ent	*head ;
	struct pq_ent	*tail ;
	uint		count ;
} ;


#if	(! defined(PQ_MASTER)) || (PQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pq_start(PQ *) ;
extern int pq_ins(PQ *,PQ_ENT *) ;
extern int pq_insgroup(PQ *,PQ_ENT *,int,int) ;
extern int pq_gettail(PQ *,PQ_ENT **) ;
extern int pq_rem(PQ *,PQ_ENT **) ;
extern int pq_remtail(PQ *,PQ_ENT **) ;
extern int pq_unlink(PQ *,PQ_ENT *) ;
extern int pq_curbegin(PQ *,PQ_CUR *) ;
extern int pq_curend(PQ *,PQ_CUR *) ;
extern int pq_enum(PQ *,PQ_CUR *,PQ_ENT **) ;
extern int pq_count(PQ *) ;
extern int pq_audit(PQ *) ;
extern int pq_finish(PQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PQ_MASTER */

#endif /* PQ_INCLUDE */


