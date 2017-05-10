/* progids */

/* program-IDS */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage IDS object initiaization.


*******************************************************************************/


#ifndef	PROGIDS_INCLUDE
#define	PROGIDS_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<localmisc.h>

#include	"defs.h"


#if	(! defined(PROGIDS_MASTER)) || (PROGIDS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int progids_begin(PROGINFO *) ;
extern int progids_end(PROGINFO *) ;
extern int progids_sperm(PROGINFO *,struct ustat *,int) ;
 
#ifdef	__cplusplus
}
#endif

#endif /* PROGIDS_MASTER */

#endif /* PROGIDS_INCLUDE */


