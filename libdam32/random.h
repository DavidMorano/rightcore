/* random */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RANDOM_INCLUDE
#define	RANDOM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	RANDOM		struct random_head


struct random_head {
	ULONG		*fptr ;
	ULONG		*rptr ;
	ULONG		*end_ptr ;
	ULONG		state[64] ;
	int		rand_type ;
	int		rand_deg ;
	int		rand_sep ;
} ;


#if	(! defined(RANDOM_MASTER)) || (RANDOM_MASTER == 0)
 
#ifdef	__cplusplus
extern "C" {
#endif

extern int	random_start(RANDOM *,int,uint) ;
extern int	random_finish(RANDOM *) ;
extern int	random_getuint(RANDOM *,uint *) ;
extern int	random_getulong(RANDOM *,ULONG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(RANDOM_MASTER)) || (RANDOM_MASTER == 0) */

#endif /* RANDOM_INCLUDE */


