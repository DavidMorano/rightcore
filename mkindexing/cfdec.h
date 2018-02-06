/* cfdec */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFDEC_INCLUDE
#define	CFDEC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int cfdec(const char *,int,int *) ;

extern int cfdeci(const char *,int,int *) ;
extern int cfdecl(const char *,int,long *) ;
extern int cfdecll(const char *,int,longlong *) ;

extern int cfdecui(const char *,int,uint *) ;
extern int cfdecul(const char *,int,ulong *) ;
extern int cfdecull(const char *,int,ulonglong *) ;

extern int cfdecmfi(const char *,int,int *) ;
extern int cfdecmfl(const char *,int,long *) ;
extern int cfdecmfll(const char *,int,longlong *) ;

extern int cfdecmfui(const char *,int,uint *) ;
extern int cfdecmful(const char *,int,ulong *) ;
extern int cfdecmfull(const char *,int,ulonglong *) ;

extern int cfdecf(const char *,int,double *) ;

extern int cfdecti(const char *,int,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFDEC_INCLUDE	*/


