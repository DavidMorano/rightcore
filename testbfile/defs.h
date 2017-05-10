/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<logfile.h>
#include	<bfile.h>
#include	<localmisc.h>


#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		(5 * 1024)
#define	JOBIDLEN	14

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	90
#endif

#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2


/* program exit status values */

#define	ES_OK		0		/* completed successfully */
#define	ES_BADARG	1		/* bad argument on invocation */
#define	ES_INFO		2		/* information only */
#define	ES_ERROR	3		/* error during processing */
#define	ES_MUTEX	4		/* mutual exclusion conflict */
#define	ES_BAD		ES_ERROR


struct proginfo_flags {
	uint		fd_stdout:1 ;
	uint		fd_stderr:1 ;	/* do we have STDERR? */
	uint		log:1 ;		/* do we have a log file? */
	uint		slog:1 ;	/* system log */
	uint		verbose:1 ;
	uint		daemon:1 ;	/* are we in daemon mode? */
	uint		quiet:1 ;	/* are we in quiet mode? */
	uint		named:1 ;	/* do we have named services? */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	LOGFILE		lh ;		/* program activity log */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*efp ;
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;
	int		debuglevel ;	/* debugging level */
	int		interval ;	/* override interval */
	int		maxjobs ;
	const char	*progname ;	/* program name */
	const char	*version ;	/* program version string */
	const char	*programroot ;	/* program root directory */
	const char	**envv ;	/* program start-up environment */
	const char	*nodename ;
	const char	*domainname ;
	const char	*hostname ;	/* concatenation of N + D */
	const char	*username ;	/* ours */
	const char	*groupname ;	/* ours */
	const char	*pwd ;
	const char	*logid ;	/* default program LOGID */
	const char	*workdir ;
	const char	*tmpdir ;	/* temporary directory */
	const char	*stampdir ;	/* timestamp directory */
	const char	*pidfname ;
	const char	*lockfname ;	/* lock file */
	const char	*prog_rmail ;
	const char	*prog_sendmail ;
	const char	*defuser ;	/* default for servers */
	const char	*defgroup ;	/* default for servers */
	const char	*userpass ;	/* user password file */
	const char	*machpass ;	/* machine password file */
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


