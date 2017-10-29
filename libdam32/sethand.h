/* sethand */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SETHAND_INCLUDE
#define	SETHAND_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>		/* for 'uint' */


#define	SETHAND		HDB
#define	SETHAND_CUR	HDB_CUR


typesef uint (* sethandhash_t)(const void *) ;
typesef int (* sethandcmp_t)(const void *,const void *) ;

typedef SETHAND		sethand ;
typedef SETHAND_CUR	sethand_cur ;


#if	(! defined(SETHAND_MASTER)) || (SETHAND_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sethand_start(sethand *,int) ;
extern int sethand_already(sethand *,cchar *,int) ;
extern int sethand_add(sethand *,cchar *,int) ;
extern int sethand_del(sethand *,cchar *,int) ;
extern int sethand_count(sethand *) ;
extern int sethand_curbegin(sethand *,sethand_cur *) ;
extern int sethand_enum(sethand *,sethand_cur *,cchar **) ;
extern int sethand_curend(sethand *,sethand_cur *) ;
extern int sethand_finish(sethand *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SETHAND_MASTER */

#endif /* SETHAND_INCLUDE */


