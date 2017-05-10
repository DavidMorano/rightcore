/* hdbstr */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HDBSTR_INCLUDE
#define	HDBSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	HDBSTR		HDB
#define	HDBSTR_CUR	HDB_CUR


typedef	HDB		hdbstr ;
typedef	HDB_CUR		hdbstr_cur ;


#if	(! defined(HDBSTR_MASTER)) || (HDBSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hdbstr_start(HDBSTR *,int) ;
extern int hdbstr_add(HDBSTR *,cchar *,int,cchar *,int) ;
extern int hdbstr_curbegin(HDBSTR *,HDBSTR_CUR *) ;
extern int hdbstr_curend(HDBSTR *,HDBSTR_CUR *) ;
extern int hdbstr_fetch(HDBSTR *,cchar *,int,HDBSTR_CUR *,cchar **) ;
extern int hdbstr_fetchrec(HDBSTR *,cchar *,int,HDBSTR_CUR *,
				cchar **,cchar **,int *) ;
extern int hdbstr_getrec(HDBSTR *,HDBSTR_CUR *,cchar **,cchar **,int *) ;
extern int hdbstr_enum(HDBSTR *,HDBSTR_CUR *,cchar **,cchar **,int *) ;
extern int hdbstr_delkey(HDBSTR *,cchar *,int) ;
extern int hdbstr_delcur(HDBSTR *,HDBSTR_CUR *,int) ;
extern int hdbstr_next(HDBSTR *,HDBSTR_CUR *) ;
extern int hdbstr_nextkey(HDBSTR *,cchar *,int,HDBSTR_CUR *) ;
extern int hdbstr_count(HDBSTR *) ;
extern int hdbstr_finish(HDBSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HDBSTR_MASTER */

#endif /* HDBSTR_INCLUDE */


