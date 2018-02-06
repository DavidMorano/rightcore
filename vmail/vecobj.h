/* vecobj */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VECOBJ_INCLUDE
#define	VECOBJ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<lookaside.h>
#include	<localmisc.h>


/* object defines */

#define	VECOBJ			struct vecobj_head
#define	VECOBJ_CUR		struct vecobj_c
#define	VECOBJ_FL		struct vecobj_flags
#define	VECOBJ_DEFENTS		10

/* options */

#define	VECOBJ_ODEFAULT		0x0000
#define	VECOBJ_OREUSE		0x0001		/* reuse empty slots */
#define	VECOBJ_OCOMPACT		0x0002		/* means NOHOLES */
#define	VECOBJ_OSWAP		0x0004		/* use swapping */
#define	VECOBJ_OSTATIONARY	0x0008		/* entries should not move */
#define	VECOBJ_OCONSERVE	0x0010		/* conserve space */
#define	VECOBJ_OSORTED		0x0020		/* keep sorted */
#define	VECOBJ_OORDERED		0x0040		/* keep ordered */

/* policy modes (Vector String Policy) */

#define	VECOBJ_PDEFAULT		0		/* default */
#define	VECOBJ_PHOLES		0		/* leave holes */
#define	VECOBJ_PSWAP		VECOBJ_OSWAP	/* swap entries */
#define	VECOBJ_PREUSE		VECOBJ_OREUSE	/* reuse empty entries */
#define	VECOBJ_PORDERED		VECOBJ_OORDERED	/* keep them ordered */
#define	VECOBJ_PSORTED		VECOBJ_OSORTED	/* keep them sorted */
#define	VECOBJ_PNOHOLES		VECOBJ_OCOMPACT	/* swap entries */
#define	VECOBJ_PCONSERVE	VECOBJ_OCONSERVE /* conserve space */


struct vecobj_flags {
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

struct vecobj_head {
	void		**va ;
	LOOKASIDE	la ;
	int		c ;		/* count of items in list */
	int		i ;		/* highest index */
	int		n ;		/* extent of array */
	int		fi ;		/* free index */
	int		esize ;		/* object size */
	VECOBJ_FL	f ;
} ;

struct vecobj_c {
	int		i, c ;
} ;


typedef struct vecobj_head	vecobj ;
typedef struct vecobj_c		vecobj_cur ;


#if	(! defined(VECOBJ_MASTER)) || (VECOBJ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecobj_start(vecobj *,int,int,int) ;
extern int vecobj_add(vecobj *,const void *) ;
extern int vecobj_adduniq(vecobj *,const void *) ;
extern int vecobj_addnew(vecobj *,void **) ;
extern int vecobj_inorder(vecobj *,void *,int (*)(),int) ;
extern int vecobj_del(vecobj *,int) ;
extern int vecobj_delall(vecobj *) ;
extern int vecobj_count(vecobj *) ;
extern int vecobj_sort(vecobj *,int (*)()) ;
extern int vecobj_setsorted(vecobj *) ;
extern int vecobj_find(vecobj *,void *) ;
extern int vecobj_curbegin(vecobj *,vecobj_cur *) ;
extern int vecobj_fetch(vecobj *,void *,vecobj_cur *,int (*)(),void *) ;
extern int vecobj_curend(vecobj *,vecobj_cur *) ;
extern int vecobj_search(vecobj *,void *,int (*)(),void *) ;
extern int vecobj_get(vecobj *,int,void *) ;
extern int vecobj_store(vecobj *,const void *,void *) ;
extern int vecobj_getvec(vecobj *,void *) ;
extern int vecobj_audit(vecobj *) ;
extern int vecobj_finish(vecobj *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECOBJ_MASTER */

#endif /* VECOBJ_INCLUDE */


