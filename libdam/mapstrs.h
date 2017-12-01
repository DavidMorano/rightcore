/* mapstrs */

/* environment container */
/* last modified %G% version %I% */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAPSTRS_INCLUDE
#define	MAPSTRS_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<strpack.h>
#include	<localmisc.h>


/* object defines */

#define	MAPSTRS			struct mapstrs_head
#define	MAPSTRS_CHUNKSIZE	200


struct mapstrs_head {
	HDB		list ;
	STRPACK		store ;
} ;


typedef MAPSTRS		mapstrs ;


#if	(! defined(MAPSTRS_MASTER)) || (MAPSTRS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mapstrs_start(MAPSTRS *,int) ;
extern int mapstrs_add(MAPSTRS *,cchar *,int,cchar *,int) ;
extern int mapstrs_delkey(MAPSTRS *,cchar *,int) ;
extern int mapstrs_present(MAPSTRS *,cchar *,int,cchar **) ;
extern int mapstrs_count(MAPSTRS *) ;
extern int mapstrs_finish(MAPSTRS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(MAPSTRS_MASTER)) || (MAPSTRS_MASTER == 0) */

#endif /* MAPSTRS_INCLUDE */


