/* plainq */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PLAINQ_INCLUDE
#define	PLAINQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	PLAINQ		struct plainq_head
#define	PLAINQ_ENT	struct plainq_ent


struct plainq_ent {
	long		next ;
	long		prev ;
} ;

struct plainq_head {
	long		head ;
	long		tail ;
	uint		magic ;
	uint		count ;
} ;


#if	(! defined(PLAINQ_MASTER)) || (PLAINQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int plainq_start(PLAINQ *) ;
extern int plainq_finish(PLAINQ *) ;
extern int plainq_ins(PLAINQ *,PLAINQ_ENT *) ;
extern int plainq_insgroup(PLAINQ *,PLAINQ_ENT *,int,int) ;
extern int plainq_unlink(PLAINQ *,PLAINQ_ENT *) ;
extern int plainq_rem(PLAINQ *,PLAINQ_ENT **) ;
extern int plainq_gethead(PLAINQ *,PLAINQ_ENT **) ;
extern int plainq_remtail(PLAINQ *,PLAINQ_ENT **) ;
extern int plainq_gettail(PLAINQ *,PLAINQ_ENT **) ;
extern int plainq_count(PLAINQ *) ;
extern int plainq_audit(PLAINQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(PLAINQ_MASTER)) || (PLAINQ_MASTER == 0) */

#endif /* PLAINQ_INCLUDE */


