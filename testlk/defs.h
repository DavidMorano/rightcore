/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

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
#define	BUFLEN		(MAXPATHLEN + (4 * 1024))

/* extra exit codes */

#define	EX_TIMEDOUT	1
#define	EX_BADINIT	2
#define	EX_INVALID	3






struct proginfo_flags {
	uint	log : 1 ;		/* do we have a log file ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
} ;

struct proginfo {
	char	**envv ;
	char	*pwd ;
	char	*progename ;
	char	*progdname ;
	char	*progname ;		/* program name */
	char	*pr ;		/* program root directory */
	char	*searchname ;
	char	*version ;		/* program version string */
	char	*banner ;
	char	*nodename ;
	char	*domainname ;
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*logid ;		/* default program LOGID */
	char	*homedname ;
	char	*workdname ;
	char	*tmpdname ;		/* temporary directory */
	char	*pidfname ;
	char	*lockfname ;		/* lock file */
	char	*fileroot ;
	bfile		*efp ;
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	struct proginfo_flags	f ;
	logfile		lh ;		/* program activity log */
	time_t	daytime ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
} ;


#endif /* DEFS_INCLUDE */


