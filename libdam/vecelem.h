/* vecelem */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VECELEM_INCLUDE
#define	VECELEM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


/* object defines */

#define	VECELEM_MAGIC		0x73625198
#define	VECELEM			struct vecelem_head
#define	VECELEM_FL		struct vecelem_flags
#define	VECELEM_DEFENTS		2

/* options */

#define	VECELEM_ODEFAULT	0x0000
#define	VECELEM_OREUSE		0x0001		/* reuse empty slots */
#define	VECELEM_OCOMPACT	0x0002		/* means NOHOLES */
#define	VECELEM_OSWAP		0x0004		/* use swapping */
#define	VECELEM_OSTATIONARY	0x0008		/* entries should not move */
#define	VECELEM_OCONSERVE	0x0010		/* conserve space */
#define	VECELEM_OSORTED		0x0020		/* keep sorted */
#define	VECELEM_OORDERED	0x0040		/* keep ordered */

/* policy modes */

#define	VECELEM_PDEFAULT	0		/* default */
#define	VECELEM_PHOLES		0		/* leave holes */
#define	VECELEM_PSWAP		VECELEM_OSWAP	/* swap entries */
#define	VECELEM_PREUSE		VECELEM_OREUSE	/* reuse empty entries */
#define	VECELEM_PORDERED	VECELEM_OORDERED /* keep them ordered */
#define	VECELEM_PSORTED		VECELEM_OSORTED	/* keep them sorted */
#define	VECELEM_PNOHOLES	VECELEM_OCOMPACT /* keep them compacted */
#define	VECELEM_PCONSERVE	VECELEM_OCONSERVE /* conserve space */


struct vecelem_flags {
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

struct vecelem_head {
	uint		magic ;
	void		*va ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free index */
	int		esize ;		/* element size */
	VECELEM_FL	f ;
} ;


typedef struct vecelem_head	vecelem ;


#if	(! defined(VECELEM_MASTER)) || (VECELEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecelem_start(vecelem *,int,int,int) ;
extern int vecelem_finish(vecelem *) ;
extern int vecelem_add(vecelem *,const void *) ;
extern int vecelem_adduniq(vecelem *,const void *) ;
extern int vecelem_count(vecelem *) ;
extern int vecelem_sort(vecelem *,int (*)()) ;
extern int vecelem_setsorted(vecelem *) ;
extern int vecelem_search(vecelem *,int (*)()) ;
extern int vecelem_get(vecelem *,int,void *) ;
extern int vecelem_getval(vecelem *,int,void *) ;
extern int vecelem_getvec(vecelem *,void *) ;
extern int vecelem_audit(vecelem *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECELEM_MASTER */

#endif /* VECELEM_INCLUDE */


