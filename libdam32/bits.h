/* bits (BitArray) */


/* revision history:

	= 1998-02-15, David A­D­ Morano
	This code was started.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BITS_INCLUDE
#define	BITS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* local object defines */

#define	BITS		struct bits_head


struct bits_head {
	ULONG		*a ;
	int		n ;		/* bits addressed */
	int		nbits ;		/* allocated */
	int		nwords ;	/* allocated */
} ;


#if	(! defined(BITS_MASTER)) || (BITS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bits_start(BITS *,int) ;
extern int	bits_set(BITS *,int) ;
extern int	bits_clear(BITS *,int) ;
extern int	bits_test(BITS *,int) ;
extern int	bits_anyset(BITS *) ;
extern int	bits_ffbs(BITS *) ;
extern int	bits_extent(BITS *) ;
extern int	bits_count(BITS *) ;
extern int	bits_finish(BITS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BITS_MASTER */

#endif /* BITS_INCLUDE */


