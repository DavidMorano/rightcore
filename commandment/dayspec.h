/* dayspec */


/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	DAYSPEC_INCLUDE
#define	DAYSPEC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>		/* for the signed special types */


#define	DAYSPEC		struct dayspec


struct dayspec {
	short		y ;
	schar		m, d ;
} ;


#if	(! defined(DAYSPEC_MASTER)) || (DAYSPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dayspec_default(DAYSPEC *) ;
extern int dayspec_load(DAYSPEC *,cchar *,int) ;
extern int dayspec_yday(DAYSPEC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DAYSPEC_MASTER */

#endif /* DAYSPEC_INCLUDE */


