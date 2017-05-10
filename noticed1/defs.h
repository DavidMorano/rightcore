/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<logfile.h>
#include	<bfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<pwfile.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"jobdb.h"


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
#define	LOGNAMELEN	32
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
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

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)
#define	JOBIDLEN	14


/* connection failure codes */

#define	SERVICE_CFNOSVC		1	/* specified service not found */
#define	SERVICE_CFACCESS	2	/* access denied to host for service */
#define	SERVICE_CFNOSRV		3	/* server was not found */

/* general request codes */

#define	RCODE_NOOP		0
#define	RCODE_EXIT		1
#define	RCODE_PASSFD		2

/* standing request codes */

#define	RCODE_START		100
#define	RCODE_BOOTTIME		100
#define	RCODE_SYSTEMMISC	101
#define	RCODE_MAX		102


struct proginfo_flags {
	uint	fd_stdout : 1 ;
	uint	fd_stderr : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	slog : 1 ;		/* system log */
	uint	verbose : 1 ;
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
	uint	acctab : 1 ;		/* do we have an ACCess TABle ? */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	LOGFILE		lh ;		/* program activity log */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*efp ;
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	char	**envv ;		/* our startup program environment */
	char	*progname ;		/* program name */
	char	*version ;		/* program version string */
	char	*programroot ;		/* program root directory */
	char	*nodename ;
	char	*domainname ;
	char	*hostname ;		/* concatenation of N + D */
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*pwd ;
	char	*logid ;		/* default program LOGID */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*defacc ;		/* default access name */
	char	*spooldname ;		/* spool directory */
	char	*prog_rmail ;
	char	*prog_sendmail ;
	char	*defuser ;		/* default for servers */
	char	*defgroup ;		/* default for servers */
	char	*userpass ;		/* user password file */
	char	*machpass ;		/* machine password file */
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbose output */
	int	marktime ;		/* interval */
	int	serial ;
} ;

struct serverinfo {
	varsub		*ssp ;
	vecstr		*elp ;			/* exports */
	PWFILE		*pfp ;			/* passwd file object */
	SRVTAB		*sfp ;
	ACCTAB		*atp ;
	JOBDB	jobs ;
	int	fd_ipc ;
	int	fd_listentcp ;
	int	fd_input, fd_output ;
	int	mqid_ipc ;
	int	serial ;
} ;

struct clientinfo {
	SOCKADDRESS	sa ;			/* network socket */
	char		*peername ;
	char		*netuser ;
	char		*netpass ;
	char		*netident ;
	long		mtype ;			/* message type */
	int		salen ;			/* socket length */
} ;

struct msgbuffer {
	long	msgtype ;
	uchar	buf[MSGBUFLEN + 1] ;
} ;


#endif /* DEFS_INCLUDE */


