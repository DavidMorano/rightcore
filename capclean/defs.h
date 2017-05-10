/* defs */

/* capclean header */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>

#include	<bfile.h>
#include	<localmisc.h>


struct proginfo_flags {
	uint	verbose : 1 ;
	uint	remove : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	bfile		*efp ;
	bfile		*ofp ;
	char		*version ;
	char		*searchname ;
	char		*progdir ;
	char		*pwd ;
	char		*progname ;
	char		*programroot ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


