/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<paramopt.h>
#include	<localmisc.h>


#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN * 2)
#endif

#define	PASSLEN		32
#define	PROMPTLEN	100

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct gflags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	sevenbit : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	struct gflags	f ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		debuglevel ;
	char		*progname ;
	char		*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


