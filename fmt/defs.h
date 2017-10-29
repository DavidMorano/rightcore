/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>		/* for 'struct timeb' */
#include	<limits.h>

#include	<vecstr.h>
#include	<logfile.h>
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

#ifndef	PWBUFLEN
#define	PWBUFLEN	1024		/* _SC_GETPW_R_SIZE_MAX */
#endif

#ifndef	GRBUFLEN
#define	GRBUFLEN	7296		/* _SC_GETGR_R_SIZE_MAX */
#endif

#ifndef	PJBUFLEN
#define	PJBUFLEN	(4 * 1024)	/* Solaris recommends (4*1024) */
#endif

/* protocol entry */
#ifndef	PEBUFLEN
#define	PEBUFLEN	100
#endif

/* host entry */
#ifndef	HEBUFLEN
#define	HEBUFLEN	(8 * 1024)
#endif

/* service entry */
#ifndef	SEBUFLEN
#define	SEBUFLEN	100
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

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#ifndef	ORGLEN
#define	ORGLEN		80
#endif

#ifndef	NOFILE
#define	NOFILE		20
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

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
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
#define	DIGBUFLEN	45		/* can hold int128_t in decimal */
#endif

#ifndef	EX_OLDER
#define	EX_OLDER	1
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;	/* leading dash on program-name */
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		ignore:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
	uint		pcsconf:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		logprog:1 ;
	uint		pidfile:1 ;
	uint		pidfname:1 ;
	uint		config:1 ;
	uint		daemon:1 ;
	uint		logsize:1 ;
	uint		nodeonly:1 ;
	uint		tmpdate:1 ;
	uint		disable:1 ;
	uint		all:1 ;
	uint		def:1 ;
	uint		list:1 ;
	uint		bufwhole:1 ;
	uint		bufline:1 ;
	uint		bufnone:1 ;
	uint		intrun:1 ;
	uint		intdis:1 ;
	uint		intpoll:1 ;
	uint		intlock:1 ;
	uint		intmark:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* execution filename */
	const char	*progdname ;	/* dirname of arg[0] */
	const char	*progname ;	/* basename of arg[0] */
	const char	*pr ;		/* program root */
	const char	*rootname ;	/* distribution name */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*usysname ;	/* UNAME OS system-name */
	const char	*umachine ;	/* UNAME machine name */
	const char	*urelease ;	/* UNAME OS release */
	const char	*uversion ;	/* UNAME OS version */
	const char	*architecture ;	/* UAUX machine architecture */
	const char	*platform ;	/* UAUX machine platform */
	const char	*provider ;	/* UAUX machine provider */
	const char	*hz ;		/* OS HZ */
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
	const char	*newsdname ;
	const char	*helpfname ;
	const char	*cfname ;
	const char	*lfname ;
	const char	*pidfname ;
	void		*efp ;
	void		*buffer ;	/* general buffer */
	void		*lip ;		/* local information */
	void		*config ;	/* configuration */
	void		*pcp ;		/* save space when not needed */
	void		*contextp ;	/* SHELL context */
	void		*userlist ;	/* user-list state */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	struct timeb	now ;
	IDS		id ;
	LOGFILE		lh ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		progmode ;	/* program mode */
	int		debuglevel ;
	int		verboselevel ;
	int		quietlevel ;
	int		to ;		/* general time-out */
	int		to_open ;	/* open time-out */
	int		to_read ;	/* read time-out */
	int		n ;
	int		logsize ;	/* max log-file size */
	int		intrun ;	/* run interval */
	int		intdis ;	/* disable interval */
	int		intpoll ;	/* poll interval */
	int		intmark ;	/* mark interval */
	int		intlock ;	/* lock interval */
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
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,
		const char *,const char *) ;
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


