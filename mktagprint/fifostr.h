/* fifostr */
/* FIFO string structures (FIFO String) */


/* revision history:

	= 1999-12-09, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-09-12, David A­D­ Morano
	Small interface change to |fifostr_entread()|.

*/

/* Copyright © 1999,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	FIFOSTR_INCLUDE
#define	FIFOSTR_INCLUDE	1


/* object defines */

#define	FIFOSTR_MAGIC	0x12345678
#define	FIFOSTR		struct fifostr_head
#define	FIFOSTR_CUR	struct fifostr_cur
#define	FIFOSTR_ENT	struct fifostr_e


struct fifostr_e {
	FIFOSTR_ENT	*next, *prev ;
	int		slen ;
} ;

struct fifostr_head {
	uint		magic ;
	FIFOSTR_ENT	*head, *tail ;
	int		ic ;		/* item count */
	int		cc ;		/* character count */
} ;

struct fifostr_cur {
	FIFOSTR_ENT	*current ;
} ;


typedef struct fifostr_head	fifostr ;
typedef struct fifostr_cur	fifostr_cur ;


#if	(! defined(FIFOSTR_MASTER)) || (FIFOSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int fifostr_start(fifostr *) ;
extern int fifostr_add(fifostr *,const char *,int) ;
extern int fifostr_headread(fifostr *,char *,int) ;
extern int fifostr_headlen(fifostr *) ;
extern int fifostr_entread(fifostr *,char *,int,int) ;
extern int fifostr_entlen(fifostr *,int) ;
extern int fifostr_remove(fifostr *,char *,int) ;
extern int fifostr_count(fifostr *) ;
extern int fifostr_curbegin(fifostr *,fifostr_cur *) ;
extern int fifostr_curend(fifostr *,fifostr_cur *) ;
extern int fifostr_enum(fifostr *,fifostr_cur *,char *,int) ;
extern int fifostr_del(fifostr *,fifostr_cur *) ;
extern int fifostr_finish(fifostr *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FIFOSTR_MASTER */

#endif /* FIFOSTR_INCLUDE */


