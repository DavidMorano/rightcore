/* querystring */
/* lang=C99 */


/* revision history:

	= 2017-09-25, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	QUERYSTRING_INCLUDE
#define	QUERYSTRING_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<strpack.h>
#include	<localmisc.h>		/* extra types */


#define	QUERYSTRING	struct querystring_head
#define	QUERYSTRING_FL	struct querystring_flags
#define	QUERYSTRING_CUR	struct querystring_cur


struct querystring_cur {
	int		i ;
} ;

struct querystring_flags {
	uint		packer:1 ;
} ;

struct querystring_head {
	QUERYSTRING_FL	open ;
	strpack		packer ;
	cchar		*(*kv)[2] ;
	int		n ;
} ;


#if	(! defined(QUERYSTRING_MASTER)) || (QUERYSTRING_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int querystring_start(QUERYSTRING *,cchar *,int) ;
extern int querystring_count(QUERYSTRING *) ;
extern int querystring_already(QUERYSTRING *,cchar *,int) ;
extern int querystring_curbegin(QUERYSTRING *,QUERYSTRING_CUR *) ;
extern int querystring_curend(QUERYSTRING *,QUERYSTRING_CUR *) ;
extern int querystring_fetch(QUERYSTRING *,cchar *,int,QUERYSTRING_CUR *,
		cchar **) ;
extern int querystring_enum(QUERYSTRING *,QUERYSTRING_CUR *,cchar **,cchar **) ;
extern int querystring_get(QUERYSTRING *,int,cchar **,cchar **) ;
extern int querystring_finish(QUERYSTRING *) ;

#ifdef	__cplusplus
}
#endif

#endif /* QUERYSTRING_MASTER */

#endif /* QUERYSTRING_INCLUDE */


