/* pretime */

/* UNIX® program-data cache */


/* revision history:

	- 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	PRETIME_INCLUDE
#define	PRETIME_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/time.h>		/* |gettimeofday(3c)| */
#include	<sys/timeb.h>		/* |ftime(3c)| */


#if	(! defined(PRETIME_MASTER)) || (PRETIME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pretime_init() ;
extern int pretime_getoff(long *) ;
extern int pretime_modtime(time_t *) ;
extern int pretime_modtv(struct timeval *,void *) ;
extern int pretime_modts(struct timespec *) ;
extern int pretime_modtimeb(struct timeb *) ;
extern void pretime_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* PRETIME_MASTER */

#endif /* PRETIME_INCLUDE */


