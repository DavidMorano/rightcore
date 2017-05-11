/* ctdec */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTDEC_INCLUDE
#define	CTDEC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ctdec(char *,int,int) ;

extern int ctdeci(char *,int,int) ;
extern int ctdecl(char *,int,long) ;
extern int ctdecll(char *,int,longlong) ;

extern int ctdecui(char *,int,uint) ;
extern int ctdecul(char *,int,ulong) ;
extern int ctdecull(char *,int,ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTDEC_INCLUDE	*/


