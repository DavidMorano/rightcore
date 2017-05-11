/* vecpstr */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VECPSTR_INCLUDE
#define	VECPSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vechand.h>
#include	<localmisc.h>


/* object defines */

#define	VECPSTR_MAGIC		0x88776215
#define	VECPSTR			struct vecpstr_head
#define	VECPSTR_CHUNK		struct vecpstr_chunk
#define	VECPSTR_FL		struct vecpstr_flags
#define	VECPSTR_DEFENTS		10

/* options */

#define	VECPSTR_ODEFAULT	0x0000
#define	VECPSTR_OREUSE		0x0001		/* reuse empty slots */
#define	VECPSTR_OCOMPACT	0x0002		/* means NOHOLES */
#define	VECPSTR_OSWAP		0x0004		/* use swapping */
#define	VECPSTR_OSTATIONARY	0x0008		/* entries should not move */
#define	VECPSTR_OCONSERVE	0x0010		/* conserve space */
#define	VECPSTR_OSORTED		0x0020		/* keep sorted */
#define	VECPSTR_OORDERED	0x0040		/* keep ordered */
#define	VECPSTR_OSTSIZE		0x0080		/* maintain STSIZE */

/* policy modes (Vector String Policy) */

#define	VECPSTR_PDEFAULT	0		/* default */
#define	VECPSTR_PHOLES		0		/* leave holes */
#define	VECPSTR_PSWAP		VECPSTR_OSWAP	/* swap entries */
#define	VECPSTR_PREUSE		VECPSTR_OREUSE	/* reuse empty entries */
#define	VECPSTR_PORDERED	VECPSTR_OORDERED /* keep them ordered */
#define	VECPSTR_PSORTED		VECPSTR_OSORTED	/* keep them sorted */
#define	VECPSTR_PNOHOLES	VECPSTR_OCOMPACT /* swap entries */
#define	VECPSTR_PCONSERVE	VECPSTR_OCONSERVE /* conserve space */


struct vecpstr_flags {
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

struct vecpstr_chunk {
	char		*tab ;
	int		tabsize ;	/* allocated extent */
	int		tablen ;	/* amount used */
	int		count ;		/* number of items */
} ;

struct vecpstr_head {
	uint		magic ;
	const char	**va ;
	VECPSTR_CHUNK	*ccp ;
	VECPSTR_FL	f ;
	VECHAND		chunks ;
	int		chunksize ;
	int		an ;		/* suggested add-number */
	int		c ;		/* total item count */
	int		i ;		/* index */
	int		n ;		/* extent */
	int		fi ;		/* first-index */
	int		stsize ;	/* string table size */
} ;


typedef struct vecpstr_head	vecpstr ;


#if	(! defined(VECPSTR_MASTER)) || (VECPSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int vecpstr_start(VECPSTR *,int,int,int) ;
extern int vecpstr_finish(VECPSTR *) ;
extern int vecpstr_add(VECPSTR *,const char *,int) ;
extern int vecpstr_adduniq(VECPSTR *,const char *,int) ;
extern int vecpstr_addkeyval(VECPSTR *,cchar *,int,cchar *,int) ;
extern int vecpstr_store(VECPSTR *,const char *,int,const char **) ;
extern int vecpstr_already(VECPSTR *,const char *,int) ;
extern int vecpstr_get(VECPSTR *,int,const char **) ;
extern int vecpstr_del(VECPSTR *,int) ;
extern int vecpstr_delall(VECPSTR *) ;
extern int vecpstr_count(VECPSTR *) ;
extern int vecpstr_sort(VECPSTR *,int (*)(const char **,const char **)) ;
extern int vecpstr_search(VECPSTR *,const char *,int (*)(),const char **) ;
extern int vecpstr_find(VECPSTR *,const char *) ;
extern int vecpstr_findn(VECPSTR *,const char *,int) ;
extern int vecpstr_finder(VECPSTR *,const char *,int (*)(),const char **) ;
extern int vecpstr_findaddr(VECPSTR *,const char *) ;
extern int vecpstr_getsize(VECPSTR *) ;
extern int vecpstr_strsize(VECPSTR *) ;
extern int vecpstr_strmk(VECPSTR *,char *,int) ;
extern int vecpstr_recsize(VECPSTR *) ;
extern int vecpstr_recmk(VECPSTR *,int *,int) ;
extern int vecpstr_indlen(VECPSTR *) ;
extern int vecpstr_indsize(VECPSTR *) ;
extern int vecpstr_indmk(VECPSTR *,int (*)[3],int,int) ;
extern int vecpstr_recmkstr(VECPSTR *,int *,int,char *,int) ;
extern int vecpstr_audit(VECPSTR *) ;
extern int vecpstr_getvec(VECPSTR *,const char ***) ;
extern int vecpstr_finish(VECPSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VECPSTR_MASTER */

#endif /* VECPSTR_INCLUDE */


