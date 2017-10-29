/* bvsbook */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVSBOOK_INCLUDE
#define	BVSBOOK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	BVSBOOK		struct bvsbook_head


struct bvsbook_head {
	uchar		*ap ;		/* array of verses for each chapter */
	ushort		nverses ;	/* total for the whole book */
	ushort		nzverses ;	/* non-zero for the whole book */
	ushort		ci ;		/* chapter index */
	uchar		book ;
	uchar		al ;		/* also is the number of chapters */
} ;


#if	(! defined(BVSBOOK_MASTER)) || (BVSBOOK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvsbook_set(BVSBOOK *,ushort *) ;
extern int	bvsbook_get(BVSBOOK *,ushort *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVSBOOK_MASTER */

#endif /* BVSBOOK_INCLUDE */


