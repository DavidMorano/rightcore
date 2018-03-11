/* mapstrint */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAPSTRINT_INCLUDE
#define	MAPSTRINT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<hdb.h>


#define	MAPSTRINT	HDB
#define	MAPSTRINT_CUR	HDB_CUR


typedef	HDB		mapstrint ;
typedef	HDB_CUR		mapstrint_cur ;


#if	(! defined(MAPSTRINT_MASTER)) || (MAPSTRINT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mapstrint_start(MAPSTRINT *,int) ;
extern int mapstrint_count(MAPSTRINT *) ;
extern int mapstrint_add(MAPSTRINT *,cchar *,int,int) ;
extern int mapstrint_already(MAPSTRINT *,cchar *,int) ;
extern int mapstrint_fetch(MAPSTRINT *,cchar *,int,
		MAPSTRINT_CUR *,int *) ;
extern int mapstrint_fetchrec(MAPSTRINT *,cchar *,int,
		MAPSTRINT_CUR *, char **,int *) ;
extern int mapstrint_getrec(MAPSTRINT *,MAPSTRINT_CUR *,
		cchar **,int *) ;
extern int mapstrint_enum(MAPSTRINT *,MAPSTRINT_CUR *,
		cchar **,int *) ;
extern int mapstrint_delkey(MAPSTRINT *,cchar *,int) ;
extern int mapstrint_delcur(MAPSTRINT *,MAPSTRINT_CUR *,int) ;
extern int mapstrint_next(MAPSTRINT *,MAPSTRINT_CUR *) ;
extern int mapstrint_nextkey(MAPSTRINT *,cchar *,int,MAPSTRINT_CUR *) ;
extern int mapstrint_curbegin(MAPSTRINT *,MAPSTRINT_CUR *) ;
extern int mapstrint_curend(MAPSTRINT *,MAPSTRINT_CUR *) ;
extern int mapstrint_setval(MAPSTRINT *,MAPSTRINT_CUR *,int) ;
extern int mapstrint_finish(MAPSTRINT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAPSTRINT_MASTER */

#endif /* MAPSTRINT_INCLUDE */


