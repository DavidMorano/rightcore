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


struct querystring_head {
	cchar		*(*kv)[2] ;
	strpack		p ;
	int		n ;
} ;



#if	(! defined(QUERYSTRING_MASTER)) || (QUERYSTRING_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int querystring_start(QUERYSTRING *,cchar *,int) ;
extern int querystring_getkey(QUERYSTRING *,int,cchar **,cchar **) ;
extern int querystring_getval(QUERYSTRING *,int,cchar **,cchar **) ;
extern int querystring_finish(QUERYSTRING *) ;

#ifdef	__cplusplus
}
#endif

#endif /* QUERYSTRING_MASTER */

#endif /* QUERYSTRING_INCLUDE */


