/* cthex */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTHEX_INCLUDE
#define	CTHEX_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cthex(char *,int,int) ;

extern int cthexc(char *,int,int) ;
extern int cthexs(char *,int,int) ;
extern int cthexi(char *,int,int) ;
extern int cthexl(char *,int,long) ;
extern int cthexll(char *,int,longlong) ;

extern int cthexuc(char *,int,uint) ;
extern int cthexus(char *,int,uint) ;
extern int cthexui(char *,int,uint) ;
extern int cthexul(char *,int,ulong) ;
extern int cthexull(char *,int,ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTHEX_INCLUDE	*/


