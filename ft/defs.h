/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#define BUFSIZE		((2*MAXPATHLEN) + 4)
#define	BUFLEN		BUFSIZE


struct gflags {
	uint	debug : 1 ;
	uint	verbose : 1 ;
} ;

struct global {
	bfile		*efp ;
	struct userinfo	*up ;
	struct gflags	f ;
	logfile		lh ;
	utime_t		daytime ;
	int		debuglevel ;
	char		*progname ;
	char		*programroot ;
	char		*logfile ;
	char		*helpfile ;
	char		*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


