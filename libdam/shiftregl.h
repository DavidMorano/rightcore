/* shiftregl */

/* Shift Register of Longs */


/* revision history:

	= 2001-07-10, David A­D­ Morano
	This code was started.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SHIFTREGL_INCLUDE
#define	SHIFTREGL_INCLUDE	1


#include	<envstandards.h>

#include	<vsystem.h>
#include	<localmisc.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


/* object defines */

#define	SHIFTREGL_MAGIC		0x74896233
#define	SHIFTREGL		struct shiftregl_head


struct shiftregl_head {
	uint		magic ;
	ULONG		*regs ;
	int		n ;
	int		i ;		/* next write index */
} ;


#if	(! defined(SHIFTREGL_MASTER)) || (SHIFTREGL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int shiftregl_start(SHIFTREGL *,int) ;
extern int shiftregl_finish(SHIFTREGL *) ;
extern int shiftregl_shiftin(SHIFTREGL *,ULONG) ;
extern int shiftregl_get(SHIFTREGL *,int,ULONG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SHIFTREGL_MASTER */

#endif /* SHIFTREGL_INCLUDE */


