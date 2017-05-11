/* sysmiscem */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISCEM_INCLUDE
#define	SYSMISCEM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>

#include	"modload.h"
#include	"sysmiscems.h"


#define	SYSMISCEM		struct sysmiscem_head
#define	SYSMISCEM_DATA		struct sysmiscem_d


struct sysmiscem_d {
	uint		intstale ;		/* "stale" interval */
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
} ;

struct sysmiscem_calls {
	int	(*open)(void *,const char *) ;
	int	(*get)(void *,time_t,int,SYSMISCEMS_DATA *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct sysmiscem_head {
	uint		magic ;
	MODLOAD		loader ;
	void		*obj ;		/* object pointer */
	struct sysmiscem_calls	call ;
} ;


#if	(! defined(SYSMISCEM_MASTER)) || (SYSMISCEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sysmiscem_open(SYSMISCEM *,const char *) ;
extern int	sysmiscem_get(SYSMISCEM *,time_t,int,SYSMISCEM_DATA *) ;
extern int	sysmiscem_close(SYSMISCEM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISCEM_MASTER */

#endif /* SYSMISCEM */


