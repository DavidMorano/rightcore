/* fifoitem */

/* FIFO container object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FIFOITEM_INCLUDE
#define	FIFOITEM_INCLUDE	1


#define	FIFOITEM_MAGIC	0x12345678
#define	FIFOITEM	struct fifoitem_head
#define	FIFOITEM_ENT	struct fifoitem_ent
#define	FIFOITEM_CUR	struct fifoitem_cur


struct fifoitem_ent {
	struct fifoitem_ent	*prev, *next ;
	int			dl ;
	const void		*dp ;
} ;

struct fifoitem_head {
	struct fifoitem_ent	*head, *tail ;
	int			magic ;
	int			n ;
} ;

struct fifoitem_cur {
	struct fifoitem_ent	*current ;
} ;


typedef struct fifoitem_head	fifoitem ;
typedef struct fifoitem_ent	fifoitem_ent ;
typedef struct fifoitem_cur	fifoitem_cur ;


#if	(! defined(FIFOITEM_MASTER)) || (FIFOITEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int fifoitem_start(fifoitem *) ;
extern int fifoitem_finish(fifoitem *) ;
extern int fifoitem_add(fifoitem *,const void *,int) ;
extern int fifoitem_remove(fifoitem *,void *,int) ;
extern int fifoitem_count(fifoitem *) ;
extern int fifoitem_enum(fifoitem *,fifoitem_cur *,const void **) ;
extern int fifoitem_curbegin(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_curend(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_del(fifoitem *,fifoitem_cur *) ;
extern int fifoitem_fetch(fifoitem *,fifoitem_cur *,fifoitem_ent **) ;
extern int fifoitem_finder(fifoitem *,const char *,int (*)(),const char **) ;

#ifdef	__cplusplus
}
#endif

#endif /* FIFOITEM_MASTER */

#endif /* FIFOITEM_INCLUDE */


