/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecstr.h>
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

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		watchers:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		carriage:1 ;
	uint		files:1 ;
	uint		background:1 ;
	uint		usestdin:1 ;
	uint		useown:1 ;
	uint		wait:1 ;
	uint		fold:1 ;
	uint		clean:1 ;
	uint		ssfile:1 ;
	uint		sxfile:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;
	cchar		*progdname ;
	cchar		*progname ;
	cchar		*pr ;
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*rootname ;
	cchar		*username ;
	cchar		*groupname ;
	cchar		*nodename ;
	cchar		*domainname ;
	cchar		*tmpdname ;
	cchar		*helpfname ;
	cchar		*ssfname ;
	cchar		*sxfname ;
	void		*efp ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	time_t		daytime ;
	pid_t		pid_session ;
	pid_t		pid_parent ;
	pid_t		pid_track ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		linelen ;
	int		indent ;
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
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

extern int proginfo_start(struct proginfo *,cchar **,cchar *,cchar *) ;
extern int proginfo_setprogroot(struct proginfo *,cchar *,int) ;
extern int proginfo_setentry(struct proginfo *,cchar **,cchar *,int) ;
extern int proginfo_setversion(struct proginfo *,cchar *) ;
extern int proginfo_setbanner(struct proginfo *,cchar *) ;
extern int proginfo_setsearchname(struct proginfo *,cchar *,cchar *) ;
extern int proginfo_setprogname(struct proginfo *,cchar *) ;
extern int proginfo_setexecname(struct proginfo *,cchar *) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


