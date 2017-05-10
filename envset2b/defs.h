/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<ids.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"envs.h"


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

/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#ifndef	LOGNAME_MAX
#define	GROUPNAMELEN	LOGNAME_MAX
#else
#define	GROUPNAMELEN	32
#endif
#endif

/* Solaris project name */
#ifndef	PROJNAMELEN
#ifndef	LOGNAME_MAX
#define	PROJNAMELEN	LOGNAME_MAX
#else
#define	PROJNAMELEN	32
#endif
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

/* timezone (zoneinfo) name */
#ifndef	TZLEN
#define	TZLEN		60
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	NULLDEV
#define	NULLDEV		"/dev/null"
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(10 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(11 * MAXPATHLEN)
#endif

#define	BUFLEN		MAX((MAXPATHLEN + MAXNAMELEN),MAXHOSTNAMELEN)

#ifndef	EX_OLDER
#define	EX_OLDER	1
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		background:1 ;
	uint		id:1 ;
	uint		cooks:1 ;
	uint		defs:1 ;
	uint		envs:1 ;
	uint		pvars:1 ;
	uint		exports:1 ;
	uint		cfname:1 ;
	uint		dfname:1 ;
	uint		xfname:1 ;
	uint		config:1 ;
	uint		svars:1 ;
	uint		daemon:1 ;
	uint		shell:1 ;
	uint		loadparams:1 ;
	uint		xeall:1 ;
	uint		xextra:1 ;
	uint		nopreload:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;			/* starting PWD */
	const char	*progename ;		/* execution filename */
	const char	*progdname ;		/* dirname of arg[0] */
	const char	*progname ;		/* basename of arg[0] */
	const char	*pr ;			/* program root */
	const char	*rootname ;		/* distribution name */
	const char	*searchname ;		/* program searchname */
	const char	*version ;		/* program version */
	const char	*banner ;		/* program banner */
	const char	*umachine ;		/* UNAME machine name */
	const char	*usysname ;		/* UNAME OS system-name */
	const char	*urelease ;		/* UNAME OS release */
	const char	*uversion ;		/* UNAME OS version */
	const char	*architecture ;		/* UAUX machine architecture */
	const char	*platform ;		/* UAUX machine platform */
	const char	*provider ;		/* UAUX machine provider */
	const char	*hwserial ;		/* UAUX machine HW-serial */
	const char	*nisdomainname ;	/* UAUX NIS domain */
	const char	*hz ;			/* OS HZ */
	const char	*clustername ;		/* NODEDB */
	const char	*systemname ;		/* NODEDB */
	const char	*nodename ;		/* USERINFO */
	const char	*domainname ;		/* USERINFO */
	const char	*username ;		/* USERINFO */
	const char	*homedname ;		/* USERINFO */
	const char	*shell ;		/* USERINFO */
	const char	*org ;			/* USERINFO */
	const char	*gecosname ; 		/* USERINFO */
	const char	*realname ;		/* USERINFO */
	const char	*name ;			/* USERINFO */
	const char	*printer ;		/* USERINFO */
	const char	*tz ;			/* USERINFO */
	const char	*logid ;		/* USERINFO */
	const char	*fullname ;		/* USERINFO */
	const char	*groupname ;
	const char	*hostname ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*helpfname ;
	const char	*cfname ;
	const char	*dfname ;
	const char	*xfname ;
	const char	*defprog ;
	const char	*conf_defprog ;		/* from configuration */
	const char	*shprog ;		/* from configuration */
	void		*efp ;
	void		*buffer ;		/* general buffer */
	void		*lip ;			/* local information */
	void		*config ;		/* for MSU */
	void		*contextp ;		/* SHELL context */
	PROGINFO_FL	have, f, change, final ;
	PROGINFO_FL	open ;
	IDS		id ;
	EXPCOOK		cooks ;
	VECSTR		svars ;
	VECSTR		defs ;
	VECSTR		pvars, exports ;
	ENVS		xenvs ;
	LOGFILE		lh ;
	time_t		daytime ;
	time_t		lastcheck ;
	pid_t		pid ;
	gid_t		gid, egid ;
	uid_t		uid, euid ;
	int		pwdlen ;
	int		progmode ;		/* program mode */
	int		debuglevel ;
	int		verboselevel ;
	int		to ;			/* general time-out */
	int		to_open ;		/* open time-out */
	int		to_read ;		/* read time-out */
	int		niceval ;
	int		ncpu ;			/* number of CPUs */
	int		loglen ;		/* log-file length */
	int		runint ;		/* run interval */
	int		disint ;		/* disable interval */
	int		pollint ;		/* poll interval */
	int		markint ;		/* mark interval */
	int		lockint ;		/* lock interval */
	int		speedint ;		/* speed-check interval */
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
	int		argr ;
	int		argi ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(PROGINFO *,cchar **,cchar *,cchar *) ;
extern int proginfo_setentry(PROGINFO *,const char **,const char *,int) ;
extern int proginfo_setversion(PROGINFO *,const char *) ;
extern int proginfo_setbanner(PROGINFO *,const char *) ;
extern int proginfo_setsearchname(PROGINFO *,const char *,const char *) ;
extern int proginfo_setprogname(PROGINFO *,const char *) ;
extern int proginfo_setexecname(PROGINFO *,const char *) ;
extern int proginfo_setprogroot(PROGINFO *,const char *,int) ;
extern int proginfo_pwd(PROGINFO *) ;
extern int proginfo_rootname(PROGINFO *) ;
extern int proginfo_progdname(PROGINFO *) ;
extern int proginfo_progename(PROGINFO *) ;
extern int proginfo_nodename(PROGINFO *) ;
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_getename(PROGINFO *,char *,int) ;
extern int proginfo_getenv(PROGINFO *,const char *,int,const char **) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


