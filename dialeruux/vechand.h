/* vechand */

/* vector list structure (Vector Handle) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VECHAND_INCLUDE
#define	VECHAND_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


/* object defines */

#define	VECHAND		struct vechand_head
#define	VECHAND_FL	struct vechand_flags
#define	VECHAND_DEFENTS	10

/* options */

#define	VECHAND_ODEFAULT	0x0000
#define	VECHAND_OREUSE		0x0001		/* reuse empty slots */
#define	VECHAND_OCOMPACT	0x0002		/* means NOHOLES */
#define	VECHAND_OSWAP		0x0004		/* use swapping */
#define	VECHAND_OSTATIONARY	0x0008		/* entries should not move */
#define	VECHAND_OCONSERVE	0x0010		/* conserve space */
#define	VECHAND_OSORTED		0x0020		/* keep sorted */
#define	VECHAND_OORDERED	0x0040		/* keep ordered */

/* policy modes (Vector String Policy) */

#define	VECHAND_PDEFAULT	0		/* default */
#define	VECHAND_PHOLES		0		/* leave holes */
#define	VECHAND_PSWAP		VECHAND_OSWAP	/* swap entries */
#define	VECHAND_PREUSE		VECHAND_OREUSE	/* reuse empty entries */
#define	VECHAND_PORDERED	VECHAND_OORDERED	/* keep them ordered */
#define	VECHAND_PSORTED		VECHAND_OSORTED	/* keep them sorted */
#define	VECHAND_PNOHOLES	VECHAND_OCOMPACT	/* swap entries */
#define	VECHAND_PCONSERVE	VECHAND_OCONSERVE	/* conserve space */


struct vechand_flags {
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

struct vechand_head {
	const void	**va ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free entry index */
	VECHAND_FL	f ;
} ;


typedef struct vechand_head	vechand ;


#if	(! defined(VECHAND_MASTER)) || (VECHAND_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vechand_start(vechand *,int,int) ;
extern int vechand_add(vechand *,const void *) ;
extern int vechand_count(vechand *) ;
extern int vechand_sort(vechand *, int (*)()) ;
extern int vechand_setsorted(vechand *) ;
extern int vechand_get(vechand *,int,const void *) ; 
extern int vechand_getlast(vechand *,const void *) ; 
extern int vechand_ent(vechand *,const void *) ;
extern int vechand_search(vechand *,const void *,
		int (*)(const void **,const void **), void *) ;
extern int vechand_issorted(vechand *) ;
extern int vechand_del(vechand *,int) ;
extern int vechand_delhand(vechand *,const void *) ;
extern int vechand_delall(vechand *) ;
extern int vechand_getvec(vechand *,void *) ;
extern int vechand_extent(vechand *) ;
extern int vechand_audit(vechand *) ;
extern int vechand_finish(vechand *) ;

#ifdef	COMMENT
extern int vechand_finder(vechand *,void *,
	int (*)(const void *,const void *),void *) ;
#endif /* COMMENT */

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(VECHAND_MASTER)) || (VECHAND_MASTER == 0) */

#endif /* VECHAND_INCLUDE */


