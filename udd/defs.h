/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<logfile.h>


#define	BUFLEN		(4096 + MAXPATHLEN)
#define	LINELEN		(100 + MAXPATHLEN + 3)


struct gflags {
	uint	debug : 1 ;
	uint	verbose : 1 ;
	uint	sysv_rt : 1 ;
	uint	sysv_ct : 1 ;
	uint	newprogram : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	logfile	lh ;
	struct gflags	f ;
	time_t		daytime ;
	int		debuglevel ;
	char		*progname ;
	char		*logfname ;
} ;


#endif /* DEFS_INCLUDE */


