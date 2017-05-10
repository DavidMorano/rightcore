/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<bfile.h>


#define	BUFLEN		100


struct proginfo_flags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	bfile		*efp ;
	char		*programroot ;
	char		*progname ;
	char		*helpfile ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


