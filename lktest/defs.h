/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<localmisc.h>


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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(MAXPATHLEN + (4 * 1024))

/* extra exit codes */

#define	EX_TIMEDOUT	1
#define	EX_BADINIT	2
#define	EX_INVALID	3






struct proginfo_flags {
	uint	progdash : 1 ;
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	quiet : 1 ;		/* are we in quiet mode? */
	uint	log : 1 ;		/* do we have a log file? */
	uint	readlock : 1 ;
} ;

struct proginfo {
	vecstr	stores ;
	char	**envv ;
	char	*pwd ;
	char	*progename ;
	char	*progdname ;		/* program directory */
	char	*progname ;		/* program name */
	char	*pr ;			/* program root directory */
	char	*searchname ;
	char	*version ;		/* program version string */
	char	*banner ;
	char	*nodename ;
	char	*domainname ;
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*tmpdname ;		/* temporary directory */
	char	*homedname ;
	char	*workdname ;
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*fileroot ;
	char	*logid ;		/* default program LOGID */
	void	*efp ;
	void	*lfp ;		/* system log "Basic" file */
	void	*lockfp ;	/* BIO FP for lock file */
	void	*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	logfile		lh ;
	time_t	daytime ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	pwdlen ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
	int	holdtime ;
} ;

struct pivars {
	char	*vpr1 ;
	char	*vpr2 ;
	char	*vpr3 ;
	char	*pr ;
	char	*vprname ;
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
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


