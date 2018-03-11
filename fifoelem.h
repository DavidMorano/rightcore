/* fifoelem */

/* FIFO container object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FIFOELEM_INCLUDE
#define	FIFOELEM_INCLUDE	1


#include	<sys/types.h>

#include	<localmisc.h>


#define	FIFOELEM_MAGIC	0x12345678
#define	FIFOELEM	struct fifoelem_head
#define	FIFOELEM_ENT	struct fifoelem_ent
#define	FIFOELEM_CURSOR	struct fifoelem_cur


struct fifoelem_ent {
	FIFOELEM_ENT	*previous, *next ; /* must be first! */
	int		slen ;
	void		*s ;
} ;


struct fifoelem_head {
	FIFOELEM_ENT	*head, *tail ; /* must be first! */
	uint		magic ;
	int		n ;
} ;


struct fifoelem_cur {
	FIFOELEM_ENT	*current ;
} ;


typedef struct fifoelem_head	fifoelem ;
typedef struct fifoelem_ent	fifoelem_ent ;
typedef struct fifoelem_cur	fifoelem_cur ;


#if	(! defined(FIFOELEM_MASTER)) || (FIFOELEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int fifoelem_start(fifoelem *) ;
extern int fifoelem_finish(fifoelem *) ;
extern int fifoelem_add(fifoelem *,void *,int) ;
extern int fifoelem_remove(fifoelem *,void *,int) ;
extern int fifoelem_count(fifoelem *) ;
extern int fifoelem_enum(fifoelem *,fifoelem_cur *,void *) ;
extern int fifoelem_curbegin(fifoelem *,fifoelem_cur *) ;
extern int fifoelem_curend(fifoelem *,fifoelem_cur *) ;
extern int fifoelem_del(fifoelem *,fifoelem_cur *) ;
extern int fifoelem_fetch(fifoelem *,fifoelem_cur *,fifoelem_ent **) ;

/*
extern int fifoelem_search(fifoelem *,char *,int (*)(),char **) ;
extern int fifoelem_find(fifoelem *,void *) ;
extern int fifoelem_finder(fifoelem *,char *,int (*)(),char **) ;
*/

#ifdef	__cplusplus
}
#endif

#endif /* FIFOELEM_MASTER */

#endif /* FIFOELEM_INCLUDE */


