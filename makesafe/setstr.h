/* setstr */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SETSTR_INCLUDE
#define	SETSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>		/* for 'uint' */


#define	SETSTR		HDB
#define	SETSTR_CUR	HDB_CUR


typedef SETSTR		setstr ;
typedef SETSTR_CUR	setstr_cur ;


#if	(! defined(SETSTR_MASTER)) || (SETSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int setstr_start(setstr *,int) ;
extern int setstr_already(setstr *,cchar *,int) ;
extern int setstr_add(setstr *,cchar *,int) ;
extern int setstr_del(setstr *,cchar *,int) ;
extern int setstr_count(setstr *) ;
extern int setstr_curbegin(setstr *,setstr_cur *) ;
extern int setstr_enum(setstr *,setstr_cur *,cchar **) ;
extern int setstr_curend(setstr *,setstr_cur *) ;
extern int setstr_finish(setstr *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SETSTR_MASTER */

#endif /* SETSTR_INCLUDE */


