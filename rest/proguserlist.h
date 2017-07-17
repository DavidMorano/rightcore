/* proguserlist */

/* program-user-list-ing */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage a program user-list operation.


*******************************************************************************/


#ifndef	PROGUSERLIST_INCLUDE
#define	PROGUSERLIST_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"defs.h"


#if	(! defined(PROGUSERLIST_MASTER)) || (PROGUSERLIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;
 
#ifdef	__cplusplus
}
#endif

#endif /* PROGUSERLIST_MASTER */

#endif /* PROGUSERLIST_INCLUDE */


