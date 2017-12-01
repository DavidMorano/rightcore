/* fifoitem */

/* FIFO container object */
/* last modified %G% version %I% */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FIFOITEM_INCLUDE
#define	FIFOITEM_INCLUDE	1


#define	FIFOITEM_MAGIC	0x12345678
#define	FIFOITEM	struct fifoitem_head
#define	FIFOITEM_ENT	struct fifoitem_ent
#define	FIFOITEM_CUR	struct fifoitem_cur


struct fifoitem_ent {
	FIFOITEM_ENT	*prev, *next ;
	int		dl ;
	const void	*dp ;
} ;

struct fifoitem_head {
	FIFOITEM_ENT	*head, *tail ;
	int		magic ;
	int		n ;
} ;

struct fifoitem_cur {
	FIFOITEM_ENT	*current ;
} ;


typedef struct fifoitem_head	fifoitem ;
typedef struct fifleitem_ent	fifoitem_ent ;
typedef struct fifoitem_cur	fifoitem_cur ;


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


