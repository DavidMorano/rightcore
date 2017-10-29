/* userlines */

/* TMPX-Interlocked */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage TMPX operations for multi-threaded use.


*******************************************************************************/


#ifndef	USERLINES_INCLUDE
#define	USERLINES_INCLUDE	1


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<ptm.h>
#include	<localmisc.h>


#define	USERLINES		struct userlines


struct userlines {
	TMPX		ut ;
	PTM		m ;
} ;


#if	(! defined(USERLINES_MASTER)) || (USERLINES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	userlines_start(USERLINES *) ;
extern int	userlines_add(USERLINES *,const char *,int) ;
extern int	userlines_remove(USERLINES *,char *,int) ;
extern int	userlines_finish(USERLINES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USERLINES_MASTER */

#endif /* USERLINES_INCLUDE */


