/* netorder */


/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	NETORDER_INCLUDE
#define	NETORDER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* object define */

#define	NETORDER	char *


#if	(! defined(NETORDER_MASTER)) || (NETORDER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int netorder_rchar(void *,char *) ;
extern int netorder_rshort(void *,short *) ;
extern int netorder_rint(void *,int *) ;
extern int netorder_rlong(void *,long *) ;
extern int netorder_rll(void *,LONG *) ;

#if	(LONG_BIT == 64)
#define	netorder_rlong64(a,b)	netorder_rlong((a),(b))
#else
#define	netorder_rlong64(a,b)	netorder_rll((a),(b))
#endif

extern int netorder_ruchar(void *,uchar *) ;
extern int netorder_rushort(void *,ushort *) ;
extern int netorder_ruint(void *,uint *) ;
extern int netorder_rulong(void *,ulong *) ;
extern int netorder_rull(void *,ULONG *) ;

#if	(LONG_BIT == 64)
#define	netorder_rulong64(a,b)	netorder_rulong((a),(b))
#else
#define	netorder_rulong64(a,b)	netorder_rull((a),(b))
#endif

extern int netorder_wchar(void *,int) ;
extern int netorder_wshort(void *,int) ;
extern int netorder_wint(void *,int) ;
extern int netorder_wlong(void *,long) ;
extern int netorder_wll(void *,LONG) ;

#if	(LONG_BIT == 64)
#define	netorder_wlong64(a,b)	netorder_wlong((a),(b))
#else
#define	netorder_wlong64(a,b)	netorder_wll((a),(b))
#endif

#define	netorder_wuchar(A,B)	netorder_wchar((A),(B)) ;
#define	netorder_wushort(A,B)	netorder_wshort((A),(B)) ;
#define	netorder_wuint(A,B)	netorder_wint((A),(B)) ;
#define	netorder_wulong(A,B)	netorder_wlong((A),(B)) ;
#define	netorder_wull(A,B)	netorder_wll((A),(B)) ;

#if	(LONG_BIT == 64)
#define	netorder_wulong64(a,b)	netorder_wulong((a),(b))
#else
#define	netorder_wulong64(a,b)	netorder_wull((a),(b))
#endif


/* older API */


#define	netorder_reads(A,B)	netorder_rshort((A),(B)) ;
#define	netorder_readi(A,B)	netorder_rint((A),(B)) ;
#define	netorder_readl(A,B)	netorder_rlong((A),(B)) ;

#define	netorder_writes(A,B)	netorder_wshort((A),(B)) ;
#define	netorder_writei(A,B)	netorder_wint((A),(B)) ;
#define	netorder_writel(A,B)	netorder_wlong((A),(B)) ;


#ifdef	__cplusplus
}
#endif

#endif /* (! defined(NETORDER_MASTER)) || (NETORDER_MASTER == 0) */

#endif /* NETORDER_INCLUDE */


