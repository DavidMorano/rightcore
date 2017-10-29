/* bbspec */


/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	BBSPEC_INCLUDE
#define	BBSPEC_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>		/* for the signed special types */


#define	BBSPEC		struct bbspec


struct bbspec {
	const char	*np ;
	int		nl ;
	schar		b, c, v ;
} ;


#if	(! defined(BBSPEC_MASTER)) || (BBSPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int bbspec_load(BBSPEC *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BBSPEC_MASTER */

#endif /* BBSPEC_INCLUDE */


