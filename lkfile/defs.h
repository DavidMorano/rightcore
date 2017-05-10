/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<bfile.h>
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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(MAXPATHLEN + (4 * 1024))

/* extra exit codes */

#define	EX_TIMEDOUT	1
#define	EX_BADINIT	2
#define	EX_INVALID	3


struct proginfo_flags {
	uint		progdash : 1 ;
	uint		akopts : 1 ;
	uint		aparams : 1 ;
	uint		quiet : 1 ;
	uint		log : 1 ;		/* do we have a log file ? */
} ;

struct proginfo {
	VECSTR		stores ;
	const char	**envv ;
	char		*pwd ;
	char		*progename ;
	char		*progdname ;
	char		*progname ;		/* program name */
	char		*pr ;			/* program root directory */
	char		*searchname ;
	char		*version ;		/* program version string */
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;		/* ours */
	char		*groupname ;		/* ours */
	char		*homedname ;
	char		*workdname ;
	char		*tmpdname ;		/* temporary directory */
	char		*pidfname ;
	char		*lockfname ;		/* lock file */
	char		*fileroot ;
	bfile		*efp ;
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	logfile		lh ;		/* program activity log */
	time_t		daytime ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;		/* UIDs */
	gid_t		gid, egid ;
	int		pwdlen ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbosity level */
	int		logsize ;
	char		logid[LOGIDLEN + 1] ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;


#ifdef	__cplusplus
}
#endif

extern int proginfo_start(struct proginfo *,char **,const char *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_rootprogdname(struct proginfo *) ;
extern int proginfo_rootexecname(struct proginfo *,const char *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


