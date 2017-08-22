/* gspag */


/* revision history:

	= 2001-03-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	GSPAG_INCLUDE
#define	GSPAG_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	GSPAG_MAGIC	0x29656781
#define	GSPAG			struct gspag_head
#define	GSPAG_STATS		struct gspag_stats


/* more important defines */

#define	GSPAG_COUNTBITS	2	/* counter bits */


/* statistics */
struct gspag_stats {
	uint			gpht ;		/* global length */
	uint			lbht ;		/* local history length */
	uint			lpht ;		/* local pattern length */
	uint			bits ;
} ;

struct gspag_head {
	unsigned long		magic ;
	struct gspag_stats	s ;
	uint			*lbht ;		/* local BHT */
	uchar			*gpht ;		/* global PHT */
	uint			bhlen ;		/* BHT length */
	uint			phlen ;		/* GPHT length */
} ;


#if	(! defined(GSPAG_MASTER)) || (GSPAG_MASTER == 0)

extern int	gspag_init(GSPAG *,int,int) ;
extern int	gspag_lookup(GSPAG *,uint) ;
extern int	gspag_confidence(GSPAG *,uint) ;
extern int	gspag_update(GSPAG *,uint,int) ;
extern int	gspag_zerostats(GSPAG *) ;
extern int	gspag_stats(GSPAG *,GSPAG_STATS *) ;
extern int	gspag_free(GSPAG *) ;

#endif /* GSPAG_MASTER */

#endif /* GSPAG_INCLUDE */


