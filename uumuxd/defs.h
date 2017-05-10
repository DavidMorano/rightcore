/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<netdb.h>

#include	<system.h>
#include	<ema.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<bfile.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
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




/* program information */

struct gflags {
	uint	stderror : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	verbose : 1 ;
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
} ;

struct global {
	VOLATILE int	f_exit ;
	logfile	lh ;		/* program activity log */
	struct gflags	f ;
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	pid_t	pid, ppid ;
	uid_t	uid ;			/* real UID */
	uid_t	euid ;			/* effective UID */
	gid_t	gid ;
	int	debuglevel ;		/* debugging level */
	char	*progname ;		/* program name */
	char	*version ;		/* program version string */
	char	*programroot ;		/* program root directory */
	char	*nodename ;
	char	*domainname ;
	char	*username ;
	char	*groupname ;
	char	*pwd ;
	char	*logid ;		/* default program LOGID */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*pidfname ;
	char	*xfname ;
	char	*lockfile ;		/* lock file */
	char	*prog_sendmail ;
	char	*userpass ;		/* user password file */
	char	*machpass ;		/* machine password file */
} ;


#endif /* DEFS_INCLUDE */


