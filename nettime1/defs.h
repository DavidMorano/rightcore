/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<time.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<logsys.h>
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
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(6 * MAXPATHLEN)
#endif

#undef	BUFLEN
#define	BUFLEN		LINEBUFLEN

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
	uint		print:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		config:1 ;
	uint		pidfile:1 ;
	uint		lockfile:1 ;
	uint		logprog:1 ;
	uint		logsys:1 ;
	uint		pidlock:1 ;
	uint		daemon:1 ;
	uint		all:1 ;
	uint		def:1 ;
	uint		test:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;		/* execution filename */
	cchar		*progdname ;		/* dirname of arg[0] */
	cchar		*progname ;		/* basename of arg[0] */
	cchar		*pr ;			/* program root */
	cchar		*rootname ;		/* distribution name */
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*nodename ;		/* USERINFO */
	cchar		*domainname ;		/* USERINFO */
	cchar		*username ;		/* USERINFO */
	cchar		*homedname ;		/* USERINFO */
	cchar		*gecosname ;		/* USERINFO */
	cchar		*name ;			/* USERINFO */
	cchar		*shell ;		/* USERINFO */
	cchar		*org ;			/* USERINFO */
	cchar		*mailname ;		/* USERINFO */
	cchar		*logid ;		/* USERINFO */
	cchar		*groupname ;
	cchar		*svc ;
	cchar		*tmpdname ;
	cchar		*helpfname ;
	cchar		*ofname ;
	cchar		*lfname ;
	void		*efp ;
	void		*outfile ;
	void		*uip ;
	void		*ofile ;
	PROGINFO_FL	have, f, change, final ;
	PROGINFO_FL	open ;
	LOGFILE		lf ;
	LOGSYS		ls ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		euid, uid ;
	gid_t		egid, gid ;
	int		pwdlen ;
	int		progmode ;		/* program mode */
	int		debuglevel ;
	int		verboselevel ;
	int		to ;			/* general time-out */
	int		to_open ;		/* open time-out */
	int		to_read ;		/* read time-out */
	int		n ;
	int		proto ;
	int		af ;
	int		logsize ;		/* log-file length */
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
} ;

struct arginfo {
	cchar		**argv ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,cchar **,cchar *,cchar *) ;
extern int proginfo_setentry(struct proginfo *,cchar **,cchar *,int) ;
extern int proginfo_setversion(struct proginfo *,cchar *) ;
extern int proginfo_setbanner(struct proginfo *,cchar *) ;
extern int proginfo_setsearchname(struct proginfo *,cchar *,cchar *) ;
extern int proginfo_setprogname(struct proginfo *,cchar *) ;
extern int proginfo_setexecname(struct proginfo *,cchar *) ;
extern int proginfo_setprogroot(struct proginfo *,cchar *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,cchar *,int,cchar **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


