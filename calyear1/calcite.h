/* calcite */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALCITE_INCLUDE
#define	CALCITE_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	CALCITE		struct calcite_head


struct calcite_head {
	ushort		y ;
	uchar		m, d ;
} ;


#if	(! defined(CALCITE_MASTER)) || (CALCITE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	calcite_load(CALCITE *,int,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALCITE_MASTER */

#endif /* CALCITE_INCLUDE */


