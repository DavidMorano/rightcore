/* proglogenv */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PROGLOGENV_INCLUDE
#define	PROGLOGENV_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>
#include	"defs.h"


#if	(! defined(PROGLOGENV_MASTER)) || (PROGLOGENV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int proglogenv_begin(PROGINFO *) ;
extern int proglogenv_end(PROGINFO *) ;
extern int proglogenv_print(PROGINFO *,cchar *,int) ;
extern int proglogenv_vprintf(PROGINFO *,cchar *,va_list) ;
extern int proglogenv_printf(PROGINFO *,const char *,...) ;
extern int proglogenv_flush(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGLOGENV_MASTER */

#endif /* PROGLOGENV_INCLUDE */


