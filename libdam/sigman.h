/* sigman */

/* little object to block signals */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SIGMAN_INCLUDE
#define	SIGMAN_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	SIGMAN_MAGIC	0x66938271
#define	SIGMAN		struct sigman_head
#define	SIGMAN_HANDLE	struct sigman_handle


typedef void (*sigmanhand_t)(int) ;


struct sigman_handle {
	struct sigaction	action ;
	int			sig ;
} ;

struct sigman_head {
	uint		magic ;
	sigset_t	osm ;
	SIGMAN_HANDLE	*handles ;
	int		nhandles ;
	int		nblocks ;
} ;


#if	(! defined(SIGMAN_MASTER)) || (SIGMAN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sigman_start(SIGMAN *,
			const int *,const int *,const int *,sigmanhand_t) ;
extern int	sigman_finish(SIGMAN *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGMAN_MASTER */

#endif /* SIGMAN_INCLUDE */


