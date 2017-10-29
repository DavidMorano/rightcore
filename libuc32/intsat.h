/* intsat */
/* Integer Saturation Addition */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	INTSAT_INCLUDE
#define	INTSAT_INCLUDE	1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern int iaddsat(int,int) ;
extern long laddsat(long,long) ;
extern long long lladdsat(long long,long long) ;
extern uint uaddsat(uint,uint) ;
extern ulong uladdsat(ulong,ulong) ;
extern unsigned long long ulladdsat(unsigned long long,unsigned long long) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTSAT_INCLUDE */


