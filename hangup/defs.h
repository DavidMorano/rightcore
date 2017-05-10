/* defs */

/* hangup header */

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
	uint	print : 1 ;
	uint	fakeit : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	bfile		*efp ;
	int		debuglevel ;
	char		*progname ;
	char		*tmpdir ;
} ;


#endif /* DEFS_INCLUDE */


