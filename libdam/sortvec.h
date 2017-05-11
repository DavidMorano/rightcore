/* sortvec */

/* sorted list structures (Sorted List) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SORTVEC_INCLUDE
#define	SORTVEC_INCLUDE	1


/* object defines */

#define	SORTVEC		struct sortvec_head


struct sortvec_head {
	int		(*cmpfunc)(const void *,const void *) ;
	void		**va ;
	int		i ;		/* highest index */
	int		e ;		/* extent of array */
	int		c ;		/* count of items in list */
} ;


typedef struct sortvec_head	sortvec ;


#ifndef	SORTVEC_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int sortvec_start(sortvec *,int,int (*)()) ;
extern int sortvec_add(sortvec *,void *) ;
extern int sortvec_finish(sortvec *) ;
extern int sortvec_get(sortvec *,int,void **) ;
extern int sortvec_del(sortvec *,int) ;
extern int sortvec_count(sortvec *) ;
extern int sortvec_search(sortvec *,void *,void **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SORTVEC_MASTER */

#endif /* SORTVEC_INCLUDE */


