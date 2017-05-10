/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<logfile.h>
#include	<bfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<lfm.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<paramfile.h>
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

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	MAILUSERLEN
#define	MAILUSERLEN	LOGNAMELEN
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




struct proginfo_flags {
	uint	fd_stdout : 1 ;
	uint	fd_stderr : 1 ;		/* do we have STDERR ? */
	uint	onckey : 1 ;		/* we have an ONC private key */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	slog : 1 ;		/* system log */
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
	uint	named : 1 ;		/* do we have named services ? */
	uint	mbtab : 1 ;		/* mailbox table file */
	uint	srvtab : 1 ;		/* do we have an server table ? */
	uint	acctab : 1 ;		/* do we have an access table ? */
	uint	path : 1 ;		/* we have a path file */
	uint	defacc : 1 ;		/* default access restrictions */
	uint	secure : 1 ;		/* all secure */
	uint	secure_root : 1 ;	/* secure root directory */
	uint	secure_conf : 1 ;	/* secure configuration file */
	uint	secure_mbtab : 1 ;
	uint	secure_srvtab : 1 ;
	uint	secure_acctab : 1 ;
	uint	secure_path : 1 ;
} ;

struct proginfo {
	char	**envv ;		/* program start-up environment */
	char	*pwd ;
	char	*progdname ;
	char	*progname ;		/* program name */
	char	*pr ;			/* program root directory */
	char	*version ;		/* program version string */
	char	*banner ;
	char	*searchname ;		/* program search name */
	char	*nodename ;
	char	*domainname ;
	char	*hostname ;		/* concatenation of N + D */
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*logid ;		/* default program LOGID */
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*srvtab, *acctab ;	/* file names */
	char	*pathfname ;
	char	*workdname ;
	char	*tmpdname ;		/* temporary directory */
	char	*stampdname ;		/* timestamp directory */
	char	*prog_rmail ;
	char	*prog_sendmail ;
	char	*defuser ;		/* default for servers */
	char	*defgroup ;		/* default for servers */
	char	*defacc ;		/* default access group */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*efp ;
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	f ;
	varsub		tabsubs ;	/* substitutions */
	vecstr		exports ;	/* exports */
	vecstr		path ;		/* search path for servers */
	LOGFILE		lh ;		/* program activity log */
	LFM		pidlock ;
	SRVTAB		stab ;		/* service table */
	ACCTAB		atab ;		/* access table */
	PARAMFILE	mbtab ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
	int	serial ;
	int	interval ;		/* program check interval (secs) */
	int	runtime ;		/* program run time (secs) */
	int	maxjobs ;
	char	maildname[MAXPATHLEN + 1] ;
} ;

struct configinfo {
	int	mincheck ;
	char	stampfname[MAXPATHLEN + 1] ;
	char	mbfname[MAXPATHLEN + 1] ;
	char	maildname[MAXPATHLEN + 1] ;
} ;


#endif /* DEFS_INCLUDE */


