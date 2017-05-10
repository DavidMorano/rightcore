/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>

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

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
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

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#define	BUFLEN		((4 * MAXPATHLEN) + 20)

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1;
	uint		aparams:1;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		print:1 ;
	uint		add:1 ;
	uint		inc:1 ;
	uint		logprog:1 ;
	uint		binder:1 ;
	uint		all:1 ;
	uint		query:1 ;
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
	const char	*nodename ;	/* USERINFO */
	const char	*domainname ;	/* USERINFO */
	const char	*username ;	/* USERINFO */
	const char	*homedname ;	/* USERINFO */
	const char	*shell ;	/* USERINFO */
	const char	*org ;		/* USERINFO */
	const char	*gecosname ;	/* USERINFO */
	const char	*realname ;	/* USERINFO */
	const char	*name ;		/* USERINFO */
	const char	*fullname ;	/* USERINFO full-name */
	const char	*mailname ;	/* USERINFO mail-abbreviated-name */
	const char	*tz ;		/* USERINFO */
	const char	*maildname ;	/* USERINFO */
	const char	*logid ;	/* USERINFO ID for logging purposes */
	const char	*hostname ;
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*hfname ;
	const char	*lfname ;
	void		*efp ;
	void		*uip ;		/* USERINFO object */
	void		*userlist ;	/* user-list state */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	LOGFILE		lh ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		intrun ;
} ;

struct options {
	uint		lower:1 ;
	uint		inplace:1 ;
	uint		doublespace:1 ;
	uint		half:1 ;
	uint		leading:1 ;
	uint		mscrap:1 ;		/* M$ crap! */
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


