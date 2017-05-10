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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif



struct proginfo_flags {
	uint	progdash : 1 ;
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	quiet : 1 ;		/* are we in quiet mode? */
	uint	remove : 1 ;		/* system log */
	uint	log : 1 ;		/* do we have a log file? */
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	char		*pwd ;
	char		*progename ;	/* program directory */
	char		*progdname ;	/* program directory */
	char		*progname ;	/* program name */
	char		*pr ;		/* program root directory */
	char		*searchname ;	/* program search name */
	char		*version ;	/* program version string */
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;	/* ours */
	char		*groupname ;	/* ours */
	char		*logid ;	/* default program LOGID */
	char		*tmpdname ;	/* temporary directory */
	char		*homedname ;
	char		*workdname ;
	char		*pidfname ;
	char		*logfname ;
	char		*lockfname ;
	void		*efp ;
	void		*lfp ;		/* system log */
	void		*pidfp ;
	logfile		lh ;		/* program activity log */
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;	/* verbosity level */
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

extern int proginfo_start(struct proginfo *,char **,const char *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_rootprogdname(struct proginfo *) ;
extern int proginfo_rootexecname(struct proginfo *,const char *) ;
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


