/* density */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DENSITY_INCLUDE
#define	DENSITY_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	DENSITY_MAGIC	91827346
#define	DENSITY		struct density
#define	DENSITY_STATS	struct density_s


struct density {
	uint		magic ;
	ULONG		*a ;
	ULONG		c ;
	uint		max ;
	uint		ovf ;
	uint		len ;
} ;

struct density_s {
	double		mean, var ;
	ULONG		ovf ;
	ULONG		count ;
	uint		len ;
	uint		max ;
} ;


#if	(! defined(DENSITY_MASTER)) || (DENSITY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	density_start(DENSITY *,int) ;
extern int	density_update(DENSITY *,int) ;
extern int	density_slot(DENSITY *,int,ULONG *) ;
extern int	density_stats(DENSITY *,DENSITY_STATS *) ;
extern int	density_finish(DENSITY *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DENSITY_MASTER */

#endif /* DENSITY_INCLUDE */


