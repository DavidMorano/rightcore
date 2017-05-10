/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<vecstr.h>
#include	<lfm.h>
#include	<logfile.h>
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
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#define	GROUPNAMELEN	USERNAMELEN
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		(MAXPATHLEN + (4 * 1024))

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#define	STDOUTFNAME	"*STDOUT*"
#define	STDERRFNAME	"*STDERR*"
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags
#define	PROGINFO_STAT	struct proginfo_stat

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_stat {
	uint		total ;
	uint		checked ;
	uint		oversized ;
} ;

struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;	/* are we in quiet mode? */
	uint		errfile:1 ;
	uint		logprog:1 ;	/* do we have a log file? */
	uint		nogo:1 ;	/* flag to prevent actual check */
	uint		pidlock:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;
	const char	*progdname ;
	const char	*progname ;
	const char	*pr ;
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*username ;	/* ours */
	const char	*groupname ;	/* ours */
	const char	*org ;
	const char	*gecosname ;
	const char	*realname ;
	const char	*name ;
	const char	*fullname ;
	const char	*mailname ;
	const char	*logid ;	/* default program LOGID */
	const char	*homedname ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*hostname ;
	const char	*tmpdname ;	/* temporary directory */
	const char	*lfname ;
	const char	*logrootdname ;
	void		*efp ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	PROGINFO_STAT	sall, stab ;
	LOGFILE		lh ;		/* program activity log */
	LFM		pidlock ;
	time_t		daytime ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;
	int		logsize ;
	int		deflogsize ;
	int		c_logtabs ;
	int		c_processed ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
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


