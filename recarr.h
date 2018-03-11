/* RECARR */

/* record-array management */


/* revision history:

	= 1999-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	RECARR_INCLUDE
#define	RECARR_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* object defines */

#define	RECARR		struct recarr_head
#define	RECARR_FL	struct recarr_flags

/* options */

#define	RECARR_ODEFAULT		0x0000
#define	RECARR_OREUSE		0x0001		/* reuse empty slots */
#define	RECARR_OCOMPACT		0x0002		/* means NOHOLES */
#define	RECARR_OSWAP		0x0004		/* use swapping */
#define	RECARR_OSTATIONARY	0x0008		/* entries should not move */
#define	RECARR_OCONSERVE	0x0010		/* conserve space */
#define	RECARR_OSORTED		0x0020		/* keep sorted */
#define	RECARR_OORDERED		0x0040		/* keep ordered */

/* policy modes (Vector String Policy) */

#define	RECARR_PDEFAULT		0		/* default */
#define	RECARR_PHOLES		0		/* leave holes */
#define	RECARR_PSWAP		RECARR_OSWAP	/* swap entries */
#define	RECARR_PREUSE		RECARR_OREUSE	/* reuse empty entries */
#define	RECARR_PORDERED		RECARR_OORDERED	/* keep them ordered */
#define	RECARR_PSORTED		RECARR_OSORTED	/* keep them sorted */
#define	RECARR_PNOHOLES		RECARR_OCOMPACT	/* swap entries */
#define	RECARR_PCONSERVE	RECARR_OCONSERVE	/* conserve space */


struct recarr_flags {
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

struct recarr_head {
	const void	**va ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free entry index */
	RECARR_FL	f ;
} ;


typedef	struct recarr_head	recarr ;


#if	(! defined(RECARR_MASTER)) || (RECARR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int recarr_start(RECARR *,int,int) ;
extern int recarr_add(RECARR *,const void *) ;
extern int recarr_count(RECARR *) ;
extern int recarr_sort(RECARR *, int (*)()) ;
extern int recarr_setsorted(RECARR *) ;
extern int recarr_get(RECARR *,int,const void *) ; 
extern int recarr_getlast(RECARR *,const void *) ; 
extern int recarr_ent(RECARR *,const void *) ;
extern int recarr_search(RECARR *,const void *,
		int (*)(const void **,const void **), void *) ;
extern int recarr_del(RECARR *,int) ;
extern int recarr_delhand(RECARR *,const void *) ;
extern int recarr_delall(RECARR *) ;
extern int recarr_getvec(RECARR *,void *) ;
extern int recarr_extent(RECARR *) ;
extern int recarr_audit(RECARR *) ;
extern int recarr_finish(RECARR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(RECARR_MASTER)) || (RECARR_MASTER == 0) */

#endif /* RECARR_INCLUDE */


