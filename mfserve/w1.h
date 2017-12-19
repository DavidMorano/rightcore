/* mfs-watch */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSWATCH_INCLUDE
#define	MFSWATCH_INCLUDE	1

#include	<envstandards.h>

#include	"mfsmain.h"
#include	"defs.h"

enum jobtypes {
	jobtype_req,
	jobtype_listen,
	jobtype_overlast
} ;

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfswatch_begin(PROGINFO *) ;
extern int	mfswatch_service(PROGINFO *) ;
extern int	mfswatch_newjob(PROGINFO *,int,int,int,int) ;
extern int	mfswatch_end(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSWATCH_INCLUDE */


