/* cfbin */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFBIN_INCLUDE
#define	CFBIN_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cfbin(const char *,int,int *) ;

extern int cfbini(const char *,int,int *) ;
extern int cfbinl(const char *,int,long *) ;
extern int cfbinll(const char *,int,longlong *) ;

extern int cfbinui(const char *,int,uint *) ;
extern int cfbinul(const char *,int,ulong *) ;
extern int cfbinull(const char *,int,ulonglong *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFBIN_INCLUDE	*/


