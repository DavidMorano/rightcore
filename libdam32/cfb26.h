/* cfb26 */

/* convert from base-26 */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CFB26_INCLUDE
#define	CFB26_INCLUDE	1


#if	(! defined(CFB26_MASTER)) || (CFB26_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cfb26(const char *,int,int *) ;

extern int cfb26i(const char *,int,int *) ;
extern int cfb26ui(const char *,int,long *) ;
extern int cfb26l(const char *,int,LONG *) ;

extern int cfb26ul(const char *,int,uint *) ;
extern int cfb26ll(const char *,int,ulong *) ;
extern int cfb26ull(const char *,int,ULONG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CFB26_MASTER */

#endif /* CFB26_INCLUDE */


