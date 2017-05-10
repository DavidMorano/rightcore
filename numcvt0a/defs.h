/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif


struct proginfoflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
} ;

struct proginfo {
	struct proginfoflags	f ;
	bfile		*efp ;
	char		*progname ;
	char		*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


