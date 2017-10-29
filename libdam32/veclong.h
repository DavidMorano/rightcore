/* veclong */


/* revision history:

	= 1999-07-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	VECLONG_INCLUDE
#define	VECLONG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* object defines */

#define	VECLONG_MAGIC		0x73625199
#define	VECLONG_DEFENTS		2
#define	VECLONG			struct veclong_head
#define	VECLONG_FL		struct veclong_flags
#define	VECLONG_CUR		struct veclong_c
#define	VECLONG_TYPE		LONG

/* options */

#define	VECLONG_ODEFAULT	0x0000
#define	VECLONG_OREUSE		0x0001		/* reuse empty slots */
#define	VECLONG_OCOMPACT	0x0002		/* means NOHOLES */
#define	VECLONG_OSWAP		0x0004		/* use swapping */
#define	VECLONG_OSTATIONARY	0x0008		/* entries should not move */
#define	VECLONG_OCONSERVE	0x0010		/* conserve space */
#define	VECLONG_OSORTED		0x0020		/* keep sorted */
#define	VECLONG_OORDERED	0x0040		/* keep ordered */

/* policy modes */

#define	VECLONG_PDEFAULT	0			/* default */
#define	VECLONG_PHOLES		0			/* leave holes */
#define	VECLONG_PSWAP		VECLONG_OSWAP		/* swap entries */
#define	VECLONG_PREUSE		VECLONG_OREUSE		/* reuse empties */
#define	VECLONG_PORDERED	VECLONG_OORDERED	/* keep them ordered */
#define	VECLONG_PSORTED		VECLONG_OSORTED		/* keep them sorted */
#define	VECLONG_PNOHOLES	VECLONG_OCOMPACT	/* swap entries */
#define	VECLONG_PCONSERVE	VECLONG_OCONSERVE 	/* conserve space */


struct veclong_c {
	int		i ;
} ;

struct veclong_flags {
	uint		issorted:1 ;
	uint		oreuse:1 ;
	uint		onoholes:1 ;
	uint		oswap:1 ;
	uint		ostationary:1 ;
	uint		ocompact:1 ;
	uint		osorted:1 ;
	uint		oordered:1 ;
	uint		oconserve:1 ;
} ;

struct veclong_head {
	uint		magic ;
	VECLONG_TYPE	*va ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free index */
	VECLONG_FL	f ;
} ;


typedef struct veclong_head	veclong ;
typedef struct veclong_c	veclong_cur ;


#if	(! defined(VECLONG_MASTER)) || (VECLONG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int veclong_start(veclong *,int,int) ;
extern int veclong_finish(veclong *) ;
extern int veclong_add(veclong *,VECLONG_TYPE) ;
extern int veclong_addlist(veclong *,const VECLONG_TYPE *,int) ;
extern int veclong_adduniq(veclong *,VECLONG_TYPE) ;
extern int veclong_insert(veclong *,VECLONG_TYPE) ;
extern int veclong_assign(veclong *,int,VECLONG_TYPE) ;
extern int veclong_resize(veclong *,int) ;
extern int veclong_del(veclong *,int) ;
extern int veclong_count(veclong *) ;
extern int veclong_sort(veclong *,int (*)()) ;
extern int veclong_setsorted(veclong *) ;
extern int veclong_find(veclong *,VECLONG_TYPE) ;
extern int veclong_match(veclong *,VECLONG_TYPE) ;
extern int veclong_search(veclong *,VECLONG_TYPE,int (*)()) ;
extern int veclong_getval(veclong *,int,VECLONG_TYPE *) ;
extern int veclong_getvec(veclong *,VECLONG_TYPE **) ;
extern int veclong_mkvec(veclong *,VECLONG_TYPE *) ;
extern int veclong_curbegin(veclong *,veclong_cur *) ;
extern int veclong_enum(veclong *,veclong_cur *,VECLONG_TYPE *) ;
extern int veclong_curend(veclong *,veclong_cur *) ;
extern int veclong_audit(veclong *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECLONG_MASTER */

#endif /* VECLONG_INCLUDE */


