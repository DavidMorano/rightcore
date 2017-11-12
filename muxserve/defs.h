/* defs */


/* Copyright © 1999,2008 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<uname.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<expcook.h>
#include	<paramfile.h>
#include	<lfm.h>
#include	<logfile.h>
#include	<svcfile.h>
#include	<acctab.h>
#include	<sockaddress.h>
#include	<varsub.h>
#include	<localmisc.h>

#include	"envs.h"


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

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
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

/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	PEERNAMELEN
#ifdef	CONNECTION_PEERNAMELEN
#define	PEERNAMELEN	CONNECTION_PEERNAMELEN
#else
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	PROVIDERLEN
#define	PROVIDERLEN	100
#endif

#ifndef	ORGLEN
#define	ORGLEN		32
#endif

#ifndef	ORGCODELEN
#define	ORGCODELEN	32
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
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

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	SVCBUFLEN
#define	SVCBUFLEN	(4 * MAXPATHLEN)
#endif

#ifndef	VARNLSADDR
#define	VARNLSADDR	"NLSADDR"
#endif

#ifndef	VARTERM
#define	VARTERM		"TERM"
#endif

#define	BUFLEN		MAX((MAXPATHLEN + MAXNAMELEN),MAXHOSTNAMELEN)

#define	JOBIDLEN	14

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags
#define	PROGINFO_IPC	struct proginfo_ipc

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;		/* are we in quiet mode? */
	uint		errfile:1 ;
	uint		proglocal:1 ;
	uint		svars:1 ;
	uint		cmds:1 ;
	uint		pc:1 ;			/* program configuration */
	uint		params:1 ;		/* program parameters */
	uint		onckey:1 ;		/* we have an ONC private key */
	uint		logprog:1 ;		/* logging enabled? */
	uint		logsize:1 ;
	uint		daemon:1 ;		/* are we in daemon mode? */
	uint		named:1 ;		/* do we have named services? */
	uint		force:1 ;		/* force servicing */
	uint		passfd:1 ;		/* perform client FD passing */
	uint		mspoll:1 ;
	uint		orgcode:1 ;
	uint		intmin:1 ;		/* minimum interval */
	uint		intpoll:1 ;
	uint		intrun:1 ;
	uint		intmark:1 ;
	uint		intlock:1 ;
	uint		pidlock:1 ;
	uint		stampdname:1 ;		/* stamp directory */
	uint		rundname:1 ;		/* run directory */
	uint		loginsvc:1 ;		/* login finger service */
	uint		usersrv:1 ;		/* user-server-file */
	uint		stampfname:1 ;
	uint		cfname:1 ;
	uint		lfname:1 ;
	uint		pidfname:1 ;
	uint		svcfname:1 ;		/* service file */
	uint		accfname:1 ;		/* access file */
	uint		passfname:1 ;
	uint		lockfname:1 ;
	uint		reqfname:1 ;
	uint		reqfnametmp:1 ;
	uint		reuseaddr:1 ;		/* reuse-address */
	uint		path:1 ;		/* we have a path file */
	uint		defacc:1 ;		/* default access */
	uint		defnoserver:1 ;		/* use default server if none */
	uint		defpidlock:1 ;		/* use PID-locking by default */
	uint		defsvc:1 ;		/* allow a default service */
	uint		linelen:1 ;
	uint		showsysbanner:1 ;	/* show system banner */
	uint		background:1 ;		/* background mode */
	uint		caf:1 ;			/* close-all-files */
	uint		cmd:1 ;			/* enter "command" mode */
	uint		useracct:1 ;		/* user-account matches */
	uint		torecvfd:1 ;
	uint		tosendfd:1 ;
	uint		uniq:1 ;
	uint		tmptype:1 ;
	uint		secure:1 ;		/* all secure */
	uint		secure_root:1 ;		/* secure root directory */
	uint		secure_conf:1 ;		/* secure configuration file */
	uint		secure_svcfile:1 ;
	uint		secure_accfile:1 ;
	uint		secure_path:1 ;
} ;

struct proginfo_ipc {
	const char	*fname ;
	SOCKADDRESS	sa ;			/* peername socket address */
	int		salen ;
	int		fd_req ;
} ;

struct proginfo {
	const char	**envv ;		/* start-up environment */
	const char	*pwd ;
	const char	*progename ;
	const char	*progdname ;
	const char	*progname ;		/* program name */
	const char	*pr ;			/* program root directory */
	const char	*version ;		/* program version string */
	const char	*banner ;
	const char	*searchname ;		/* program search name */
	const char	*usysname ;		/* UNAME OS system-name */
	const char	*urelease ;		/* UNAME OS release */
	const char	*uversion ;		/* UNAME OS version */
	const char	*umachine ;		/* UNAME machine name */
	const char	*architecture ;		/* UAUX machine architecture */
	const char	*platform ;		/* UAUX machine platform */
	const char	*provider ;		/* UAUX machine provider */
	const char	*hz ;			/* OS HZ */
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
	const char	*groupname ;
	const char	*hostname ;		/* concatenation of N + D */
	const char	*rootname ;
	const char	*speedname ;
	const char	*bestname ;
	const char	*defuser ;		/* default for servers */
	const char	*defgroup ;		/* default for servers */
	const char	*tmpdname ;		/* temporary directory */
	const char	*workdname ;
	const char	*stampdname ;		/* timestamp directory */
	const char	*rundname ;
	const char	*jobdname ;
	const char	*cfname ;
	const char	*lfname ;
	const char	*stampfname ;
	const char	*pidfname ;
	const char	*lockfname ;		/* lock file */
	const char	*reqfname ;
	const char	*svcfname ;
	const char	*accfname ;
	const char	*msfname ;
	const char	*outfname ;
	const char	*passfname ;
	const char	*usersrv ;
	const char	*svcpass ;		/* "pass"-mode service */
	const char	*defpath ;		/* default (system) path */
	const char	*defacc ;		/* default access group */
	const char	*prog_rmail ;
	const char	*prog_sendmail ;
	void		*sip ;
	void		*efp ;
	void		*userlist ;
	PROGINFO_IPC	ipc ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	fromconf ;
	PROGINFO_FL	open ;
	IDS		id ;
	UNAME		un ;
	USERINFO	*uip ;
	VECSTR		svars ;
	EXPCOOK		cooks ;
	PARAMFILE	params ;
	LFM		pidlock, genlock ;
	LOGFILE		lh ;			/* program activity log */
	VECSTR		defs ;
	ENVS		xenvs ;
	VARSUB		subs ;
	SVCFILE		stab ;			/* service table */
	ACCTAB		atab ;			/* access table */
	KEYOPT		cmds ;			/* for "command" mode */
	vecstr		stores ;
	vecstr		pvars, exports ;
	vecstr		pathexec ;
	vecstr		pathlib ;
	vecobj		listens ;
	time_t		daytime ;
	pid_t		pid, ppid, spid ;
	uid_t		uid, euid ;		/* UIDs */
	gid_t		gid, egid ;
	gid_t		gid_rootname ;
	uint		hostid ;
	int		pwdlen ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbosity level */
	int		ncpu ;
	int		serial ;
	int		subserial ;
	int		intmin ;		/* minimum interval (secs) */
	int		intpoll ;		/* poll intercal (secs) */
	int		intrun ;		/* run interval (secs) */
	int		intmark ;
	int		intlock ;
	int		logsize ;
	int		maxjobs ;
	int		providerid ;
	int		linelen ;
	int		tmptype ;
	int		fd_listentcp ;
	int		fd_listenpass ;
	int		fd_pass ;
	int		to_recvfd ;
	int		to_sendfd ;
	char		orgcode[ORGCODELEN+1] ;
	char		cmd[LOGIDLEN + 1] ;
	char		defsvc[SVCNAMELEN + 1] ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	BITS		pargs ;
	int		argc ;
	const char	**argv ;
	int		ai_max ;
	int		ai_pos ;
} ;

/* this is similar to old NETBUF structure */
struct resultbuf {
	int		maxlen ;
	int		len ;
	char		*buf ;
} ;

enum tmptypes {
	tmptype_system,
	tmptype_user,
	tmptype_overlast
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


