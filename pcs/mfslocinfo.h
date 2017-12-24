/* mfs-locinfo */

/* last modified %G% version %I% */


/* revision history:

	= 2011-01-25, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2011,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSLOCINFO_INCLUDE
#define	MFSLOCINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<keyopt.h>
#include	<msfile.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<lfm.h>
#include	<envhelp.h>
#include	<varsub.h>

#include	"mfsmain.h"
#include	"mfsns.h"
#include	"defs.h"		/* for PROGINFO */


#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


enum svctypes {
	svctype_mfserve,
	svctype_tcpmux,
	svctype_finger,
	svvtype_overlast
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		cooks:1 ;
	uint		svars:1 ;
	uint		lockinfo:1 ;
	uint		intconf:1 ;
	uint		intsvcs:1 ;
	uint		intwait:1 ;
	uint		intcache:1 ;
	uint		intmaint:1 ;
	uint		intspeed:1 ;
	uint		intdirmaint:1 ;
	uint		intclient:1 ;
	uint		zerospeed:1 ;
	uint		quick:1 ;
	uint		adj:1 ;		/* activate adjunct */
	uint		reuseaddr:1 ;	/* reuse for multiple listeners */
	uint		tmpfname:1 ;	/* TMP lock-file */
	uint		reqfname:1 ;
	uint		mntfname:1 ;
	uint		builtdname:1 ;	/* built-svc directory name */
	uint		svcfname:1 ;	/* tabs-svc file name */
	uint		accfname:1 ;
	uint		msfname:1 ;
	uint		locking:1 ;
	uint		pidlock:1 ;	/* this is LFM on PID-file */
	uint		tmplock:1 ;	/* this is LFM on TMPPID-file */
	uint		sec_root:1 ;
	uint		sec_conf:1 ;
	uint		secure:1 ;
	uint		fg:1 ;
	uint		speedname:1 ;
	uint		envs:1 ;
	uint		subs:1 ;
	uint		ns:1 ;
	uint		cmds:1 ;
	uint		reqexit:1 ;
	uint		listens:1 ;
	uint		runasprn:1 ;
	uint		svctype:1 ;
	uint		users:1 ;	/* usernames are last servives */
	uint		maint:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	cchar		*msnode ;	/* machine-status node */
	cchar		*tmpourdname ;
	cchar		*builtdname ;
	cchar		*speedname ;
	cchar		*tmpfname ;	/* TMP lock-file */
	cchar		*reqfname ;
	cchar		*mntfname ;
	cchar		*svcfname ;
	cchar		*accfname ;
	cchar		*msfname ;
	PROGINFO	*pip ;
	void		*adj ;		/* adjunct local data */
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	LFM		pidlock, tmplock ;
	ENVHELP		envs ;
	VARSUB		subs ;
	EXPCOOK		cooks ;
	VECSTR		svars ;		/* schedule variables */
	MFSNS		ns ;		/* name-server object */
	KEYOPT		cmds ;
	VECOBJ		listens ;
	time_t		ti_lastlock ;
	time_t		ti_start ;
	time_t		ti_marklog ;
	time_t		ti_boot ;
	time_t		ti_lastreq ;
	time_t		ti_tmpmaint ;
	uid_t		uid_rootname ;
	gid_t		gid_rootname ;
	int		svctype ;	/* service (handling) type */
	int		kserial ;
	int		serial ;
	int		nu ;		/* n-updates */
	int		rfd ;		/* request file-descriptor */
	int		nreqs ;		/* count of requests */
	int		intconf ;
	int		intsvcs ;
	int		intwait ;
	int		intcache ;
	int		intmaint ;
	int		intspeed ;
	int		intdirmaint ;	/* interval dir-maintenance */
	int		intclient ;	/* interval client sockets */
	int		to_cache ;
	int		to_lock ;
	int		to_accept ;
	char		cmd[LOGIDLEN + 1] ;	/* for MFS */
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
extern int	locinfo_tmpourdir(LOCINFO *) ;
extern int	locinfo_builtdname(LOCINFO *) ;
extern int	locinfo_msfile(LOCINFO *) ;
extern int	locinfo_reqfname(LOCINFO *) ;
extern int	locinfo_ipcpid(LOCINFO *,int) ;
extern int	locinfo_rootids(LOCINFO *) ;
extern int	locinfo_nsbegin(LOCINFO *) ;
extern int	locinfo_nsend(LOCINFO *) ;
extern int	locinfo_nslook(LOCINFO *,char *,int,cchar *,int) ;
extern int	locinfo_maintourtmp(LOCINFO *) ;
extern int	locinfo_cmdsload(LOCINFO *,cchar *,int) ;
extern int	locinfo_cmdscount(LOCINFO *) ;
extern int	locinfo_reqexit(LOCINFO *,cchar *) ;
extern int	locinfo_isreqexit(LOCINFO *) ;
extern int	locinfo_getaccto(LOCINFO *) ;
extern int	locinfo_getreqs(LOCINFO *) ;
extern int	locinfo_newserial(LOCINFO *) ;
extern int	locinfo_varbegin(LOCINFO *) ;
extern int	locinfo_varend(LOCINFO *) ;
extern int	locinfo_varsub(LOCINFO *,char *,int,cchar *,int) ;
extern int	locinfo_daemonbegin(LOCINFO *) ;
extern int	locinfo_daemonend(LOCINFO *) ;
extern int	locinfo_cooksvc(LOCINFO *,cchar *,cchar *,cchar **,int) ;
extern int	locinfo_svctype(LOCINFO *,cchar *,int) ;
extern int	locinfo_newreq(LOCINFO *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSLOCINFO_INCLUDE */


