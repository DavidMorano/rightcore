/* ctbin */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTBIN_INCLUDE
#define	CTBIN_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int ctbin(char *,int,int) ;

extern int ctbini(char *,int,int) ;
extern int ctbinl(char *,int,long) ;
extern int ctbinll(char *,int,longlong) ;

extern int ctbinui(char *,int,uint) ;
extern int ctbinul(char *,int,ulong) ;
extern int ctbinull(char *,int,ulonglong) ;

#ifdef	__cplusplus
}
#endif

#endif /* CTBIN_INCLUDE	*/


