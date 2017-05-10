/* mfs-watch */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSWATCH_INCLUDE
#define	MFSWATCH_INCLUDE	1

#include	<envstandards.h>

#include	"mfsmain.h"
#include	"defs.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfswatch_begin(PROGINFO *) ;
extern int	mfswatch_service(PROGINFO *) ;
extern int	mfswatch_newjob(PROGINFO *,int,int) ;
extern int	mfswatch_end(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSWATCH_INCLUDE */


