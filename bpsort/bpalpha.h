/* bpalpha */


/* revision history:

	= 2001-03-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	BPALPHA_INCLUDE
#define	BPALPHA_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	BPALPHA_MAGIC		0x29456781
#define	BPALPHA			struct bpalpha_head
#define	BPALPHA_STATS		struct bpalpha_stats


/* statistics */
struct bpalpha_stats {
	uint			gpht ;		/* global length */
	uint			lbht ;		/* local history length */
	uint			lpht ;		/* local pattern length */
	uint			bits ;
} ;

struct bpalpha_head {
	unsigned long		magic ;
	struct bpalpha_stats	s ;
	uint			*lbht ;		/* local BHT */
	uchar			*lpht ;		/* local PHT */
	uchar			*cpht ;		/* choice PHT */
	uchar			*gpht ;		/* global PHT */
	uint			bhistory ;	/* global branch history */
	uint			lhlen ;		/* local history length */
	uint			lplen ;		/* local pattern length */
	uint			glen ;		/* global length */
	uint			historymask ;
} ;


#if	(! defined(BPALPHA_MASTER)) || (BPALPHA_MASTER == 0)

extern int	bpalpha_init(BPALPHA *,int,int,int) ;
extern int	bpalpha_lookup(BPALPHA *,uint) ;
extern int	bpalpha_confidence(BPALPHA *,uint) ;
extern int	bpalpha_update(BPALPHA *,uint,int) ;
extern int	bpalpha_zerostats(BPALPHA *) ;
extern int	bpalpha_stats(BPALPHA *,BPALPHA_STATS *) ;
extern int	bpalpha_free(BPALPHA *) ;

#endif /* BPALPHA_MASTER */

#endif /* BPALPHA_INCLUDE */


