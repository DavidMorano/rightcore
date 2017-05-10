/* proglogzone */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PROGLOGZONE_INCLUDE
#define	PROGLOGZONE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>
#include	"defs.h"


#if	(! defined(PROGLOGZONE_MASTER)) || (PROGLOGZONE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int proglogzone_begin(PROGINFO *) ;
extern int proglogzone_end(PROGINFO *) ;
extern int proglogzone_update(PROGINFO *,cchar *,int,int,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGLOGZONE_MASTER */

#endif /* PROGLOGZONE_INCLUDE */


