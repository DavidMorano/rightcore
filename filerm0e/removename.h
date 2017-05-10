/* removename */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	REMOVENAME_INCLUDE
#define	REMOVENAME_INCLUDE	1


#include	<randomvar.h>


/* object defines (options) */
#define	REMOVENAME_MDEFAULT	0
#define	REMOVENAME_MBURN	1
#define	REMOVENAME_MFOLLOW	2


#ifdef	__cplusplus
extern "C" {
#endif

extern int	removename(randomvar *,int,int,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* REMOVENAME_INCLUDE */


