/* cfoct */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFOCT_INCLUDE
#define	CFOCT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cfoct(const char *,int,int *) ;

extern int cfocti(const char *,int,int *) ;
extern int cfoctl(const char *,int,long *) ;
extern int cfoctll(const char *,int,longlong *) ;

extern int cfoctui(const char *,int,uint *) ;
extern int cfoctul(const char *,int,ulong *) ;
extern int cfoctull(const char *,int,ulonglong *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFOCT_INCLUDE	*/


