/* serialbuf */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SERIALBUF_INCLUDE
#define	SERIALBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* object define */

#define	SERIALBUF	struct serialbuf_head


struct serialbuf_head {
	char		*bp ;	/* current buffer pointer (changes) */
	int		len ;	/* supplied buffer length (doesn't change) */
	int		i ;	/* current buffer index (changes) */
} ;


#if	(! defined(SERIALBUF_MASTER)) || (SERIALBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int serialbuf_start(SERIALBUF *,char *,int) ;

extern int serialbuf_robj(SERIALBUF *,void *,int) ;

extern int serialbuf_rchar(SERIALBUF *,char *) ;
extern int serialbuf_rshort(SERIALBUF *,short *) ;
extern int serialbuf_rint(SERIALBUF *,int *) ;
extern int serialbuf_rinta(SERIALBUF *,int *,int) ;
extern int serialbuf_rlong(SERIALBUF *,long *) ;
extern int serialbuf_rlonga(SERIALBUF *,long *,int) ;
extern int serialbuf_rll(SERIALBUF *,LONG *) ;
extern int serialbuf_rstrw(SERIALBUF *,char *,int) ;
extern int serialbuf_rstrn(SERIALBUF *,char *,int) ;
extern int serialbuf_rbuf(SERIALBUF *,char *,int) ;

extern int serialbuf_ruchar(SERIALBUF *,uchar *) ;
extern int serialbuf_rushort(SERIALBUF *,ushort *) ;
extern int serialbuf_ruint(SERIALBUF *,uint *) ;
extern int serialbuf_ruinta(SERIALBUF *,uint *,int) ;
extern int serialbuf_rulong(SERIALBUF *,ulong *) ;
extern int serialbuf_rulonga(SERIALBUF *,ulong *,int) ;
extern int serialbuf_rull(SERIALBUF *,ULONG *) ;
extern int serialbuf_rustrw(SERIALBUF *,uchar *,int) ;
extern int serialbuf_rustrn(SERIALBUF *,uchar *,int) ;
extern int serialbuf_rubuf(SERIALBUF *,uchar *,int) ;

extern int serialbuf_wobj(SERIALBUF *,const void *,int) ;

extern int serialbuf_wchar(SERIALBUF *,int) ;
extern int serialbuf_wshort(SERIALBUF *,int) ;
extern int serialbuf_wint(SERIALBUF *,int) ;
extern int serialbuf_winta(SERIALBUF *,int *,int) ;
extern int serialbuf_wlong(SERIALBUF *,long) ;
extern int serialbuf_wlonga(SERIALBUF *,long *,int) ;
extern int serialbuf_wll(SERIALBUF *,LONG) ;
extern int serialbuf_wstrw(SERIALBUF *,const char *,int) ;
extern int serialbuf_wstrn(SERIALBUF *,const char *,int) ;
extern int serialbuf_wbuf(SERIALBUF *,const char *,int) ;

extern int serialbuf_wuchar(SERIALBUF *,uint) ;
extern int serialbuf_wushort(SERIALBUF *,uint) ;
extern int serialbuf_wuint(SERIALBUF *,uint) ;
extern int serialbuf_wuinta(SERIALBUF *,uint *,int) ;
extern int serialbuf_wulong(SERIALBUF *,ulong) ;
extern int serialbuf_wulonga(SERIALBUF *,ulong *,int) ;
extern int serialbuf_wull(SERIALBUF *,ULONG) ;
extern int serialbuf_wustrw(SERIALBUF *,const uchar *,int) ;
extern int serialbuf_wustrn(SERIALBUF *,const uchar *,int) ;
extern int serialbuf_wubuf(SERIALBUF *,const uchar *,int) ;

extern int serialbuf_adv(SERIALBUF *,int) ;

extern int serialbuf_getlen(SERIALBUF *) ;
extern int serialbuf_finish(SERIALBUF *) ;

#if	(LONG_BIT == 64)

#define	serialbuf_rlong64(a,b)	serialbuf_rlong((a),(b))
#define	serialbuf_rulong64(a,b)	serialbuf_rulong((a),(b))
#define	serialbuf_wlong64(a,b)	serialbuf_wlong((a),(b))
#define	serialbuf_wulong64(a,b)	serialbuf_wulong((a),(b))

#else

#define	serialbuf_rlong64(a,b)	serialbuf_rll((a),(b))
#define	serialbuf_rulong64(a,b)	serialbuf_rull((a),(b))
#define	serialbuf_wlong64(a,b)	serialbuf_wll((a),(b))
#define	serialbuf_wulong64(a,b)	serialbuf_wull((a),(b))

#endif /* (LONG_BIT == 64) */

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(SERIALBUF_MASTER) || (SERIALBUF_MASTER == 0) */

#endif /* SERIALBUF_INCLUDE */


