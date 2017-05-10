/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


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
#define	MAXPATHLEN	1024
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		(MAXPATHLEN + (4 * 1024))

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		logprog:1 ;
	uint		logsize:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progdname ;	/* program directory */
	cchar		*progename ;	/* program directory */
	cchar		*progname ;	/* program name */
	cchar		*pr ;		/* program root directory */
	cchar		*searchname ;
	cchar		*rootname ;
	cchar		*version ;	/* program version string */
	cchar		*banner ;
	cchar		*nodename ;	/* USERINFO */
	cchar		*domainname ;	/* USERINFO */
	cchar		*username ;	/* USERINFO */
	cchar		*homedname ;	/* USERINFO */
	cchar		*shell ;	/* USERINFO */
	cchar		*org ;		/* USERINFO */
	cchar		*gecosname ;	/* USERINFO */
	cchar		*realname ;	/* USERINFO */
	cchar		*name ;		/* USERINFO */
	cchar		*fullname ;	/* USERINFO full-name */
	cchar		*mailname ;	/* USERINFO mail-abbreviated-name */
	cchar		*tz ;		/* USERINFO */
	cchar		*maildname ;	/* USERINFO */
	cchar		*logid ;	/* USERINFO ID for logging purposes */
	cchar		*groupname ;	/* ours */
	cchar		*hostname ;
	cchar		*tmpdname ;	/* temporary directory */
	cchar		*workdname ;
	cchar		*helpfname ;
	cchar		*pidfname ;
	cchar		*lockfname ;	/* lock file */
	cchar		*cfname ;
	cchar		*lfname ;
	cchar		*pwfname ;	/* 'passwd' file */
	cchar		*fileroot ;
	cchar		*typespec ;	/* output file type */
	cchar		*dbname ;
	void		*efp ;
	void		*lockfp ;	/* BIO FP for lock file */
	void		*pidfp ;	/* BIO FP for PID file */
	void		*userlist ;
	void		*outfile ;
	PROGINFO_FL	have, f, final ;
	PROGINFO_FL	open ;
	LOGFILE		lh ;		/* program activity log */
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;	/* GIDs */
	gid_t		gid_tools ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;
	int		logsize ;
	int		n ;
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
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


