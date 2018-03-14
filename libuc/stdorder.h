/* stdorder */


/* revision history:

	= 2001-03-24, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	STDORDER_INCLUDE
#define	STDORDER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* object define */

#define	STDORDER	char *


#if	(! defined(STDORDER_MASTER)) || (STDORDER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int stdorder_rchar(void *,char *) ;
extern int stdorder_rshort(void *,short *) ;
extern int stdorder_rint(void *,int *) ;
extern int stdorder_rlong(void *,long *) ;
extern int stdorder_rll(void *,LONG *) ;

#if	(LONG_BIT == 64)
#define	stdorder_rlong64(a,b)	stdorder_rlong((a),(b))
#else
#define	stdorder_rlong64(a,b)	stdorder_rll((a),(b))
#endif

extern int stdorder_ruchar(void *,uchar *) ;
extern int stdorder_rushort(void *,ushort *) ;
extern int stdorder_ruint(void *,uint *) ;
extern int stdorder_rulong(void *,ulong *) ;
extern int stdorder_rull(void *,ULONG *) ;

#if	(LONG_BIT == 64)
#define	stdorder_rulong64(a,b)	stdorder_rulong((a),(b))
#else
#define	stdorder_rulong64(a,b)	stdorder_rull((a),(b))
#endif

extern int stdorder_wchar(void *,int) ;
extern int stdorder_wshort(void *,int) ;
extern int stdorder_wint(void *,int) ;
extern int stdorder_wlong(void *,long) ;
extern int stdorder_wll(void *,LONG) ;

#if	(LONG_BIT == 64)
#define	stdorder_wlong64(a,b)	stdorder_wlong((a),(b))
#else
#define	stdorder_wlong64(a,b)	stdorder_wll((a),(b))
#endif

#define	stdorder_wuchar(A,B)	stdorder_wchar((A),(B))
#define	stdorder_wushort(A,B)	stdorder_wshort((A),(B))
#define	stdorder_wuint(A,B)	stdorder_wint((A),(B))
#define	stdorder_wulong(A,B)	stdorder_wlong((A),(B))
#define	stdorder_wull(A,B)	stdorder_wll((A),(B))

#if	(LONG_BIT == 64)
#define	stdorder_wulong64(a,b)	stdorder_wulong((a),(b))
#else
#define	stdorder_wulong64(a,b)	stdorder_wull((a),(b))
#endif

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(STDORDER_MASTER) || (STDORDER_MASTER == 0) */

#endif /* STDORDER_INCLUDE */


