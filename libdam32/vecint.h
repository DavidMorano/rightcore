/* vecint */


/* revision history:

	= 1999-07-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	VECINT_INCLUDE
#define	VECINT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


/* object defines */

#define	VECINT_MAGIC		0x73625198
#define	VECINT			struct vecint_head
#define	VECINT_FL		struct vecint_flags
#define	VECINT_CUR		struct vecint_c
#define	VECINT_DEFENTS		2
#define	VECINT_TYPE		int

/* options */

#define	VECINT_ODEFAULT		0x0000
#define	VECINT_OREUSE		0x0001		/* reuse empty slots */
#define	VECINT_OCOMPACT		0x0002		/* means NOHOLES */
#define	VECINT_OSWAP		0x0004		/* use swapping */
#define	VECINT_OSTATIONARY	0x0008		/* entries should not move */
#define	VECINT_OCONSERVE	0x0010		/* conserve space */
#define	VECINT_OSORTED		0x0020		/* keep sorted */
#define	VECINT_OORDERED		0x0040		/* keep ordered */

/* policy modes */

#define	VECINT_PDEFAULT		0		/* default */
#define	VECINT_PHOLES		0		/* leave holes */
#define	VECINT_PSWAP		VECINT_OSWAP	/* swap entries */
#define	VECINT_PREUSE		VECINT_OREUSE	/* reuse empty entries */
#define	VECINT_PORDERED		VECINT_OORDERED	/* keep them ordered */
#define	VECINT_PSORTED		VECINT_OSORTED	/* keep them sorted */
#define	VECINT_PNOHOLES		VECINT_OCOMPACT	/* keep them compacted */
#define	VECINT_PCONSERVE	VECINT_OCONSERVE /* conserve space */


struct vecint_c {
	int		i ;
} ;

struct vecint_flags {
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

struct vecint_head {
	uint		magic ;
	VECINT_TYPE	*va ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free index */
	VECINT_FL	f ;
} ;


typedef struct vecint_head	vecint ;
typedef struct vecint_c		vecint_cur ;


#if	(! defined(VECINT_MASTER)) || (VECINT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecint_start(vecint *,int,int) ;
extern int vecint_finish(vecint *) ;
extern int vecint_add(vecint *,VECINT_TYPE) ;
extern int vecint_addlist(vecint *,const VECINT_TYPE *,int) ;
extern int vecint_adduniq(vecint *,VECINT_TYPE) ;
extern int vecint_insert(vecint *,int,VECINT_TYPE) ;
extern int vecint_assign(vecint *,int,VECINT_TYPE) ;
extern int vecint_resize(vecint *,int) ;
extern int vecint_del(vecint *,int) ;
extern int vecint_count(vecint *) ;
extern int vecint_extent(vecint *) ;
extern int vecint_sort(vecint *,int (*)()) ;
extern int vecint_setsorted(vecint *) ;
extern int vecint_find(vecint *,VECINT_TYPE) ;
extern int vecint_match(vecint *,VECINT_TYPE) ;
extern int vecint_search(vecint *,VECINT_TYPE (*)()) ;
extern int vecint_getval(vecint *,int,VECINT_TYPE *) ;
extern int vecint_getvec(vecint *,VECINT_TYPE **) ;
extern int vecint_mkvec(vecint *,VECINT_TYPE *) ;
extern int vecint_curbegin(vecint *,vecint_cur *) ;
extern int vecint_enum(vecint *,vecint_cur *,VECINT_TYPE *) ;
extern int vecint_curend(vecint *,vecint_cur *) ;
extern int vecint_audit(vecint *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECINT_MASTER */

#endif /* VECINT_INCLUDE */


