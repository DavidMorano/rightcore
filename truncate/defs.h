/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecstr.h>
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

#ifndef	TTYDEV
#define	TTYDEV		"/dev/tty"
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
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	EX_OLDER
#define	EX_OLDER	1
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		trunclen:1 ;
	uint		literal:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;		/* execution filename */
	const char	*progdname ;		/* dirname of arg[0] */
	const char	*progname ;		/* basename of arg[0] */
	const char	*pr ;			/* program root */
	const char	*rootname ;		/* distribution name */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*name ;			/* person name */
	const char	*logid ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*configfname ;
	const char	*helpfname ;
	const char	*pidfname ;
	const char	*logfname ;
	void		*efp ;
	void		*lip ;			/* local information */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	time_t		daytime ;
	time_t		lastcheck ;
	pid_t		pid ;
	int		pwdlen ;
	int		progmode ;		/* program mode */
	int		debuglevel ;
	int		verboselevel ;
	int		to ;			/* general time-out */
	int		to_open ;		/* open time-out */
	int		to_read ;		/* read time-out */
	int		n ;
	int		logsize ;		/* max log-file size */
	int		intrun ;		/* run interval */
	int		intdis ;		/* disable interval */
	int		intpoll ;		/* poll interval */
	int		intmark ;		/* mark interval */
	int		intlock ;		/* lock interval */
	int		intspeed ;		/* speed-check interval */
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


