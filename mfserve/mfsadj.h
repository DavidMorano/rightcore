/* mfs-adj(unct) */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

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
extern int	mfsadj_req(PROGINFO *,int,int) ;
extern int	mfsadj_register(PROGINFO *,POLLER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSADJ_INCLUDE */


