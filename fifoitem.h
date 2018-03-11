/* fifoitem */

/* FIFO container object */
/* last modified %G% version %I% */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FIFOITEM_INCLUDE
#define	FIFOITEM_INCLUDE	1


#define	FIFOITEM_MAGIC	0x12345678
#define	FIFOITEM	struct fifoitem_head
#define	FIFOITEM_ENT	struct fifoitem_e
#define	FIFOITEM_CUR	struct fifoitem_c


struct fifoitem_e {
	FIFOITEM_ENT	*prev, *next ;
	int		dl ;
	const void	*dp ;
} ;

struct fifoitem_head {
	FIFOITEM_ENT	*head, *tail ;
	int		magic ;
	int		n ;
} ;

struct fifoitem_c {
	FIFOITEM_ENT	*current ;
} ;


typedef FIFOITEM	fifoitem ;
typedef FIFOITEM_ENT	fifoitem_ent ;
typedef FIFOITEM_CUR	fifoitem_cur ;


#if	(! defined(FIFOITEM_MASTER)) || (FIFOITEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int fifoitem_start(fifoitem *) ;
extern int fifoitem_finish(fifoitem *) ;
extern int fifoitem_ins(fifoitem *,const void *,int) ;
extern int fifoitem_rem(fifoitem *,void *,int) ;
extern int fifoitem_count(fifoitem *) ;
extern int fifoitem_enum(fifoitem *,fifoitem_cur *,const void **) ;
extern int fifoitem_curbegin(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_curend(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_del(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_fetch(fifoitem *,fifoitem_cur *,fifoitem_ent **) ;
extern int fifoitem_finder(fifoitem *,const void *,int (*)(),const void **) ;

#ifdef	__cplusplus
}
#endif

#endif /* FIFOITEM_MASTER */

#endif /* FIFOITEM_INCLUDE */


