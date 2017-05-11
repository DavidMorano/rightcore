/* ctoct */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTOCT_INCLUDE
#define	CTOCT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ctoct(char *,int,int) ;

extern int ctocti(char *,int,int) ;
extern int ctoctl(char *,int,long) ;
extern int ctoctll(char *,int,longlong) ;

extern int ctoctui(char *,int,uint) ;
extern int ctoctul(char *,int,ulong) ;
extern int ctoctull(char *,int,ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTOCT_INCLUDE	*/


