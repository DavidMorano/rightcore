/* mapstr */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAPSTR_INCLUDE
#define	MAPSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	MAPSTR		HDB
#define	MAPSTR_CUR	HDB_CUR


typedef	HDB		mapstr ;
typedef	HDB_CUR		mapstr_cur ;


#if	(! defined(MAPSTR_MASTER)) || (MAPSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mapstr_start(MAPSTR *,int) ;
extern int mapstr_add(MAPSTR *,cchar *,int,cchar *,int) ;
extern int mapstr_get(MAPSTR *,cchar *,int,cchar **) ;
extern int mapstr_curbegin(MAPSTR *,MAPSTR_CUR *) ;
extern int mapstr_enum(MAPSTR *,MAPSTR_CUR *,cchar **,cchar **,int *) ;
extern int mapstr_curend(MAPSTR *,MAPSTR_CUR *) ;
extern int mapstr_count(MAPSTR *) ;
extern int mapstr_finish(MAPSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAPSTR_MASTER */

#endif /* MAPSTR_INCLUDE */


