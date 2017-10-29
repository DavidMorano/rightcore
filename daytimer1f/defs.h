/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<localmisc.h>


struct proginfo_flags {
	uint	quiet : 1 ;
	uint	lockfile : 1, lockopen : 1 ;
	uint	pidfile : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	logfile	lh ;
	bfile		*efp ;
	bfile		*pfp ;
	char		*programroot ;
	char		*progname ;
	char		*helpfile ;
	char		*banner ;
	char		*pidfname ;
	pid_t		pid ;
	pid_t		sid ;
	int		debuglevel ;
	int		verboselevel ;
} ;




/* local program defines */

#define	NPARG		2
#define	BUFLEN		MAXPATHLEN

#define	TIME_SLEEP	7
#define	MAIL_TICS	3
#define	FULL_TICS	100

#define	N		NULL

/* program states */

#define	S_NOMAIL	0
#define	S_MAIL		1
#define	S_MAILDONE	2

/* file descriptor to monitor */

#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	DEBUGFDVAR1	"DAYTIMER_DEBUGFD"
#define	DEBUGFDVAR2	"DEBUGFD"


#endif /* DEFS_INCLUDE */


