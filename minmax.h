/* minmax */
/* Minimum-Maximun */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MINMAX_INCLUDE
#define	MINMAX_INCLUDE	 1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#ifdef	__cplusplus
extern "C" {
#endif

int min(int a,int b)
int max(int a,int b)
long lmin(long a,long b)
long lmax(long a,long b)
longlong llmin(longlong a,longlong b)
longlong llmax(longlong a,longlong b)

#ifdef	__cplusplus
}
#endif

#endif /* MINMAX_INCLUDE */


