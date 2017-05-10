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

#ifndef	LINELEN
#define	LINELEN		200
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
} ;

struct proginfo {
	struct proginfo_flags	f ;
	bfile		*efp ;
	logfile		lh ;
	pid_t		pid ;
	int		debuglevel ;
	char		*programroot ;
	char		*progname ;
	char		*helpfile ;
	char		*banner ;
} ;


#endif /* DEFS_INCLUDE */


