/* cfnum */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFNUM_INCLUDE
#define	CFNUM_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cfnum(const char *,int,int *) ;

extern int cfnumi(const char *,int,int *) ;
extern int cfnuml(const char *,int,long *) ;
extern int cfnumll(const char *,int,longlong *) ;

extern int cfnumui(const char *,int,uint *) ;
extern int cfnumul(const char *,int,ulong *) ;
extern int cfnumull(const char *,int,ulonglong *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFNUM_INCLUDE	*/


