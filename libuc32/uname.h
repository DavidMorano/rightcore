/* uname */

/* UNIX® information (a cache for 'uname(2)' and sisters) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UNAME_INCLUDE
#define	UNAME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>


#define	UNAME		struct uname_head


struct uname_head {
	const char	*sysname ;
	const char	*nodename ;
	const char	*release ;
	const char	*version ;
	const char	*machine ;
	const char	*a ;		/* the memory-allocation */
} ;


#if	(! defined(UNAME_MASTER)) || (UNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uname_start(UNAME *) ;
extern int uname_finish(UNAME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UNAME_MASTER */

#endif /* UNAME_INCLUDE */


