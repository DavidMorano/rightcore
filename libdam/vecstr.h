/* vecstr */

/* vector string structures (Vector String) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VECSTR_INCLUDE
#define	VECSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* object defines */

#define	VECSTR			struct vecstr_head
#define	VECSTR_FL		struct vecstr_flags
#define	VECSTR_DEFENTS		5

/* options */

#define	VECSTR_ODEFAULT		0x0000
#define	VECSTR_OREUSE		0x0001		/* reuse empty slots */
#define	VECSTR_OCOMPACT		0x0002		/* means NOHOLES */
#define	VECSTR_OSWAP		0x0004		/* use swapping */
#define	VECSTR_OSTATIONARY	0x0008		/* entries should not move */
#define	VECSTR_OCONSERVE	0x0010		/* conserve space */
#define	VECSTR_OSORTED		0x0020		/* keep sorted */
#define	VECSTR_OORDERED		0x0040		/* keep ordered */
#define	VECSTR_OSTSIZE		0x0080		/* maintain STSIZE */

/* policy modes (Vector String Policy) */

#define	VECSTR_PDEFAULT		0		/* default */
#define	VECSTR_PHOLES		0		/* leave holes */
#define	VECSTR_PSWAP		VECSTR_OSWAP	/* swap entries */
#define	VECSTR_PREUSE		VECSTR_OREUSE	/* reuse empty entries */
#define	VECSTR_PORDERED		VECSTR_OORDERED	/* keep them ordered */
#define	VECSTR_PSORTED		VECSTR_OSORTED	/* keep them sorted */
#define	VECSTR_PNOHOLES		VECSTR_OCOMPACT	/* swap entries */
#define	VECSTR_PCONSERVE	VECSTR_OCONSERVE /* conserve space */


struct vecstr_flags {
	uint		issorted:1 ;
	uint		oreuse:1 ;
	uint		onoholes:1 ;
	uint		oswap:1 ;
	uint		ostationary:1 ;
	uint		ocompact:1 ;
	uint		osorted:1 ;
	uint		oordered:1 ;
	uint		oconserve:1 ;
	uint		stsize:1 ;
} ;

struct vecstr_head {
	const char	**va ;
	VECSTR_FL	f ;
	int		c ;		/* count of items in list */
	int		i ;		/* overlast index */
	int		n ;		/* current extent of array */
	int		fi ;		/* free index */
	int		stsize ;	/* total string-table length */
} ;


typedef struct vecstr_head	vecstr ;


#if	(! defined(VECSTR_MASTER)) || (VECSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecstr_start(VECSTR *,int,int) ;
extern int vecstr_add(VECSTR *,const char *,int) ;
extern int vecstr_addkeyval(VECSTR *,const char *,int,const char *,int) ;
extern int vecstr_insert(VECSTR *,int,const char *,int) ;
extern int vecstr_store(VECSTR *,const char *,int,const char **) ;
extern int vecstr_get(VECSTR *,int,const char **) ;
extern int vecstr_getlast(VECSTR *,const char **) ;
extern int vecstr_del(VECSTR *,int) ;
extern int vecstr_delall(VECSTR *) ;
extern int vecstr_count(VECSTR *) ;
extern int vecstr_sort(VECSTR *,int (*)(const char **,const char **)) ;
extern int vecstr_search(VECSTR *,const char *,int (*)(),const char **) ;
extern int vecstr_find(VECSTR *,const char *) ;
extern int vecstr_findn(VECSTR *,const char *,int) ;
extern int vecstr_finder(VECSTR *,const char *,int (*)(),const char **) ;
extern int vecstr_findaddr(VECSTR *,const char *) ;
extern int vecstr_getvec(VECSTR *,const char ***) ;
extern int vecstr_strsize(VECSTR *) ;
extern int vecstr_strmk(VECSTR *,char *,int) ;
extern int vecstr_recsize(VECSTR *) ;
extern int vecstr_recmk(VECSTR *,int *,int) ;
extern int vecstr_recmkstr(VECSTR *,int *,int,char *,int) ;
extern int vecstr_audit(VECSTR *) ;
extern int vecstr_finish(VECSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECSTR_MASTER */

#endif /* VECSTR_INCLUDE */


