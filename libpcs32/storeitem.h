/* storeitem */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STOREITEM_INCLUDE
#define	STOREITEM_INCLUDE	1


/* object defines */

#define	STOREITEM		struct storeitem_head


struct storeitem_head {
	char		*dbuf ;
	int		dlen ;
	int		index ;
	int		f_overflow ;
} ;


#if	(! defined(STOREITEM_MASTER)) || (STOREITEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int storeitem_start(STOREITEM *,char *,int) ;
extern int storeitem_strw(STOREITEM *,const char *,int,cchar **) ;
extern int storeitem_buf(STOREITEM *,const void *,int,cchar **) ;
extern int storeitem_dec(STOREITEM *,int,cchar **) ;
extern int storeitem_char(STOREITEM *,int,cchar **) ;
extern int storeitem_nul(STOREITEM *,cchar **) ;
extern int storeitem_ptab(STOREITEM *,int,void ***) ;
extern int storeitem_block(STOREITEM *,int,int,void **) ;
extern int storeitem_getlen(STOREITEM *) ;
extern int storeitem_finish(STOREITEM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STOREITEM_MASTER */

#endif /* STOREITEM_INCLUDE */


