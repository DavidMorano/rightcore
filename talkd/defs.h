/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<logfile.h>
#include	<bfile.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	BUFLEN		100


struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	logfile : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	bfile		*efp ;
	char		*progname ;
	char		*version ;
	char		*searchname ;
	char		*programroot ;
	char		*logfname ;
	char		*helpfname ;
	char		*logid ;
	LOGFILE		actlog ;
	struct proginfo_flags	f ;
	time_t		daytime ;
	int		pid ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


