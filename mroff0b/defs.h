/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<logfile.h>
#include	<vecstr.h>


#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

/* program exit status values */

#define	ES_OK		0		/* completed successfully */
#define	ES_BADARG	1		/* bad argument on invocation */
#define	ES_INFO		2		/* information only */
#define	ES_ERROR	3		/* error during processing */





struct proginfo_flags {
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	stderror : 1 ;		/* do we have STDERR ? */
	uint	log : 1 ;		/* do we have a log file ? */
	uint	verbose : 1 ;
	uint	quiet : 1 ;		/* are we in quiet mode ? */
} ;

struct proginfo {
	VECSTR	stores ;
	char	**envv ;
	char	*pwd ;
	char	*progdname ;		/* program name */
	char	*progename ;		/* program name */
	char	*progname ;		/* program name */
	char	*pr ;			/* program root directory */
	char	*searchname ;
	char	*version ;		/* program version string */
	char	*banner ;
	char	*rootname ;
	char	*nodename ;
	char	*domainname ;
	char	*username ;
	char	*groupname ;
	char	*logid ;		/* default program LOGID */
	char	*workdname ;
	char	*tmpdname ;		/* temporary directory */
	struct proginfo_flags	have, f ;
	struct proginfo_flags	open ;
	void		*efp ;
	logfile	lh ;		/* program activity log */
	time_t	daytime ;
	pid_t	pid ;
	uid_t	uid ;			/* real UID */
	uid_t	euid ;			/* effective UID */
	gid_t	gid ;
	int	pwdlen ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


