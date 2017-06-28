/* defs */


/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<dater.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"pingstatdb.h"


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
	uint		quiet:1 ;	/* are we in quiet mode? */
	uint		errfile:1 ;
	uint		logprog:1 ;
	uint		logextra:1 ;
	uint		config:1 ;
	uint		update:1 ;	/* update mode */
	uint		binary:1 ;	/* binary dump file */
	uint		receive:1 ;	/* receive mode? */
	uint		dgram:1 ;	/* DGRAM mode */
	uint		nooutput:1 ;	/* no output */
	uint		svars:1 ;
	uint		proglocal:1 ;
	uint		secure_conf:1 ;
	uint		secure_root:1 ;
	uint		pc:1 ;
	uint		params:1 ;
	uint		pidlock:1 ;
	uint		daemon:1 ;
	uint		marktime:1 ;
	uint		logsize:1 ;
	uint		intrun:1 ;
	uint		intmark:1 ;
	uint		defintminping:1 ;
	uint		intminping:1 ;
	uint		intminupdate:1 ;
	uint		toping:1 ;
	uint		pfname:1 ;	/* file-name PID */
	uint		cfname:1 ;	/* file-name CONF */
	uint		lfname:1 ;	/* file-name LOG */
	uint		dfname:1 ;	/* file-name DB */
	uint		logfile:1 ;	/* object */
	uint		sumfile:1 ;	/* object */
	uint		input:1 ;	/* input-mode flag */
	uint		hostdown:1 ;	/* one or more hsots are down */
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;
	cchar		*progdname ;	/* program directory */
	cchar		*progname ;	/* program name */
	cchar		*pr ;		/* program root directory */
	cchar		*searchname ;
	cchar		*version ;	/* program version string */
	cchar		*banner ;
	cchar		*nodename ;
	cchar		*domainname ;
	cchar		*username ;	/* ours */
	cchar		*groupname ;	/* ours */
	cchar		*homedname ;
	cchar		*gecosname ;
	cchar		*realname ;
	cchar		*name ;
	cchar		*fullname ;
	cchar		*mailname ;
	cchar		*org ;
	cchar		*logid ;	/* default program LOGID */
	cchar		*hostname ;
	cchar		*tmpdname ;	/* temporary directory */
	cchar		*workdname ;
	cchar		*afname ;	/* file-name ARGS */
	cchar		*ofname ;	/* file-name OUTPUT */
	cchar		*hfname ;	/* file-name HELP */
	cchar		*cfname ;	/* file-name CONF */
	cchar		*lfname ;	/* file-name LOG */
	cchar		*pfname ;	/* file-name PID */
	cchar		*dfname ;	/* file-name DB */
	cchar		*lockfname ;
	cchar		*ptfname ;	/* file-name PING-TAB */
	cchar		*sumfname ;	/* file-name summary */
	cchar		*hostspec ;
	cchar		*portspec ;
	void		*efp ;
	void		*ofp ;
	void		*pidfp ;	/* file PID */
	void		*sumfp ;	/* file summary */
	void		*userlist ;
	struct timeb	now ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	PINGSTATDB	ps ;
	VECSTR		pingtabs ;
	VECSTR		svars ;
	PARAMFILE	params ;
	EXPCOOK		cooks ;
	logfile		lh ;		/* program activity log */
	vecstr		localnames ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;	/* UIDs */
	gid_t		gid, egid ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;	/* verbosity level */
	int		pagesize ;
	int		logsize ;
	int		intmark ;	/* mark interval */
	int		intrun ;
	int		defintminping ;	/* default minimum interval */
	int		intminping ;	/* minimum interval */
	int		intminupdate ;	/* minimum update interval */
	int		intmininput ;	/* minimum input interval */
	int		toping ;	/* ping timeout */
	int		n ;
	int		c_pingtabs ;	/* number of pingtab files */
	int		c_hosts ;	/* number of hosts specified */
	int		c_up ;		/* number of hosts found UP */
	int		c_processed ;	/* number processed */
	char		zname[DATER_ZNAMESIZE + 1] ;
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
	cchar		*afname ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(PROGINFO *,cchar **,cchar *,cchar *) ;
extern int proginfo_setentry(PROGINFO *,cchar **,cchar *,int) ;
extern int proginfo_setversion(PROGINFO *,cchar *) ;
extern int proginfo_setbanner(PROGINFO *,cchar *) ;
extern int proginfo_setsearchname(PROGINFO *,cchar *,cchar *) ;
extern int proginfo_setprogname(PROGINFO *,cchar *) ;
extern int proginfo_setexecname(PROGINFO *,cchar *) ;
extern int proginfo_setprogroot(PROGINFO *,cchar *,int) ;
extern int proginfo_pwd(PROGINFO *) ;
extern int proginfo_rootname(PROGINFO *) ;
extern int proginfo_progdname(PROGINFO *) ;
extern int proginfo_progename(PROGINFO *) ;
extern int proginfo_nodename(PROGINFO *) ;
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_getename(PROGINFO *,char *,int) ;
extern int proginfo_getenv(PROGINFO *,cchar *,int,cchar **) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


