/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>
#include	<logfile.h>


#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NPARG		2
#define	BUFLEN		(MAXPATHLEN * 2)


/* program exit status values */

#define	ES_OK		0		/* completed successfully */
#define	ES_BADARG	1		/* bad argument on invocation */
#define	ES_INFO		2		/* information only */
#define	ES_ERROR	3		/* error during processing */




struct proginfo_flags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	lockfile : 1, lockopen : 1 ;
	uint	pidfile : 1 ;
	uint	server : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	bfile		*efp ;
	bfile		*ofp ;
	logfile	lh ;
	int		debuglevel ;
	int		f_exit, f_signal ;
	int		signal_num ;
	pid_t		pid ;
	char		*programroot ;
	char		*progname ;
	char		*helpfile ;
	char		*banner ;
	char		*pidfname ;
} ;

struct msgbuffer {
	long	msgtype ;
	char	buf[BUFLEN + 1] ;
} ;


#endif /* DEFS_INCLUDE */


