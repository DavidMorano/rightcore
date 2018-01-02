/* biblemeta */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLEMETA_INCLUDE
#define	BIBLEMETA_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<localmisc.h>


#define	BIBLEMETA_MAGIC		0x99447244
#define	BIBLEMETA		struct biblemeta_head
#define	BIBLEMETA_OBJ		struct biblemeta_obj


/* these are the entry definitions right here! */

enum biblemetas {
 	biblemeta_chapter,
 	biblemeta_psalm,
 	biblemeta_bookindex,
 	biblemeta_page,
 	biblemeta_booktitle,
 	biblemeta_thebookof,
 	biblemeta_book,
	biblemeta_overlast
} ;

/* this is the shared-object descritoption */

struct biblemeta_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct biblemeta_head {
	ulong		magic ;
	vecstr		db ;
} ;


#if	(! defined(BIBLEMETA_MASTER)) || (BIBLEMETA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	biblemeta_open(BIBLEMETA *,const char *,const char *) ;
extern int	biblemeta_get(BIBLEMETA *,int,char *,int) ;
extern int	biblemeta_audit(BIBLEMETA *) ;
extern int	biblemeta_close(BIBLEMETA *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEMETA_MASTER */

#endif /* BIBLEMETA_INCLUDE */


