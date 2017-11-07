/* sighand */

/* little object to block signals */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SIGHAND_INCLUDE
#define	SIGHAND_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	SIGHAND_MAGIC	0x66938271
#define	SIGHAND		struct sighand_head
#define	SIGHAND_HANDLE	struct sighand_handle


typedef void (*sighand_handler)(int,siginfo_t *,void *) ;


struct sighand_handle {
	struct sigaction	action ;
	int			sig ;
} ;

struct sighand_head {
	uint		magic ;
	sigset_t	osm ;
	SIGHAND_HANDLE	*handles ;
	int		nhandles ;
	int		nblocks ;
} ;


#if	(! defined(SIGHAND_MASTER)) || (SIGHAND_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sighand_start(SIGHAND *,const int *,const int *,const int *,
		sighand_handler) ;
extern int sighand_finish(SIGHAND *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGHAND_MASTER */

#endif /* SIGHAND_INCLUDE */


