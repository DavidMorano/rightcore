/* sysmemutil */


/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMEMUTIL_INCLUDE
#define	SYSMEMUTIL_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>		/* for the signed special types */


#define	SYSMEMUTIL		struct sysmemutil


struct sysmemutil {
	long		mt ;		/* pages "total" */
	long		ma ;		/* pages "available" */
	int		mu ;		/* as a percentage */
} ;


#if	(! defined(SYSMEMUTIL_MASTER)) || (SYSMEMUTIL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysmemutil(SYSMEMUTIL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMEMUTIL_MASTER */

#endif /* SYSMEMUTIL_INCLUDE */


