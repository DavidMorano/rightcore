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
#include	<srvtab.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
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

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)
#define	JOBIDLEN	14




struct proginfo_flags {
	uint	fd_stdout : 1 ;
	uint	fd_stderr : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	slog : 1 ;		/* system log */
	uint	verbose : 1 ;
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
	uint	named : 1 ;		/* do we have named services ? */
} ;

struct proginfo {
	char	**envv ;		/* program start-up environment */
	char	*pwd ;
	char	*progdir ;		/* program directory */
	char	*progname ;		/* program name */
	char	*version ;		/* program version string */
	char	*programroot ;		/* program root directory */
	char	*nodename ;
	char	*domainname ;
	char	*hostname ;		/* concatenation of N + D */
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*logid ;		/* default program LOGID */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*stampdir ;		/* timestamp directory */
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*prog_rmail ;
	char	*prog_sendmail ;
	char	*defuser ;		/* default for servers */
	char	*defgroup ;		/* default for servers */
	char	*userpass ;		/* user password file */
	char	*machpass ;		/* machine password file */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*efp ;
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	f ;
	LOGFILE		lh ;		/* program activity log */
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	interval ;		/* override interval */
	int	maxjobs ;
} ;

struct serverinfo {
	varsub		*ssp ;		/* substitutions */
	vecstr		*elp ;		/* exports */
	SRVTAB		*sfp ;		/* service table */
} ;


#endif /* DEFS_INCLUDE */


