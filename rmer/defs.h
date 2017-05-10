/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
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

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;	/* are we in quiet mode? */
	uint		errfile:1 ;
	uint		log:1 ;		/* do we have a log file? */
	uint		slog:1 ;	/* system log */
	uint		verbose:1 ;
	uint		msgdiscard:1 ;	/* message-discard mode */
} ;

struct proginfo {
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* program directory */
	const char	*progdname ;	/* program directory */
	const char	*progname ;	/* program name */
	const char	*pr ;		/* program root directory */
	const char	*searchname ;	/* program search name */
	const char	*version ;	/* program version string */
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;	/* ours */
	const char	*groupname ;	/* ours */
	const char	*logid ;	/* default program LOGID */
	const char	*tmpdname ;	/* temporary directory */
	const char	*homedname ;
	const char	*workdname ;
	const char	*pidfname ;
	const char	*logfname ;
	const char	*lockfname ;
	void		*efp ;
	vecstr		stores ;
	vecstr		afiles ;
	logfile		lh ;		/* program activity log */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;	/* verbosity level */
	int		pagesize ;
	int		to_file ;
	int		c_files ;
	int		c_remoted ;
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


