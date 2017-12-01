/* mfs-adj(unct) */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2005,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSADJ_INCLUDE
#define	MFSADJ_INCLUDE	1


#include	<envstandards.h>
#include	<poller.h>
#include	"defs.h"


#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfsadj_begin(PROGINFO *) ;
extern int	mfsadj_end(PROGINFO *) ;
extern int	mfsadj_poll(PROGINFO *,POLLER *,int,int) ;
extern int	mfsadj_register(PROGINFO *,POLLER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSADJ_INCLUDE */


