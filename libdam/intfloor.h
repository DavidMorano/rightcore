/* intfloor */
/* Integer Flooring */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	INTFLOOR_INCLUDE
#define	INTFLOOR_INCLUDE	 1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern int ifloor(int,int) ;
extern long lfloor(long,int) ;
extern LONG llfloor(LONG,int) ;
extern uint ufloor(uint,int) ;
extern ulong ulfloor(ulong,int) ;
extern ULONG ullfloor(ULONG,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTFLOOR_INCLUDE */


