/* vecitem */


/* revision history:

	= 1999-07-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	VECITEM_INCLUDE
#define	VECITEM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* for 'uint' */


/* object defines */

#define	VECITEM			struct vecitem_head
#define	VECITEM_FL		struct vecitem_flags
#define	VECITEM_CUR		struct vecitem_c
#define	VECITEM_DEFENTS		10

/* options */

#define	VECITEM_ODEFAULT	0x0000
#define	VECITEM_OREUSE		0x0001
#define	VECITEM_OCOMPACT	0x0002		/* means NOHOLES */
#define	VECITEM_OSWAP		0x0004
#define	VECITEM_OSTATIONARY	0x0008
#define	VECITEM_OCONSERVE	0x0010
#define	VECITEM_OSORTED		0x0020
#define	VECITEM_OORDERED	0x0040

/* policy modes (Vector String Policy) */

#define	VECITEM_PDEFAULT	0		/* default */
#define	VECITEM_PHOLES		0		/* leave holes */
#define	VECITEM_PSWAP		VECITEM_OSWAP	/* swap entries */
#define	VECITEM_PREUSE		VECITEM_OREUSE	/* reuse empty entries */
#define	VECITEM_PORDERED	VECITEM_OORDERED	/* keep them ordered */
#define	VECITEM_PSORTED		VECITEM_OSORTED	/* keep them sorted */
#define	VECITEM_PNOHOLES	VECITEM_OCOMPACT	/* swap entries */
#define	VECITEM_PCONSERVE	VECITEM_OCONSERVE /* conserve space */


struct vecitem_flags {
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

struct vecitem_head {
	void		**va ;
	VECITEM_FL	f ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free index */
} ;

struct vecitem_c {
	int		i, c ;
} ;


typedef struct vecitem_head	vecitem ;
typedef struct vecitem_c	vecitem_cur ;


#if	(! defined(VECITEM_MASTER)) || (VECITEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecitem_start(vecitem *,int,int) ;
extern int vecitem_finish(vecitem *) ;
extern int vecitem_add(vecitem *,void *,int) ;
extern int vecitem_del(vecitem *,int) ;
extern int vecitem_count(vecitem *) ;
extern int vecitem_sort(vecitem *,int (*)()) ;
extern int vecitem_find(vecitem *,void *,int) ;
extern int vecitem_fetch(vecitem *,void *,vecitem_cur *,int (*)(),void *) ;
extern int vecitem_search(vecitem *,void *,int (*)(),void *) ;
extern int vecitem_get(vecitem *,int,void *) ;
extern int vecitem_getvec(vecitem *,void *) ;
extern int vecitem_audit(vecitem *) ;
extern int vecitem_curbegin(VECITEM *,VECITEM_CUR *) ;
extern int vecitem_curend(VECITEM *,VECITEM_CUR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECITEM_MASTER */

#endif /* VECITEM_INCLUDE */


