/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<localmisc.h>


struct proginfo_flags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	lockfile : 1 ;
	uint	lockopen : 1 ;
	uint	pidfile : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progename ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*tmpdname ;
	char		*helpfname ;
	char		*pidfname ;
	void		*efp ;
	void		*pfp ;
	logfile		lh ;
	struct proginfo_flags	f ;
	time_t		daytime ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		f_exit ;
	int		signal_num ;
} ;




/* local program defines */

#define	NPARG		2
#define	BUFLEN		MAXPATHLEN

#define	TIME_SLEEP	7
#define	MAIL_TICS	3
#define	FULL_TICS	100

#define	N		NULL


/* file descriptor to monitor */

#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


#endif /* DEFS_INCLUDE */


