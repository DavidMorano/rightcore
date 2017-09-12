/* progreport */

/* program-reporting */


/* revision history:

	= 2017-09-11, David A­D­ Morano
        Taken out of existing code to try to make more useful for a wider range
        of programs.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We report some simple data when a program (an important program) fails.


*******************************************************************************/


#ifndef	PROGREPORT_INCLUDE
#define	PROGREPORT_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


#ifndef	ADMINUSER
#define	ADMINUSER	"admin"
#endif
#ifndef	DISARGLEN
#define	DISARGLEN	50
#endif
#ifndef	REPORTDNAME
#define	REPORTDNAME	"/var/tmp/reports"
#endif


#if	(! defined(PROGREPORT_MASTER)) || (PROGREPORT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mkreport(PROGINFO *,int,cchar **,int) ;

 
#ifdef	__cplusplus
}
#endif

#endif /* PROGREPORT_MASTER */

#endif /* PROGREPORT_INCLUDE */


