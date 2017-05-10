/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<dater.h>
#include	<ids.h>
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

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
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
	uint		log:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;		/* execution filename */
	cchar		*progdname ;		/* dirname of arg[0] */
	cchar		*progname ;		/* basename of arg[0] */
	cchar		*pr ;			/* program root */
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*nodename ;
	cchar		*domainname ;
	cchar		*rootname ;		/* distribution name */
	cchar		*username ;
	cchar		*groupname ;
	cchar		*tmpdname ;
	cchar		*maildname ;
	cchar		*helpfname ;
	void		*efp ;
	void		*buffer ;		/* general buffer */
	void		*cip ;			/* configuration information */
	void		*config ;		/* for MSU */
	void		*contextp ;		/* SHELL context */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	struct timeb	now ;
	LOGFILE		lh ;
	DATER		tmpdate, mdate ;
	IDS		id ;
	time_t		daytime ;
	time_t		lastcheck ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		to ;			/* general time-out */
	int		n ;
	int		loglen ;		/* log-file length */
	int		runint ;		/* run interval */
	int		disint ;		/* disable interval */
	int		pollint ;		/* poll interval */
	int		markint ;		/* mark interval */
	int		lockint ;		/* lock interval */
	int		speedint ;		/* speed-check interval */
	char		logid[LOGIDLEN + 1] ;
	char		cmd[LOGIDLEN + 1] ;
	char		logfname[MAXNAMELEN + 1] ;
	char		msfname[MAXNAMELEN + 1] ;
	char		pidfname[MAXNAMELEN + 1] ;
	char		speedname[MAXNAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,const char *,
		const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,const char *,int,const char **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


