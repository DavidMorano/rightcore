/* bbhosts */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	BBHOSTS_INCLUDE
#define	BBHOSTS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>


#define	BBHOSTS		VECSTR


#if	(! defined(BBHOSTS_MASTER)) || (BBHOSTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bbhosts_start(BBHOSTS *,const char *,const char *) ;
extern int	bbhosts_finish(BBHOSTS *) ;
extern int	bbhosts_get(BBHOSTS *,int,const char **) ;
extern int	bbhosts_find(BBHOSTS *,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BBHOSTS_MASTER */

#endif /* BBHOSTS_INCLUDE */


