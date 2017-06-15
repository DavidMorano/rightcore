/* newobj */

/* new-object */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NEWOBJ_INCLUDE
#define	NEWOBJ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	newobj(otype,n)		(otype*)newobjsub((n),sizeof(otype))


#if	(! defined(NEWOBJ_MASTER)) || (NEWOBJ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern void	*newobjsub(int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* NEWOBJ_MASTER */

#endif /* NEWOBJ_INCLUDE */


