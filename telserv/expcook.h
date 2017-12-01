/* expcook */

/* last modified %G% version %I% */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	EXPCOOK_INCLUDE
#define	EXPCOOK_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<hdbstr.h>
#include	<buffer.h>
#include	<localmisc.h>


#define	EXPCOOK		struct expcook_head
#define	EXPCOOK_CUR	struct expcook_c
#define	EXPCOOK_MAGIC	0x75837393


struct expcook_c {
	HDBSTR_CUR	cur ;
} ;

struct expcook_head {
	uint		magic ;
	HDBSTR		subs ;
} ;


#if	(! defined(EXPCOOK_MASTER)) || (EXPCOOK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int expcook_start(EXPCOOK *) ;
extern int expcook_add(EXPCOOK *,cchar *,cchar *,int) ;
extern int expcook_curbegin(EXPCOOK *,EXPCOOK_CUR *) ;
extern int expcook_curend(EXPCOOK *,EXPCOOK_CUR *) ;
extern int expcook_enum(EXPCOOK *,EXPCOOK_CUR *,char *,int) ;
extern int expcook_findkey(EXPCOOK *,cchar *,int,cchar **) ;
extern int expcook_delkey(EXPCOOK *,cchar *) ;
extern int expcook_exp(EXPCOOK *,int,char *,int,cchar *,int) ;
extern int expcook_expbuf(EXPCOOK *,int,BUFFER *,cchar *,int) ;
extern int expcook_finish(EXPCOOK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* EXPCOOK_MASTER */

#endif /* EXPCOOK_INCLUDE */


