/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	RMAILERD_INCLUDE
#define	RMAILERD_INCLUDE	1


#include	<envstandards.h>

#include	<sys/param.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"recipient.h"


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

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	HOSTPARTLEN	(MAXHOSTNAMELEN + 200)
#define	LOCALPARTLEN	MAXHOSTNAMELEN

#define	BUFLEN		(MAXHOSTNAMELEN * 10)
#define	LINELEN		(HOSTPARTLEN + LOCALPARTLEN + 100)


/* SENDMAIL Input Modes */

#define	IM_SEEK		0
#define	IM_PIPE		1
#define	IM_FILE 	2


/* processing options */

#define	OPT_COMPRESSED	"compressed"
#define	OPT_ENCRYPTED	"encrypted"


/* pogram information */

struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;	/* are we in quiet mode ? */
	uint		errfile:1 ;	/* do we have STDERR ? */
	uint		logprog:1 ;	/* program log */
	uint		logsys:1 ;	/* system log */
	uint		verbose:1 ;
	uint		daemon:1 ;	/* are we in daemon mode ? */
	uint		sender:1 ;	/* send sender on envelope */
	uint		optin:1 ;
	uint		svars:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;		/* program directory */
	const char	*progdname ;		/* program directory */
	const char	*progname ;		/* program name */
	const char	*pr ;			/* program root directory */
	const char	*searchname ;
	const char	*version ;		/* program version string */
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*maildomain ;
	const char	*tmpdname ;		/* temporary directory */
	const char	*workdname ;
	const char	*pidfname ;		/* mutex PID file */
	const char	*lockfname ;		/* lock file */
	const char	*userpass ;		/* user password file */
	const char	*machpass ;		/* machine password file */
	const char	*prog_sendmail ;	/* 'sendmail' program path */
	const char	*prog_dmail ;
	const char	*xfname ;
	const char	*logid ;
	void		*efp ;		/* system log "Basic" file */
	void		*lfp ;		/* system log "Basic" file */
	void		*lockfp ;	/* BIO FP for lock file */
	void		*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	logfile		lh ;			/* program activity log */
	time_t		daytime ;
	pid_t		ppid, pid ;
	uid_t		uid ;			/* real UID */
	uid_t		euid ;			/* effective UID */
	gid_t		gid ;
	int		pwdlen ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;
	int		nrecips ;		/* number of recips at a time */
} ;

struct prog_params {
	const char	*protocol ;
	const char	*transport_host ;
	const char	*envelope_host ;
	const char	*envelope_from ;
	RECIPIENT_VALUE	*rp ;
	int		ofd, efd ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
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


