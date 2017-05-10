/* cfhex */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFHEX_INCLUDE
#define	CFHEX_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cfhex(const char *,int,int *) ;

extern int cfhexi(const char *,int,int *) ;
extern int cfhexl(const char *,int,long *) ;
extern int cfhexll(const char *,int,longlong *) ;

extern int cfhexui(const char *,int,uint *) ;
extern int cfhexul(const char *,int,ulong *) ;
extern int cfhexull(const char *,int,ulonglong *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFHEX_INCLUDE	*/


