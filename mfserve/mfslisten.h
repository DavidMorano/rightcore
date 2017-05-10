/* mfs-listen */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSLISTEN_INCLUDE
#define	MFSLISTEN_INCLUDE	1

#include	<envstandards.h>

#include	<vecobj.h>
#include	<listenspec.h>
#include	<poller.h>

#include	"defs.h"
#include	"mfsmain.h"


#define	MFSLISTEN_ACQ		struct mfslisten_acq


struct mfslisten_acq {
	vecobj		tmps ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfslisten_begin(PROGINFO *) ;
extern int	mfslisten_acqbegin(PROGINFO *,MFSLISTEN_ACQ *) ;
extern int	mfslisten_acqadd(PROGINFO *,MFSLISTEN_ACQ *,cchar *,int) ;
extern int	mfslisten_acqend(PROGINFO *,MFSLISTEN_ACQ *) ;
extern int	mfslisten_maint(PROGINFO *,POLLER *) ;
extern int	mfslisten_handle(PROGINFO *,POLLER *,int,int) ;
extern int	mfslisten_end(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSLISTEN_INCLUDE */


