/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<logfile.h>
#include	<bfile.h>
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
#define	BUFLEN		((5 * 1024) + MAXHOSTNAMELEN)
#define	JOBIDLEN	14




/* program information */

struct proginfo_flags {
	uint	fd_stdout : 1 ;
	uint	fd_stderr : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	verbose : 1 ;
	uint	quiet : 1 ;		/* are we in quiet mode ? */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	LOGFILE		lh ;		/* program activity log */
	bfile		*efp ;
	char	*progname ;		/* program name */
	char	*version ;		/* program version string */
	char	*programroot ;		/* program root directory */
	char	**envv ;		/* program start-up environment */
	char	*nodename ;
	char	*domainname ;
	char	*hostname ;		/* concatenation of N + D */
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*logid ;		/* default program LOGID */
	char	*tmpdir ;		/* temporary directory */
	char	*userpass ;		/* user password file */
	char	*machpass ;		/* machine password file */
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
} ;


#endif /* DEFS_INCLUDE */


