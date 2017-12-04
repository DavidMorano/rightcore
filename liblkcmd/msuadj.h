/* msu-adjunct */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSUADJUNCT_INCLUDE
#define	MSUADJUNCT_INCLUDE	1


#include	<envstandards.h>

#include	<msfile.h>

#include	"msumain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	procipcbegin(PROGINFO *) ;
extern int	procipcend(PROGINFO *) ;
extern int	procipcreq(PROGINFO *,int,MSFILE_ENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSUADJUNCT_INCLUDE */


