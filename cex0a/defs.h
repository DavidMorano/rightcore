/* defs (INCLUDE FILE) */


/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	BUFLEN		(8 * 1024)
#define	CMDBUFLEN	(8 * MAXPATHLEN)

#define	REX_POLLTIME	30000		/* poll timeout */
#define	REX_PINGTO	20

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags


struct proginfo_flags {
	uint		progdash:1;
	uint		akopts:1;
	uint		aparams:1;
	uint		quiet:1;
	uint		errfile:1 ;
	uint		logprog:1;
	uint		verbose:1;
	uint		keepalive:1;
	uint		sanity:1;
	uint		remote:1;
	uint		ni:1;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;			/* program directory */
	const char	*progename ;		/* program directory */
	const char	*progdname ;		/* program directory */
	const char	*progname ;		/* program name */
	const char	*pr ;			/* program root directory */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*clustername ;
	const char	*gecosname ;
	const char	*realname ;
	const char	*name ;
	const char	*fullname ;
	const char	*mailname ;
	const char	*org ;
	const char	*logid ;
	const char	*hostname ;
	const char	*tmpdname ;		/* temporary directory */
	const char	*lfname ;		/* log file name */
	const char	*netfname ;		/* net file name */
	const char	*timestring ;		/* current time (CTIME) */
	void		*userlist ;
	void		*efp ;			/* error Basic file */
	void		*lip ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	logfile		lh ;
	time_t		daytime ;		/* system time of day */
	time_t		intkeep ;		/* keepalive timeout */
	pid_t		pid ;
	uid_t		uid ;			/* real UID */
	uid_t		euid ;			/* effective UID */
	gid_t		gid ;
	gid_t		egid ;
	int		pwdlen ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbosity level */
	int		logsize ;
} ;

struct worm {
	struct proginfo	*pip ;
	const char	wormfname[MAXPATHLEN + 1] ;
	offset_t	offset ;
	int		wfd ;
	int		f_delete ;
} ;

struct jobinfo {
	void		*jfp ;
	int		f_remotedomain ;
	const char	*nodename ;
	const char	*domainname ;
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


