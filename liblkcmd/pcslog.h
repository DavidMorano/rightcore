/* pcs-log */


/* revision history:

	= 2011-01-25, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSLOG_INCLUDE
#define	PCSLOG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<msfile.h>
#include	<userinfo.h>
#include	<lfm.h>

#include	"pcsmain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	logbegin(PROGINFO *,USERINFO *) ;
extern int	logend(PROGINFO *) ;
extern int	logflush(PROGINFO *) ;
extern int	logcheck(PROGINFO *) ;
extern int	logprint(PROGINFO *,cchar *,int) ;
extern int	logprintf(PROGINFO *,cchar *,...) ;
extern int	logprogname(PROGINFO *) ;
extern int	logmark(PROGINFO *,int) ;
extern int	logreport(PROGINFO *) ;
extern int	loginvalidcmd(PROGINFO *,cchar *) ;
extern int	loginfo(PROGINFO *) ;
extern int	loglock(PROGINFO *,LFM_CHECK *,cchar *,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSLOG_INCLUDE */


