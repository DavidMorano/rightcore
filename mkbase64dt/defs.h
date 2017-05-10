/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<bfile.h>
#include	<localmisc.h>


#define	BUFLEN		100
#define	USAGECOLS	4


struct proginfo_flags {
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		quiet:1 ;
} ;

struct proginfo {
	const char	**envv ;
	const char	*version ;
	const char	*pwd ;
	const char	*progdir ;
	const char	*progname ;
	const char	*searchname ;
	const char	*pr ;
	const char	*helpfname ;
	bfile		*efp ;
	struct proginfo_flags	f ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


