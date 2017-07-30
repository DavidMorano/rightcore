/* intceil */
/* Integer Ceiling */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	INTCEIL_INCLUDE
#define	INTCEIL_INCLUDE	 1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern int iceil(int,int) ;
extern long lceil(long,int) ;
extern LONG llceil(LONG,int) ;
extern uint uceil(uint,int) ;
extern ulong ulceil(ulong,int) ;
extern ULONG ullceil(ULONG,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTCEIL_INCLUDE */


