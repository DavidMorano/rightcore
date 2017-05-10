/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#define	BUFLEN		200
#define	LINELEN		200


struct flags {
	uint	verbose : 1 ;
} ;


struct global {
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	struct flags	f ;
	int	debuglevel ;
	char	*progname ;
	char	*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


