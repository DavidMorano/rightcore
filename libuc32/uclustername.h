/* uclustername */

/* UNIX® cluster name */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UCLUSTERNAME_INCLUDE
#define	UCLUSTERNAME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>


#if	(! defined(UCLUSTERNAME_MASTER)) || (UCLUSTERNAME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uclustername_init() ;
extern int uclustername_set(const char *,int,const char *,int) ;
extern int uclustername_get(char *,int,const char *) ;
extern void uclustername_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UCLUSTERNAME_MASTER */

#endif /* UCLUSTERNAME_INCLUDE */


