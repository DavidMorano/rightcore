/* sigign */

/* little object to block signals */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SIGIGN_INCLUDE
#define	SIGIGN_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	SIGIGN_MAGIC	0x66938271
#define	SIGIGN		struct sigign_head
#define	SIGIGN_HANDLE	struct sigign_handle


struct sigign_handle {
	struct sigaction	action ;
	int			sig ;
} ;

struct sigign_head {
	uint		magic ;
	sigset_t	osm ;
	SIGIGN_HANDLE	*handles ;
	int		nhandles ;
	int		nblocks ;
} ;


#if	(! defined(SIGIGN_MASTER)) || (SIGIGN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sigign_start(SIGIGN *,const int *) ;
extern int	sigign_finish(SIGIGN *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGIGN_MASTER */

#endif /* SIGIGN_INCLUDE */


