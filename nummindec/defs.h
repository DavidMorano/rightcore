/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif

#define	ES_OK		0
#define	ES_BADOPEN	1


struct gflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
} ;

struct global {
	struct gflags	f ;
	bfile		*efp ;
	char		*progname ;
	char		*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


