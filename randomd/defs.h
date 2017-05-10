/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<system.h>
#include	<logfile.h>
#include	<bfile.h>


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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)
#define	JOBIDLEN	14


/* program exit status values */

#define	ES_OK		0		/* completed successfully */
#define	ES_BADARG	1		/* bad argument on invocation */
#define	ES_INFO		2		/* information only */
#define	ES_ERROR	3		/* error during processing */
#define	ES_MUTEX	4		/* mutual exclusion conflict */



/* program information */

struct proginfo_flags {
	uint	fd_stdout : 1 ;
	uint	fd_stderr : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	verbose : 1 ;
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	VOLATILE int	f_exit ;
	logfile	lh ;		/* program activity log */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	char	*version ;		/* program version string */
	char	*progname ;		/* program name */
	char	*searchname ;
	char	*programroot ;		/* program root directory */
	char	*nodename ;
	char	*domainname ;
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*pwd ;
	char	*logid ;		/* default program LOGID */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*devicefname ;
	char	*seedfname ;
	char	*prog_ps ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


