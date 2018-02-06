/* progeigen */

/* program eigen-database */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage the EIGENDB object initiaization.


*******************************************************************************/


#ifndef	PROGEIGEN_INCLUDE
#define	PROGEIGEN_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"defs.h"


#define	PROGEIGEN_CUR	struct progeigen_cur


struct progeigen_cur {
	EIGENDB_CUR	cur ;
} ;


#if	(! defined(PROGEIGEN_MASTER)) || (PROGEIGEN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int progeigen_begin(PROGINFO *) ;
extern int progeigen_end(PROGINFO *) ;
extern int progeigen_count(PROGINFO *) ;
extern int progeigen_curbegin(PROGINFO *,PROGEIGEN_CUR *) ;
extern int progeigen_enum(PROGINFO *,PROGEIGEN_CUR *,cchar **) ;
extern int progeigen_curend(PROGINFO *,PROGEIGEN_CUR *) ;
 
#ifdef	__cplusplus
}
#endif

#endif /* PROGEIGEN_MASTER */

#endif /* PROGEIGEN_INCLUDE */


