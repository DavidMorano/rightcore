/* bcspec */


/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	BCSPEC_INCLUDE
#define	BCSPEC_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>		/* for the signed special types */


#define	BCSPEC		struct bcspec


struct bcspec {
	const char	*np ;
	int		nl ;
	schar		b, c, v ;
} ;


#if	(! defined(BCSPEC_MASTER)) || (BCSPEC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int bcspec_load(BCSPEC *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BCSPEC_MASTER */

#endif /* BCSPEC_INCLUDE */


