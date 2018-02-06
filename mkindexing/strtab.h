/* strtab */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRTAB_INCLUDE
#define	STRTAB_INCLUDE	1


#define	STRTAB_ALLOCOBJ	0		/* use ALLOCOBJ object? */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<hdb.h>

#if	STRTAB_ALLOCOBJ
#include	<allocobj.h>
#else
#include	<lookaside.h>
#endif /* STRTAB_ALLOCOBJ */

#include	<localmisc.h>


#define	STRTAB			struct strtab_head
#define	STRTAB_CHUNK		struct strtab_chunk

#if	STRTAB_ALLOCOBJ
#define	STRTAB_AOBJ		ALLOCOBJ
#else
#define	STRTAB_AOBJ		LOOKASIDE
#endif /* STRTAB_ALLOCOBJ */

#define	STRTAB_MAGIC		0x88776215
#define	STRTAB_MINCHUNKSIZE	40


struct strtab_chunk {
	char		*cdata ;
	int		csize ;		/* allocated extent */
	int		cl ;		/* amount used */
	int		count ;		/* number of items */
} ;

struct strtab_head {
	uint		magic ;
	STRTAB_CHUNK	*ccp ;
	VECHAND		chunks ;
	HDB		strdb ;
	STRTAB_AOBJ	intdb ;
	int		chunksize ;
	int		stsize ;	/* "string table" size */
	int		count ;		/* total item count */
} ;


#if	(! defined(STRTAB_MASTER)) || (STRTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strtab_start(STRTAB *,int) ;
extern int	strtab_finish(STRTAB *) ;
extern int	strtab_add(STRTAB *,cchar *,int) ;
extern int	strtab_addfast(STRTAB *,cchar *,int) ;
extern int	strtab_already(STRTAB *,cchar *,int) ;
extern int	strtab_count(STRTAB *) ;
extern int	strtab_strsize(STRTAB *) ;
extern int	strtab_strmk(STRTAB *,char *,int) ;
extern int	strtab_recsize(STRTAB *) ;
extern int	strtab_recmk(STRTAB *,int *,int) ;
extern int	strtab_indlen(STRTAB *) ;
extern int	strtab_indsize(STRTAB *) ;
extern int	strtab_indmk(STRTAB *,int (*)[3],int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRTAB_MASTER */

#endif /* STRTAB_INCLUDE */


