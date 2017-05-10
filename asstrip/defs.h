/* as_strip - header */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


struct gflags {
	uint	verbose : 1 ;
	uint	noblanks : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	struct gflags	f ;
	int		debuglevel ;
	char		*programroot ;
	char		*helpfile ;
	char		*progname ;
} ;


#endif /* DEFS_INCLUDE */


