/* cdaudio - header */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


struct gflags {
	uint	verbose : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	struct gflags	f ;
	int		debuglevel ;
	char		*programroot ;
	char		*helpfile ;
	char		*progname ;
} ;


#endif /* DEFS_INCLUDE */


