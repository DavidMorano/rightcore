/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<bfile.h>
#include	<vecstr.h>
#include	<localmisc.h>


#define	CMDBUFLEN	((2 * MAXPATHLEN) + 20)


struct proginfo_flags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	char		**envv ;
	char		*version ;
	char		*pwd ;
	char		*progdir ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*tmpdname ;
	bfile		*efp ;
	struct proginfo_flags	f ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


