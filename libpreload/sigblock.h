/* sigblock */

/* little object to block signals */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SIGBLOCK_INCLUDE
#define	SIGBLOCK_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<localmisc.h>


#define	SIGBLOCK	struct sigblock_head


struct sigblock_head {
	sigset_t	osm ;
} ;


#if	(! defined(SIGBLOCK_MASTER)) || (SIGBLOCK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sigblock_start(SIGBLOCK *,const int *) ;
extern int	sigblock_finish(SIGBLOCK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGBLOCK_MASTER */

#endif /* SIGBLOCK_INCLUDE */


