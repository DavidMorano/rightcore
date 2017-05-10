/* subprocs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SUBPROCS_INCLUDE
#define	SUBPROCS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecint.h>
#include	<localmisc.h>


#define	SUBPROCS		struct subprocs_head
#define	SUBPROCS_MAGIC		0x86529873


struct subprocs_head {
	uint		magic ;
	int		pi ;
	VECINT		pids ;
} ;


#if	(! defined(SUBPROCS_MASTER)) || (SUBPROCS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int subprocs_start(SUBPROCS *) ;
extern int subprocs_finish(SUBPROCS *) ;
extern int subprocs_count(SUBPROCS *) ;
extern int subprocs_add(SUBPROCS *,pid_t) ;
extern int subprocs_poll(SUBPROCS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SUBPROCS_MASTER */


#endif /* SUBPROCS_INCLUDE */


