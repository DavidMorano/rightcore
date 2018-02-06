/* msu-log */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSULOG_INCLUDE
#define	MSULOG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<msfile.h>
#include	<lfm.h>

#include	"msumain.h"
#include	"defs.h"		/* for PROGINFO */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	logbegin(PROGINFO *) ;
extern int	logend(PROGINFO *) ;
extern int	logflush(PROGINFO *) ;
extern int	logcheck(PROGINFO *) ;
extern int	logprintf(PROGINFO *,const char *,...) ;
extern int	logprogname(PROGINFO *) ;
extern int	logmark(PROGINFO *,int) ;
extern int	logreport(PROGINFO *) ;
extern int	loginvalidcmd(PROGINFO *,const char *) ;
extern int	loginfo(PROGINFO *) ;
extern int	loglock(PROGINFO *,LFM_CHECK *,cchar *,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSULOG_INCLUDE */


