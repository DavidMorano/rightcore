/* msu-locinfo */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSULOCINFO_INCLUDE
#define	MSULOCINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<msfile.h>
#include	<lfm.h>

#include	"msumain.h"
#include	"defs.h"		/* for PROGINFO */


#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


struct locinfo_flags {
	uint		stores:1 ;
	uint		lockinfo:1 ;
	uint		intspeed:1 ;
	uint		intconfig:1 ;
	uint		zerospeed:1 ;
	uint		quick:1 ;
	uint		listen:1 ;	/* listen on IPC */
	uint		reuseaddr:1 ;	/* reuse for multiple listeners */
	uint		msfname:1 ;
	uint		tmpfname:1 ;	/* TMP lock-file */
	uint		reqfname:1 ;
	uint		mntfname:1 ;
	uint		pidlock:1 ;	/* this is LFM on PID-file */
	uint		tmplock:1 ;	/* this is LFM on TMPPID-file */
	uint		sec_root:1 ;
	uint		sec_conf:1 ;
	uint		secure:1 ;
	uint		fg:1 ;
	uint		speedname:1 ;
	uint		reqexit:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	cchar		*msnode ;
	cchar		*tmpourdname ;
	cchar		*msfname ;
	cchar		*tmpfname ;	/* TMP lock-file */
	cchar		*reqfname ;
	cchar		*mntfname ;
	cchar		*speedname ;
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	LFM		pidlock, tmplock ;
	time_t		ti_lastlock ;
	time_t		ti_start ;
	time_t		ti_marklog ;
	time_t		ti_boot ;
	uid_t		uid_rootname ;
	gid_t		gid_rootname ;
	int		intconfig ;	/* interval configuration changed */
	int		intspeed ;	/* interval speed update */
	int		nu ;		/* n-updates */
	int		rfd ;		/* request file-descriptor */
	int		to_cache ;
	int		to_lock ;
	char		cmd[LOGIDLEN + 1] ;	/* for MSU */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	locinfo_start(LOCINFO *,PROGINFO *) ;
extern int	locinfo_finish(LOCINFO *) ;
extern int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
extern int	locinfo_rootname(LOCINFO *) ;
extern int	locinfo_defs(LOCINFO *) ;
extern int	locinfo_lockbegin(LOCINFO *) ;
extern int	locinfo_lockcheck(LOCINFO *) ;
extern int	locinfo_lockend(LOCINFO *) ;
extern int	locinfo_defreg(LOCINFO *) ;
extern int	locinfo_defdaemon(LOCINFO *) ;
extern int	locinfo_tmpourdname(LOCINFO *) ;
extern int	locinfo_msfile(LOCINFO *) ;
extern int	locinfo_reqfname(LOCINFO *) ;
extern int	locinfo_ipcpid(LOCINFO *,int) ;
extern int	locinfo_gidrootname(LOCINFO *) ;
extern int	locinfo_reqexit(LOCINFO *,cchar *) ;
extern int	locinfo_isreqexit(LOCINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSULOCINFO_INCLUDE */


