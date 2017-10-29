/* lookaside */
/* lookaside memory allocation manager */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LOOKASIDE_INCLUDE
#define	LOOKASIDE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<pq.h>


/* object defines */

#define	LOOKASIDE		struct lookaside_head
#define	LOOKASIDE_MINENTRIES	4


struct lookaside_head {
	caddr_t		eap ;		/* entry allocation pointer base */
	PQ		cq ;		/* chunk list */
	PQ		estack ;	/* stack of free blocks */
	int		nchunks ;	/* number of chunks allocated */
	int		esize ;		/* entry size */
	int		eaoff ;		/* entry-array offset */
	int		n ;		/* entries per chunk */
	int		i ;		/* current chunk usage (index) */
	int		nfree ;		/* total free entries */
	int		nused ;		/* total entries used */
} ;


#if	(! defined(LOOKASIDE_MASTER)) || (LOOKASIDE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int lookaside_start(LOOKASIDE *,int,int) ;
extern int lookaside_get(LOOKASIDE *,void *) ;
extern int lookaside_release(LOOKASIDE *,void *) ;
extern int lookaside_count(LOOKASIDE *) ;
extern int lookaside_finish(LOOKASIDE *) ;
extern int lookaside_audit(LOOKASIDE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(LOOKASIDE_MASTER)) || (LOOKASIDE_MASTER == 0) */

#endif /* LOOKASIDE_INCLUDE */


