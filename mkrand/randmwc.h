/* randmwc */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RANDMWC_INCLUDE
#define	RANDMWC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	RANDMWC		struct randmwc_head


struct randmwc_head {
	uint		a ;
	ULONG		x, c ;
} ;


#if	(! defined(RANDMWC_MASTER)) || (RANDMWC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	randmwc_start(RANDMWC *,int,uint) ;
extern int	randmwc_finish(RANDMWC *) ;
extern int	randmwc_getulong(RANDMWC *,ULONG *) ;

#ifdef	__cplusplus
}
#endif

#endif

#endif /* RANDMWC_INCLUDE */


