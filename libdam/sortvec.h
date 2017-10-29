/* vecsorthand */

/* vector of sorted handles */


/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

#ifndef	VECSORTHAND_INCLUDE
#define	VECSORTHAND_INCLUDE	1


/* object defines */

#define	VECSORTHAND		struct vecsorthand_head


struct vecsorthand_head {
	int		(*cmpfunc)(const void *,const void *) ;
	void		**va ;
	int		i ;		/* highest index */
	int		e ;		/* extent of array */
	int		c ;		/* count of items in list */
} ;


typedef struct vecsorthand_head	vecsorthand ;


#ifndef	VECSORTHAND_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecsorthand_start(vecsorthand *,int,int (*)()) ;
extern int vecsorthand_add(vecsorthand *,void *) ;
extern int vecsorthand_finish(vecsorthand *) ;
extern int vecsorthand_get(vecsorthand *,int,void **) ;
extern int vecsorthand_del(vecsorthand *,int) ;
extern int vecsorthand_count(vecsorthand *) ;
extern int vecsorthand_search(vecsorthand *,void *,void **) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECSORTHAND_MASTER */

#endif /* VECSORTHAND_INCLUDE */


