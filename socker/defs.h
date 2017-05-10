/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<logfile.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<varsub.h>
#include	<pwfile.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<lfm.h>
#include	<sockaddress.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"jobdb.h"


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

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		(MAXPATHLEN + 100)
#define	JOBIDLEN	14

#define	SPECIALHOSTADDR	0x812cbf37
#define	SPECIALHOSTNAME	"uucp.rightcore.com"


/* connection failure codes (returned to clients) */

#define	SERVICE_CFNOSVC		1	/* specified service not found */
#define	SERVICE_CFACCESS	2	/* access denied to host for service */
#define	SERVICE_CFNOSRV		3	/* server was not found */

/* connection failure codes (returned to clients) */

#define	TCPMUXD_CFOK		0
#define	TCPMUXD_CFNOSVC		1	/* specified service not found */
#define	TCPMUXD_CFACCESS	2	/* access denied to host for service */
#define	TCPMUXD_CFNOSRV		3	/* server was not configured */
#define	TCPMUXD_CFNOEXIST	4	/* server does not exist */
#define	TCPMUXD_CFOVERLAST	5


/* compilation options */

#define	DEFS_ROOTNAME	1


struct proginfo_config {
	PARAMFILE	p ;
	EXPCOOK	cooks ;
	uint		f_p ;
} ;

struct proginfo_flags {
	uint	progdash : 1 ;
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	quiet : 1 ;		/* are we in quiet mode? */
	uint	svars : 1 ;
	uint	ofile : 1 ;
	uint	efile : 1 ;		/* do we have STDERR? */
	uint	log : 1 ;		/* do we have a log file? */
	uint	slog : 1 ;		/* system log */
	uint	daemon : 1 ;		/* are we in daemon mode? */
	uint	loginsvc : 1 ;		/* login finger service? */
	uint	srvtab : 1 ;		/* do we have an SRVTAB? */
	uint	acctab : 1 ;		/* do we have an ACCess TABle? */
	uint	passwd : 1 ;		/* PWFILE? */
	uint	lockfile : 1 ;		/* have a lock file */
	uint	mspoll : 1 ;		/* do MS polling */
	uint	geekout : 1 ;
	uint	zerospeed : 1 ;		/* zero out 'speed' element */
	uint	nopass : 1 ;		/* don't listen on PASS */
	uint	msfile : 1 ;
	uint	loglen : 1 ;
	uint	lockint : 1 ;
	uint	runint : 1 ;
	uint	pollint : 1 ;
	uint	speedint : 1 ;
	uint	markint : 1 ;
	uint	pidfile : 1 ;
	uint	logfile : 1 ;
	uint	cmd : 1 ;
	uint	reuse : 1 ;
	uint	portspec : 1 ;
	uint	logfname : 1 ;
	uint	pidfname : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* program execute name */
	const char	*progdname ;	/* program directory */
	const char	*progname ;	/* program name */
	const char	*pr ;		/* program root directory */
	const char	*searchname ;	/* program search name */
	const char	*rootname ;	/* programroot name */
	const char	*version ;	/* program version string */
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*hostname ;	/* concatenation of N + D */
	const char	*username ;	/* ours */
	const char	*groupname ;	/* ours */
	const char	*logid ;	/* default program LOGID */
	const char	*tmpdname ;	/* temporary directory */
	const char	*workdname ;
	const char	*vardname ;	/* VAR directory */
	const char	*spooldname ;	/* spool directory */
	const char	*confname ;
	const char	*pidfname ;
	const char	*lockfname ;	/* lock file */
	const char	*reqfname ;	/* request file */
	const char	*shmfname ;	/* SHM file */
	const char	*passfname ;	/* pass (FD) file */
	const char	*msfname ;	/* MS file */
	const char	*logfname ;
	const char	*defacc ;	/* default access name */
	const char	*prog_rmail ;
	const char	*prog_sendmail ;
	const char	*defuser ;	/* default for servers */
	const char	*defgroup ;	/* default for servers */
	const char	*userpass ;	/* user password file */
	const char	*machpass ;	/* machine password file */
	const char	*org ;
	const char	*orgcode ;	/* organization-code (for DAYTIME) */
	const char	*speedname ;	/* CPUSPEED module name */
	void		*efp ;
	void		*pidfp ;	/* BIO FP for PID file */
	void		*config ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	IDS		id ;
	LOGFILE		lh ;			/* program activity log */
	varsub		subs ;			/* substitutions */
	vecstr		exports ;		/* exports */
	PWFILE		passwd ;		/* passwd file object */
	SRVTAB		stab ;
	ACCTAB		atab ;
	LFM		plock ;			/* program lock */
	JOBDB		jobs ;
	SOCKADDRESS	sa ;			/* IPC socket address */
	time_t		daytime ;
	pid_t		pid, ppid ;
	uid_t		uid, euid ;		/* UIDs */
	gid_t		gid, egid ;
	uint		providerid, hostid ;
	int		pwdlen ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbose output */
	int		lockint ;
	int		runint ;		/* run interval */
	int		markint ;		/* mark-time interval */
	int		pollint ;		/* MS poll interval */
	int		speedint ;
	int		loglen ;
	int		salen ;			/* IPC socket address length */
	int		serial, subserial ;
	int		fd_input, fd_output ;
	int		fd_listenpass, fd_listentcp ;
	int		fd_req ;
	const char	zname[ZNAMELEN + 1] ;
	const char	cmd[LOGIDLEN + 1] ;
} ;

struct clientinfo {
	SOCKADDRESS	sa ;			/* peername socket address */
	const char	*peername ;
	const char	*netuser ;
	const char	*netpass ;
	const char	*netident ;
	const char	*service ;		/* service */
	const char	*subservice ;		/* subservice */
	long		mtype ;			/* message type */
	time_t		ctime ;			/* create time */
	pid_t		pid ;
	int		salen ;			/* socket length */
	int		fd_input, fd_output ;
	int		f_long ;		/* the "long" switch */
} ;

/* this is similar to old NETBUF structure */
struct resultbuf {
	int	maxlen ;
	int	len ;
	char	*buf ;
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
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


