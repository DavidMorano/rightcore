/* defs */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>		/* for 'struct timeb' */
#include	<limits.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<ids.h>
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

/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#ifndef	LOGNAME_MAX
#define	GROUPNAMELEN	LOGNAME_MAX
#else
#define	GROUPNAMELEN	32
#endif
#endif

/* Solaris project name */
#ifndef	PROJNAMELEN
#ifndef	LOGNAME_MAX
#define	PROJNAMELEN	LOGNAME_MAX
#else
#define	PROJNAMELEN	32
#endif
#endif

#ifndef	MAXNETNAMELEN
#define	MAXNETNAMELEN	255	/* maximum length of network user's name */
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

/* timezone (zoneinfo) name */
#ifndef	TZLEN
#define	TZLEN		60
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#ifndef	ORGCODELEN
#define	ORGCODELEN	80
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	NULLDEV
#define	NULLDEV		"/dev/null"
#endif

#ifndef	TTYDEV
#define	TTYDEV		"/dev/tty"
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

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	EX_OLDER
#define	EX_OLDER	1
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


struct proginfo_flags {
	uint		progdash:1 ;	/* leading dash on program-name */
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		ignore:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
	uint		pcsconf:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		pidfname:1 ;
	uint		logprog:1 ;
	uint		config:1 ;
	uint		watch:1 ;
	uint		background:1 ;
	uint		daemon:1 ;
	uint		logsize:1 ;
	uint		nodeonly:1 ;
	uint		tmpdate:1 ;
	uint		disable:1 ;
	uint		all:1 ;
	uint		def:1 ;
	uint		list:1 ;
	uint		bufwhole:1 ;
	uint		bufline:1 ;
	uint		bufnone:1 ;
	uint		intrun:1 ;
	uint		intidle:1 ;
	uint		intpoll:1 ;
	uint		intmark:1 ;
	uint		intlock:1 ;
	uint		intdis:1 ;
	uint		reuseaddr:1 ;
	uint		to:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;	/* execution filename */
	cchar		*progdname ;	/* dirname of arg[0] */
	cchar		*progname ;	/* basename of arg[0] */
	cchar		*pr ;		/* program root */
	cchar		*rootname ;	/* distribution name */
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*umachine ;	/* UNAME machine name */
	cchar		*usysname ;	/* UNAME OS system-name */
	cchar		*urelease ;	/* UNAME OS release */
	cchar		*uversion ;	/* UNAME OS version */
	cchar		*architecture ;	/* UAUX machine architecture */
	cchar		*platform ;	/* UAUX machine platform */
	cchar		*provider ;	/* UAUX machine provider */
	cchar		*hwserial ;	/* UAUX machine hwserial */
	cchar		*nisdomain ;	/* UAUX machine nisdomain */
	cchar		*hz ;		/* OS HZ */
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
	cchar		*orgcode ;
	cchar		*groupname ;
	cchar		*hostname ;
	cchar		*tmpdname ;
	cchar		*newsdname ;
	cchar		*hfname ;
	cchar		*cfname ;
	cchar		*lfname ;
	cchar		*pidfname ;
	void		*efp ;
	void		*buffer ;	/* general buffer */
	void		*contextp ;	/* SHELL context */
	void		*lip ;		/* local information */
	void		*uip ;		/* USERINFO object */
	void		*userlist ;	/* user-list state */
	void		*config ;	/* configuration */
	void		*watch ;	/* watch */
	void		*pcsconf ;	/* save space when not needed */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	struct timeb	now ;
	IDS		id ;
	LOGFILE		lh ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		progmode ;	/* program mode */
	int		debuglevel ;
	int		verboselevel ;
	int		quietlevel ;
	int		ncpu ;
	int		to ;		/* general time-out */
	int		to_open ;	/* open time-out */
	int		to_read ;	/* read time-out */
	int		n ;
	int		logsize ;	/* max log-file size */
	int		intrun ;	/* interval run */
	int		intidle ;	/* interval idle */
	int		intpoll ;	/* interval poll */
	int		intmark ;	/* interval mark */
	int		intlock ;	/* interval lock */
	int		intdis ;	/* interval disable */
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


