/* keyvals */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KEYVALS_INCLUDE
#define	KEYVALS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	KEYVALS			struct keyvals_head
#define	KEYVALS_CUR		struct keyvals_c
#define	KEYVALS_MAGIC		0x31415993
#define	KEYVALS_DEFENTS	100


struct keyvals_c {
	HDB_CUR		ec ;
	int		i ;
} ;

struct keyvals_head {
	uint		magic ;
	VECOBJ		keys ;
	HDB		bykey ;		/* indexed by key only */
	HDB		bykeyval ;	/* indexed by key-val together */
} ;


typedef struct keyvals_head	keyvals ;
typedef struct keyvals_c	keyvals_cur ;


#if	(! defined(KEYVALS_MASTER)) || (KEYVALS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int keyvals_start(KEYVALS *,int) ;
extern int keyvals_add(KEYVALS *,int,const char *,const char *,int) ;
extern int keyvals_count(KEYVALS *) ;
extern int keyvals_curbegin(KEYVALS *,KEYVALS_CUR *) ;
extern int keyvals_curend(KEYVALS *,KEYVALS_CUR *) ;
extern int keyvals_enumkey(KEYVALS *,KEYVALS_CUR *,const char **) ;
extern int keyvals_enum(KEYVALS *,KEYVALS_CUR *,const char **,const char **) ;
extern int keyvals_fetch(KEYVALS *,const char *,KEYVALS_CUR *,const char **) ;
extern int keyvals_delset(KEYVALS *,int) ;
extern int keyvals_delkey(KEYVALS *,const char *,int) ;
extern int keyvals_check(KEYVALS *,time_t) ;
extern int keyvals_finish(KEYVALS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KEYVALS_MASTER */

#endif /* KEYVALS_INCLUDE */


