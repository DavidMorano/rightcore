/* uinfo */

/* UNIX® information (a cache for 'uname(2)' and sisters) */


/* revision history:

	= 2000-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	UINFO_INCLUDE
#define	UINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#define	UINFO_NAME	struct uinfo_name
#define	UINFO_AUX	struct uinfo_aux


struct uinfo_name {
	const char	*sysname ;
	const char	*nodename ;
	const char	*release ;
	const char	*version ;
	const char	*machine ;
} ;

struct uinfo_aux {
	const char	*architecture ;
	const char	*platform ;
	const char	*provider ;
	const char	*hwserial ;
	const char	*nisdomain ;
} ;


#if	(! defined(UINFO_MASTER)) || (UINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uinfo_init() ;
extern int uinfo_name(struct uinfo_name *) ;
extern int uinfo_aux(struct uinfo_aux *) ;
extern void uinfo_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UINFO_MASTER */

#endif /* UINFO_INCLUDE */


