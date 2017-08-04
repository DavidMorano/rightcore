/* intrem */
/* Integer Ceiling */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	INTREM_INCLUDE
#define	INTREM_INCLUDE	 1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern int irem(int,int) ;
extern long lrem(long,long) ;
extern LONG llrem(LONG,LONG) ;
extern uint urem(uint,uint) ;
extern ulong ulrem(ulong,ulong) ;
extern ULONG ullrem(ULONG,ULONG) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTREM_INCLUDE */


