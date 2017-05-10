/* umask */

/* UNIX® information (a cache for 'uname(2)' and sisters) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UINFO_INCLUDE
#define	UINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#if	(! defined(UINFO_MASTER)) || (UINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int umask_init() ;
extern int umaskset(mode_t) ;
extern int umaskget() ;
extern int getumask() ;
extern int setumask(mode_t) ;
extern void umask_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UINFO_MASTER */

#endif /* UINFO_INCLUDE */


