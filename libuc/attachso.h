/* attachso */

/* ATTACHSO function */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ATTACHSO_INCLUDE
#define	ATTACHSO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* extra types */


#if	(! defined(ATTACHSO_MASTER)) || (ATTACHSO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int attachso(const char **,const char *,const char **,const char **,
		int,void **) ;

#ifdef	__cplusplus
}
#endif

#endif /* ATTACHSO_MASTER */

#endif /* ATTACHSO_INCLUDE */


