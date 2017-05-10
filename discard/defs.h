/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<date.h>
#include	<vecstr.h>
#include	<localmisc.h>


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

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(MAXPATHLEN + (4 * 1024))





/* program information */

struct proginfo_flags {
	uint	log : 1 ;		/* do we have a log file ? */
	uint	slog : 1 ;		/* system log */
	uint	quiet : 1 ;		/* are we in quiet mode ? */
	uint	update : 1 ;		/* update mode */
	uint	binary : 1 ;		/* binary dump file */
	uint	receive : 1 ;		/* receive mode ??? */
	uint	dgram : 1 ;		/* DGRAM mode */
} ;

struct proginfo {
	char	**envv ;
	char	*version ;		/* program version string */
	char	*pwd ;
	char	*progdir ;		/* program directory */
	char	*progname ;		/* program name */
	char	*pr ;			/* program root directory */
	char	*searchname ;
	char	*nodename ;
	char	*domainname ;
	char	*username ;		/* ours */
	char	*groupname ;		/* ours */
	char	*logid ;		/* default program LOGID */
	char	*pidfname ;
	char	*lockfname ;
	char	*sumfname ;		/* summary file name */
	char	*homedname ;
	char	*workdname ;
	char	*tmpdname ;		/* temporary directory */
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*lfp ;		/* system log */
	bfile		*pidfp ;
	bfile		*sumfp ;	/* summary file */
	logfile		lh ;		/* program activity log */
	struct timeb	now ;
	struct proginfo_flags	f ;
	vecstr	localnames ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
	int	markint ;		/* mark interval */
	int	minpingint ;		/* minimum interval */
	int	minupdate ;		/* minimum update interval */
	int	mininputint ;		/* minimum input interval */
	char	zname[DATE_ZNAMESIZE + 1] ;
} ;

struct pinghost {
	int	minpingint ;
	int	to ;
	char	*name ;
} ;


#endif /* DEFS_INCLUDE */


