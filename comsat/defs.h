/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<ptm.h>
#include	<termnote.h>
#include	<dater.h>
#include	<localmisc.h>


#ifndef	nelem
#ifdef	nelements
#define	nelem		nelements
#else
#define	nelem(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif

#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold 'int128_t' in decimal */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags
#define	PROGINFO_LOG	struct proginfo_log

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_log {
	PTM		lm ;		/* LOGFILE mutex */
	time_t		ti_logsize ;
	time_t		ti_logcheck ;
	time_t		ti_logflush ;
	int		intlogsize ;
	int		intlogcheck ;
	int		intlogflush ;
} ;

struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		logprog:1 ;
	uint		background:1 ;
	uint		daemon:1 ;
	uint		intrun:1 ;
	uint		intidle:1 ;
	uint		listen:1 ;
	uint		reuseaddr:1 ;
	uint		issocket:1 ;
	uint		isstream:1 ;
	uint		termnote:1 ;
	uint		logsize:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		lm:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*version ;
	const char	*pwd ;
	const char	*progename ;
	const char	*progdname ;
	const char	*progname ;
	const char	*pr ;		/* program-root (normally PCS) */
	const char	*searchname ;
	const char	*banner ;
	const char	*nodename ;	/* USERINFO */
	const char	*domainname ;	/* USERINFO */
	const char	*username ;	/* USERINFO */
	const char	*homedname ;	/* USERINFO */
	const char	*shell ;	/* USERINFO */
	const char	*org ;		/* USERINFO */
	const char	*gecosname ;	/* USERINFO */
	const char	*realname ;	/* USERINFO */
	const char	*name ;		/* USERINFO */
	const char	*fullname ;	/* USERINFO full-name */
	const char	*mailname ;	/* USERINFO mail-abbreviated-name */
	const char	*tz ;		/* USERINFO */
	const char	*maildname ;	/* USERINFO */
	const char	*logid ;	/* USERINFO ID for logging purposes */
	const char	*groupname ;
	const char	*hostname ;
	const char	*tmpdname ;
	const char	*cfname ;
	const char	*lfname ;
	const char	*hfname ;
	const char	*prlocal ;	/* program-root (LOCAL) */
	const char	*udname ;	/* user dir-name? */
	const char	*hostspec ;
	const char	*portspec ;
	void		*efp ;
	void		*userlist ;
	void		*progcs ;
	LOGFILE		lh ;
	PTM		efm ;		/* error-file mutex */
	TERMNOTE	tn ;		/* terminal-note */
	PTM		tmutex ;	/* terminal-note mutex */
	DATER		d ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	PROGINFO_LOG	logdata ;
	time_t		daytime ;
	time_t		ti_start ;
	time_t		ti_loglast ;
	pid_t		pid ;		/* child PID */
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		npar ;		/* n-parallelism */
	int		intrun ;
	int		intnote ;
	int		intdeletecheck ;
	int		intidle ;
	int		logsize ;
	int		notesmax ;
	int		fd_msg ;
	int		c_processed ;
	int		c_updated ;
	char		zname[DATER_ZNAMESIZE+1] ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	const char	**argv ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;


#ifdef	__cplusplus
}
#endif

extern int proginfo_start(PROGINFO *,cchar **,cchar *,cchar *) ;
extern int proginfo_setprogroot(PROGINFO *,const char *,int) ;
extern int proginfo_setentry(PROGINFO *,const char **,const char *,int) ;
extern int proginfo_setversion(PROGINFO *,const char *) ;
extern int proginfo_setbanner(PROGINFO *,const char *) ;
extern int proginfo_setsearchname(PROGINFO *,const char *,const char *) ;
extern int proginfo_setprogname(PROGINFO *,const char *) ;
extern int proginfo_pwd(PROGINFO *) ;
extern int proginfo_progdname(PROGINFO *) ;
extern int proginfo_progename(PROGINFO *) ;
extern int proginfo_nodename(PROGINFO *) ;
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_getename(PROGINFO *,char *,int) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


