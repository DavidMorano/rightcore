/* termtype */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-12-16, David A­D­ Morano
	Updated.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */


#ifndef	TERMTYPE_INCLUDE
#define	TERMTYPE_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>	/* for special types */


#define	TERMTYPE	struct termtype


struct termtype {
	cchar		*name ;
	short		pv[4] ;
	short		sv[4] ;
} ;


#if	(! defined(TERMTYPE_MASTER)) || (TERMTYPE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termtype(const TERMTYPE *,const short *,const short *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMTYPE_MASTER */

#endif /* TERMTYPE_INCLUDE */


