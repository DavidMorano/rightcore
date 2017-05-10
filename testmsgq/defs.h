/* defs (logname) */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<bfile.h>
#include	<logfile.h>
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

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		100
#define	USAGECOLS	4




struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	quiet : 1 ;
	uint	verbose : 1 ;
	uint	lockfile : 1, lockopen : 1 ;
	uint	pidfile : 1 ;
	uint	server : 1 ;
} ;

struct proginfo {
	char		**envv ;
	char		*pwd ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*version ;
	char		*banner ;
	char		*searchname ;
	char		*helpfname ;
	char		*tmpdname ;
	bfile		*efp ;
	bfile		*ofp ;
	logfile	lh ;
	struct proginfo_flags	f ;
	time_t		daytime ;
	pid_t		pid ;
	int		debuglevel ;
	int		verboselevel ;
	int		f_exit ;
} ;

struct msgbuffer {
	long	msgtype ;
	char	buf[BUFLEN + 1] ;
} ;


#endif /* DEFS_INCLUDE */


