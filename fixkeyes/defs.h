/* textset - header */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


struct gflags {
	uint	verbose : 1 ;
	uint	headers : 1 ;
} ;

struct global {
	char		*pwd ;
	char		*progdir ;
	char		*progname ;
	char		*pr ;
	char		*helpfile ;
	char		*blanks, *blankstring ;
	char		*headerstring ;
	bfile		*efp ;
	bfile		*ofp ;
	struct gflags	f ;
	int		debuglevel ;
	int		maxlines ;
	int		coffset ;
} ;


#endif /* DEFS_INCLUDE */


